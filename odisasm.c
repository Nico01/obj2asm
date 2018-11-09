#include <stdio.h>
#include <string.h>
#include "o.h"

/*
** Local Prototypes
*/
char *reg_format( int, char *, char *, char *, char * );
int check_getc( dword * );
int check_getw( dword * );
int check_getl( dword * );
int check_getv( dword *, int );
int get_checkc( dword * );
void instr_opcode( char * );
void instr_operand( char * );
int check_forward( INST_T [], int );
int mod0( int, int, char *, char *, uchar, int, int * );
int mod1( int, int, char *, char *, uchar );
int mod2( int, int, char *, char *, uchar, int, int * );
int mod3( int, char *, uchar, int, int * );
int do_sib( char *, uchar * );
int do_mod_rm( char *, uchar, uchar, int, int, int );
int byte_immed( char *, int, int );
int word_immed( char *, int );

char    *regs[4][8] = {
      "al",  "cl",  "dl",  "bl",  "ah",  "ch",  "dh",  "bh",
      "ax",  "cx",  "dx",  "bx",  "sp",  "bp" , "si",  "di",
     "eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi",
      "??",  "??",  "??",  "??",  "??",  "??",  "??",  "??"
};

char    *sregs [] = { "es" , "cs" , "ss" , "ds" , "fs" , "gs"  };
char    *sregsc[] = { "es:", "cs:", "ss:", "ds:", "fs:", "gs:" };

char    *cr_regs[] = { "cr0",    "", "cr2", "cr3", "", "",    "",    "" };
char    *dr_regs[] = { "dr0", "dr1", "dr2", "dr3", "", "", "dr6", "dr7" };
char    *tr_regs[] = {    "",    "",    "",    "", "", "", "tr6", "tr7" };

char    *addr_mode[] = { "[bx+si", "[bx+di", "[bp+si", "[bp+di",
                            "[si",    "[di",    "[bp",    "[bx" };

char    *addr_386m[] = {   "[eax",   "[ecx",   "[edx",   "[ebx",
                           "[esp",   "[ebp",   "[esi",   "[edi" };

char    *sib_scale[] = {  "", "*2", "*4", "*8" };

char    *op_grp[10][8] = {
     "add",   "or",  "adc",  "sbb",  "and",  "sub",  "xor",  "cmp",
     "rol",  "ror",  "rcl",  "rcr",  "shl",  "shr",     "",  "sar",
    "test",     "",  "not",  "neg",  "mul", "imul",  "div", "idiv",
     "inc",  "dec",     "",     "",     "",     "",     "",     "",
     "inc",  "dec", "call", "call",  "jmp",  "jmp", "push",     "",
    "sldt",  "str", "lldt",  "ltr", "verr", "verw",     "",     "",
    "sgdt", "sidt", "lgdt", "lidt", "smsw",     "", "lmsw",     "",
        "",     "",     "",     "",   "bt",  "bts",  "btr",  "btc",
     "pop",     "",     "",     "",     "",     "",     "",     "",
     "mov",     "",     "",     "",     "",     "",     "",     ""
};

char    *sz_text[] = { "byte ptr ", "word ptr ", "dword ptr ",
                       "qword ptr ", "tbyte ptr "              };

char *dir_fmt[] = {
    "%s,%s",                    /* 0 = mem,reg */
    "%s,%s",                    /* 1 = reg,mem */
    "%s",                       /* 2 = mem */
    "%s,%s,%s",                 /* 3 = reg,mem,third */
    "%s,%s,%s"                  /* 4 = mem,reg,third */
};

typedef struct modrm_s MODRM_CLASS;

struct modrm_s {
    int     dir;
    int     size;
    int     method;
    int     group;
    int     size_needed;
    int     type;
};


/*
** These mod r/m classes are not in any particular order, but the
** instruction table indexes these entries with it's mod_rm_type field.
*/
MODRM_CLASS modrm_class[] = {
    0,  0,  1,  0,  FALSE,  UNKNOWN,    /*  0 Math instructions (byte) M,reg */
    0,  1,  1,  0,  FALSE,  UNKNOWN,    /*  1 Math instructions (word) M,reg (and ARPL) */
    1,  0,  1,  0,  FALSE,  UNKNOWN,    /*  2 Math instructions (byte) reg,M */
    1,  1,  1,  0,  FALSE,  UNKNOWN,    /*  3 Math instructions (word) reg,M */
    0,  0,  5,  1,  TRUE,   UNKNOWN,    /*  4 Group 1 (math) byte */
    0,  1,  6,  1,  TRUE,   UNKNOWN,    /*  5 Group 1 (math) word */
    0,  1,  7,  1,  TRUE,   UNKNOWN,    /*  6 Group 1 (math) signed byte */
    0,  1,  2,  0,  TRUE,   UNKNOWN,    /*  7 Segment register unloading */
    1,  1,  2,  0,  TRUE,   UNKNOWN,    /*  8 Segment register loading */
    1,  1,  1,  0,  FALSE,  UNKNOWN,    /*  9 LEA instruction */
    2,  1,  0,  9,  FALSE,  UNKNOWN,    /* 10 POP mod 0 r/m instruction */
    0,  0,  5,  2,  FALSE,  UNKNOWN,    /* 11 Group 2 (rotates) byte */
    0,  1,  5,  2,  FALSE,  UNKNOWN,    /* 12 Group 2 (rotates) word */
    0,  0,  3,  2,  TRUE,   UNKNOWN,    /* 13 Group 2 (rotates) byte by 1 */
    0,  1,  3,  2,  TRUE,   UNKNOWN,    /* 14 Group 2 (rotates) word by 1 */
    0,  0,  4,  2,  TRUE,   UNKNOWN,    /* 15 Group 2 (rotates) byte by CL */
    0,  1,  4,  2,  TRUE,   UNKNOWN,    /* 16 Group 2 (rotates) word by CL */
    1,  2,  1,  0,  FALSE,  UNKNOWN,    /* 17 LES,LDS,BOUND,LSS,LFS,LGS */
    0,  0,  5, 10,  TRUE,   UNKNOWN,    /* 18 Move immediate (byte) */
    0,  1,  6, 10,  TRUE,   UNKNOWN,    /* 19 Move immediate (word) */
    2,  0,  0,  3,  FALSE,  UNKNOWN,    /* 20 Group 3 (special) byte */
    2,  1,  0,  3,  FALSE,  UNKNOWN,    /* 21 Group 3 (special) word */
    2,  0,  0,  4,  TRUE,   UNKNOWN,    /* 22 Group 4 (inc/dec) byte */
    2,  1,  0,  5,  FALSE,  UNKNOWN,    /* 23 Group 5 (special) word/dword */
    3,  1,  8,  0,  FALSE,  UNKNOWN,    /* 24 IMUL (3 parms [reg,mem,immed word]) */
    3,  1,  9,  0,  FALSE,  UNKNOWN,    /* 25 IMUL (3 parms [reg,mem,immed signed byte]) */
    2,  1,  0,  6,  FALSE,  UNKNOWN,    /* 26 Group 6 (special 286/386 instructions) */
    2,  3,  0,  7,  TRUE,   FWORD_PTR,  /* 27 Group 7 (special 286/386 instructions) */
    1,  1,  1,  0,  TRUE,   UNKNOWN,    /* 28 LAR,LSL reg,M (word) */
    0,  0,  5,  8,  TRUE,   BYTE_PTR,   /* 29 Group 8 (386-bit) byte */
    2,  0,  0,  0,  TRUE,   BYTE_PTR,   /* 30 386 Set cc Instructions */
    0,  2, 10,  0,  FALSE,  UNKNOWN,    /* 31 386 Mov r32,crX */
    1,  2, 10,  0,  FALSE,  UNKNOWN,    /* 32 386 Mov crX,r32 */
    0,  2, 11,  0,  FALSE,  UNKNOWN,    /* 33 386 Mov r32,drX */
    1,  2, 11,  0,  FALSE,  UNKNOWN,    /* 34 386 Mov drX,r32 */
    0,  2, 12,  0,  FALSE,  UNKNOWN,    /* 35 386 Mov r32,trX */
    1,  2, 12,  0,  FALSE,  UNKNOWN,    /* 36 386 Mov trX,r32 */
    1,  1,  1,  0,  TRUE,   BYTE_PTR,   /* 37 386 Movsx, Movzx */
    1,  1,  1,  0,  TRUE,   WORD_PTR,   /* 38 386 Movsx, Movzx */
    4,  1,  9,  0,  TRUE,   UNKNOWN,    /* 39 386 Shld,Shrd mem/reg,reg,imm byte */
    4,  1, 13,  0,  TRUE,   UNKNOWN,    /* 40 386 Shld,Shrd mem/reg,reg,cl */
};


extern int pub_compare();
extern int ext_compare();

int over_seg     = -1;      /*   -1  = No overiding segment yet      */

static  int over_opsize  = FALSE;   /* FALSE = No overiding operand size yet */
static  int over_adrsize = FALSE;   /* FALSE = No overiding address size yet */
static  int size_large   = FALSE;   /* Within a WORD address size            */
static  int addr_large   = FALSE;   /* Special Addressing Modes              */
static  int size_bytes   = 2;       /* Default WORD size                     */
static  int addr_bytes   = 2;       /* Default WORD size                     */

static  char    fp_opcode[8] = {0};     /* 7 + \0 */
static  char    fp_wait = FALSE;        /* No preceeding "wait" */

void inst_init()
{
    over_seg     = -1;              /* Setup for the next instruction */
    over_opsize  = FALSE;
    over_adrsize = FALSE;

    if ( segment_mode == 386 ) {
        size_large = TRUE;
        addr_large = TRUE;
    } else {
        size_large = FALSE;
        addr_large = FALSE;
    }
    size_bytes = segment_bytes;
    addr_bytes = segment_bytes;
}


char *reg_format( reg, output_string, base_text, base_fmt, extra_fmt )
    int             reg;
    char            *output_string;
    char            *base_text;
    char            *base_fmt;
    char            *extra_fmt;
{
    if ( reg == NREG ) {
        sprintf( output_string, base_fmt, base_text );
    } else {
        if ( reg < 9 ) {        /* Byte sized half register */
            sprintf( output_string, extra_fmt, base_text, regs[0][reg-1] );
        } else {
            if ( reg < 17 ) {   /* Word/DWord sized general register */
                if ( size_large ) {
                    sprintf( output_string, extra_fmt, base_text, 
                                                            regs[2][reg-9] );
                } else {
                    sprintf( output_string, extra_fmt, base_text, 
                                                            regs[1][reg-9] );
                }
            } else {
                sprintf( output_string, extra_fmt, base_text, sregs[reg-17] );
            }
        }
    }
    return( output_string );
}

int check_getc( result )
    dword   *result;
{
    int             ch;

    ch = buff_getc();

    if ( ch == EOF ) {
        return( TRUE );
    } else {
        *result = (dword)ch;
        return( FALSE );
    }
}

int check_getw( result )
    dword   *result;
{
    int             ch;

    ch = buff_getc();
    if ( ch == EOF ) {
        return( TRUE );
    }
    *result = ch;
    ch = buff_getc();
    if ( ch == EOF ) {
        return( TRUE );
    }
    *result += ch << 8;
    return( FALSE );
}

int check_getl( result )
    dword   *result;
{
    int             ch;

    ch = buff_getc();
    if ( ch == EOF ) {
        return( TRUE );
    }
    *result = (dword)((uchar)ch);

    ch = buff_getc();
    if ( ch == EOF ) {
        return( TRUE );
    }
    *result += (dword)((uchar)ch) << 8;

    ch = buff_getc();
    if ( ch == EOF ) {
        return( TRUE );
    }
    *result += (dword)((uchar)ch) << 16;
    ch = buff_getc();
    if ( ch == EOF ) {
        return( TRUE );
    }
    *result += (dword)((uchar)ch) << 24;

    return( FALSE );
}

int check_getv( value, flag )
    dword   *value;
    int             flag;
{
    int             result;

    if ( flag ) {
        result = check_getl( value );
    } else {
        result = check_getw( value );
    }
    return( result );
}

int get_checkc( offset )
    dword   *offset;
{
    int     dcheck;

    dcheck = data_check( 1 );

    if ( dcheck != NORMAL && dcheck != BAD ) {
        return( TRUE );                 /* Must be no labels or fixups */
    }
    return( check_getc( offset ) );
}

void instr_opcode( text )
    char    *text;
{
    if ( pass == 3 ) {
        out_opcode( text );
    }
    tab_offset = 0;
    inst_init();            /* Reset for next instruction */
}

void instr_operand( text )
    char    *text;
{
    if ( pass == 3 ) {
        out_operand( text );
    }
}

int stub( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    byte = byte;               /* Prevent unused variable warnings */
    text = text;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */
    return( 0 );

}

int in_out( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             dir;
    int             size;
    char            temp[50];

    dir  = ( byte & 0x02 ) >> 1;
    size = ( byte & 0x01 );

    if ( size == 1 && size_large ) {
        size = 2;
    }
    instr_opcode(text);

    if ( dir == 0 ) {
        sprintf( temp, "%s,dx", regs[size][0] );
    } else {
        sprintf( temp, "dx,%s", regs[size][0] );
    }
    instr_operand(temp);

    class = class;              /* Prevent unused variable warnings */

    return(1);
}

int string_byte( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    char            opcode[10];
    char            operand[50];
    int             size;
    int             mode;
    char            *format;

    class = class;                  /* Prevent unused variable warnings */

    size = byte & 0x01;
    if ( size == 1 && size_large ) {
        size = 2;
    }

    switch( byte ) {
        case 0xA4:
        case 0xA5:
            format = "%ses:[di],%s%s[si]";
            mode = 0;
            break;
        case 0xA6:
        case 0xA7:
            format = "%ses:[di],%s%s[si]";
            mode = 0;
            break;
        case 0xAA:
        case 0xAB:
            mode = 2;
            break;
        case 0xAC:
        case 0xAD:
            format = "%s%s[si]";
            mode = 1;
            break;
        case 0xAE:
        case 0xAF:
            mode = 2;
            break;
    }

    strcpy( opcode, text );

    if ( over_seg == -1 ) {
        switch( size ) {
            case 0:     strcat(opcode,"b");     break;
            case 1:     strcat(opcode,"w");     break;
            case 2:     strcat(opcode,"d");     break;
        }
        instr_opcode(opcode);
        return(1);
    } else {
        switch( mode ) {
            case 0:
                sprintf( operand, format, sz_text[size],
                                            sz_text[size], sregsc[over_seg] );
                break;
            case 1:
                sprintf( operand, format, sz_text[size], sregsc[over_seg] );
                break;
            case 2:                 /* No segment overide on */
                return( 0 );        /* SCAS instructions     */
        }
        instr_opcode( opcode );
        instr_operand( operand );
        return(1);
    }

}

int one_byte( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    char            line[50];

    if ( strlen(text) == 0 ) {
        return(0);
    }

    reg_format( class, line, text, "%s", "%s\t%s" );

    instr_opcode(line);
    byte = byte;               /* Prevent unused variable warnings */

    return(1);
}

int two_byte( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    dword   offset;
    char            line[50];
    char            temp[50];

    if ( get_checkc( &offset ) ) {
        return( 0 );
    }

    reg_format( class, line, text, "%s\t", "%s\t%s," );

    out_hexize( offset, temp, 1 );      /* Convert 1 byte into hex */
    strcat( line, temp );

    instr_opcode( line );

    byte = byte;                   /* Prevent unused varaible warnings */

    return(2);
}

int two_ubyte( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    dword   offset;
    char            line[50];

    if ( get_checkc( &offset ) ) {
        return( 0 );
    }

    if ( offset <= 127L ) {
        sprintf( line, "%s+0%02lXh", text, offset );
    } else {
        sprintf( line, "%s-0%02lXh", text, 0x0100L - offset );
    }
    instr_opcode( line );

    byte = byte;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    return(2);
}

int three_byte( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    dword   offset;
    int             dummy;
    int             data_size;
    int             size_tmp;
    char            line[50];
    char            temp[50];

    if ( over_seg != -1 ) {         /* No segment overide allowed */
        return( 0 );
    }

    reg_format( class, line, text, "%s\t", "%s\t%s," );

    size_tmp = size_large;

    if ( byte == 0xC2 || byte == 0xCA ) {       /* <RET jjkk>/<RETF jjkk> */
        size_tmp = FALSE;
        size_bytes = 2;
    }

    data_size = size_bytes;

    switch( data_check( 1 ) ) {
        case BAD:       return( 0 );
        case LABEL:     return( 0 );
        case FIXUP:     if ( fix_rec->word_sized
                                && fix_rec->extended == size_tmp ) {
                            get_fix( temp, 2, TRUE, size_bytes,
                                                    NEAR, FALSE, &dummy, DS );
                            strcat( line, temp );
                            adjust_assumes();
                            instr_opcode( line );
                            return( data_size+1 );
                        } else {
                            return( 0 );
                        }
        case NORMAL:    if ( check_getv( &offset, size_tmp ) ) {
                            return( 0 );
                        }
                        out_hexize( offset, temp, size_bytes );
                        strcat( line, temp );
                        instr_opcode(line);
                        return( data_size+1 );
    }
    return(0);
}

int five_byte( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    dword   offset;
    dword   segment;
    int             size_known;
    char            line[50];
    char            temp[50];
    int             data_size;

    if ( over_seg != -1 ) {         /* No segment overide allowed */
        return(0);
    }

    data_size = size_bytes;

    switch( data_check( 1 ) ) {
        case BAD:
            return( 0 );
        case LABEL:
            return( 0 );
        case FIXUP:
            if ( fix_rec->form != POINTER || fix_rec->extended != size_large ) {
                return( 0 );
            }
            get_fix( temp, 1, FALSE, size_bytes, NEAR, TRUE, &size_known, CS );
            sprintf( line, "far ptr %s", temp );
            break;
        case NORMAL:
            if ( check_getv( &offset, size_large ) ) {
                return( 0 );
            }
            if ( data_check(3) != NORMAL ) {
                return( 0 );
            }
            if ( check_getw( &segment ) ) {
                return( 0 );
            }
            out_hexize( segment, temp, 2 );      /* Convert 2 byte into hex */
            strcpy( line, temp );
            strcat( line, ":" );
            out_hexize( offset, temp, size_bytes ); /* Convert 2/4 bytes into hex */
            strcat( line, temp );
            break; 
    }

    adjust_assumes();

    instr_opcode( text );
    instr_operand( line );

    byte = byte;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    return( 3 + data_size );
}

int one_a( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    char            line[50];

    if ( size_large ) {
        reg_format( class, line, text, "%s\teax", "%s\teax,%s" );
    } else {
        reg_format( class, line, text, "%s\tax", "%s\tax,%s" );
    }

    instr_opcode(line);

    byte = byte;                   /* Prevent unused varaible warnings */

    return(1);
}

int two_a( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             size;
    dword   offset;
    char            line[50];

    size = (byte & 0x01);

    if ( check_getc( &offset ) ) {
        return( 0 );
    }

    if ( size == 0 ) {
        sprintf( line, "0%02lXh,al", offset );
    } else {
        sprintf( line, "0%02lXh,ax", offset );
    }
    instr_opcode( text );
    instr_operand(line);

    byte = byte;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    return(2);
}

int three_a( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             dir;
    int             reg_size;
    int             data_size;
    int             type;
    int             size_known;
    dword   offset;
    char            line[50];
    char            seg_text[10];
    char            temp[50];
    char            temp2[15];
    int             ref_seg;

    data_size = addr_bytes;

    dir  = ( byte & 0x02 ) >> 1;
    reg_size = ( byte & 0x01 );

    if ( reg_size == 1 && size_large ) {
        reg_size = 2;
    }
    type = reg_size_to_type(reg_size);

    if ( over_seg != -1 ) {                 /* Any previous segment overide? */
        ref_seg = 17+over_seg;
        strcpy( seg_text, sregsc[over_seg] );
    } else {
        ref_seg = DS;
        seg_text[0] = '\0';
    }

    switch( data_check( 1 ) ) {
        case BAD:
            return( 0 );
        case LABEL:     
            return( 0 );
        case FIXUP:
            if ( fix_rec->word_sized && fix_rec->extended == addr_large ) {
                get_fix( temp, 0, FALSE, size_bytes, type, TRUE,
                                                        &size_known, ref_seg );
            } else {
                return( 0 );
            }
            strcpy( line, "" );
            if ( compatibility == 2 ) {
                strcat( line, seg_text );
                strcat( line, " " );
                if ( !size_known ) {
                    strcat( line, type_to_text(type) );
                }
            } else {
                if ( !size_known ) {
                    strcat( line, type_to_text(type) );
                }
                strcat( line, seg_text );
            }
            strcat( line, temp );
            break;
        case NORMAL:
            if ( check_getv( &offset, addr_large ) ) {
                return( 0 );
            }
            out_hexize( offset, temp2, addr_bytes );    /* Make hex digits */

            if ( compatibility == 2 ) {
                sprintf( line, "%s .%s", seg_text, temp2 );
            } else {
                if ( over_seg == -1 ) {
                    sprintf( line, "ds:[%s]", temp2 );
                } else {
                    sprintf( line, "%s[%s]", seg_text, temp2 );
                }
            }
            break;
    }

    if ( dir == 0 ) {
        sprintf( temp, "%s,%s", regs[reg_size][0], line );
    } else {
        sprintf( temp, "%s,%s", line, regs[reg_size][0] );
    }

    adjust_assumes();

    instr_opcode( text );
    instr_operand( temp );

    class = class;              /* Prevent unused variable warnings */

    return( 1 + data_size );
}

int enter( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    dword   num_bytes;
    dword   nest_level;
    char            line[50];

    switch( data_check(1) ) {
        case BAD:
        case LABEL:
        case FIXUP:
            return( 0 );
        case NORMAL:
            if ( check_getw(&num_bytes) ) {
                return( 0 );
            }
            if ( check_getc(&nest_level) ) {
                return( 0 );
            }
            sprintf( line, "0%04lXh,0%02lXh", num_bytes, nest_level );
            break;
    }
    instr_opcode( text );
    instr_operand( line );

    byte = byte;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    return(4);
}

int check_forward( inst_table, method )
    INST_T          inst_table[];
    int             method;
{
    dword   byte;
    int             data;
    int             valid;
    extern char     *buff_cur;
    char            *buff_save;

    buff_save = buff_cur;
    /*
    ** The next byte must be a valid instruction
    */
    if ( get_checkc( &byte ) ) {
        return( 0 );
    }
    data = (uchar)byte;

#ifdef DEBUG
    printf("Checking forward with byte [%02X]\n", data );
#endif

    inst_offset += 1;

    if ( method == 0 && pass == 3 ) {
        pass = 2;               /* Temporarily disable output */
        valid = (*inst_table[data].rtn)
                    ((uchar)data,
                    inst_table[data].text,
                    inst_table[data].special );
        pass = 3;
    } else {
        valid = (*inst_table[data].rtn)
                    ((uchar)data,
                    inst_table[data].text,
                    inst_table[data].special );
    }

    inst_offset -= 1;

    if ( method == 0 || valid == 0 ) {
        buff_cur = buff_save;
    }

    return( valid );
}

int seg_over( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             valid;
    int             save_seg;

    text = text;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    save_seg = over_seg;

    if ( byte >= 0x64 ) {
        over_seg = 4 + byte - 0x64;
    } else {
        over_seg = (byte & 0x18) >> 3;
    }

    valid = check_forward( instr, 1 );
    if ( valid ) {
        return( valid+1 );
    } else {
        over_seg = save_seg;
        return( 0 );
    }
}

int opsize_over( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             valid;
    int             save_large;
    int             save_bytes;

    byte = byte;               /* Prevent unused variable warnings */
    text = text;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    save_large = size_large;
    save_bytes = size_bytes;

    size_large = !size_large;       /* Toggle size */
    if ( size_bytes == 2 ) {
        size_bytes = 4;
    } else {
        size_bytes = 2;
    }
    over_opsize = TRUE;

    valid = check_forward( instr, 1 );
    if ( valid ) {
        return( valid+1 );
    } else {
        size_large = save_large;
        size_bytes = save_bytes;
        return( 0 );
    }
}

int adrsize_over( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             valid;
    int             save_large;
    int             save_bytes;

    byte = byte;               /* Prevent unused variable warnings */
    text = text;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    save_large = addr_large;
    save_bytes = addr_bytes;

    addr_large = !addr_large;           /* Toggle address */
    if ( addr_bytes == 2 ) {
        addr_bytes = 4;
    } else {
        addr_bytes = 2;
    }
    over_adrsize = TRUE;

    valid = check_forward( instr, 1 );
    if ( valid ) {
        return( valid+1 );
    } else {
        addr_large = save_large;
        addr_bytes = save_bytes;
        return( 0 );
    }
}

int wait( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             valid;

    byte = byte;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    fp_opcode[0] = '\0';

    valid = check_forward( instr, 0 );
    if ( !valid ) {
        instr_opcode(text);
        return(1);
    } else {
        if ( strlen(fp_opcode) == 0 ) {
            instr_opcode( text );
            return( 1 );
        } else {
            fp_wait = TRUE;         /* Will be reset by "esc" routine */
            hex_finish = FALSE;
            return( 1 );
        }
    }
}

int prefix( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             valid;

    valid = check_forward( instr, 0 );

    byte = byte;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    if ( valid ) {
        instr_opcode( text );
        tab_offset = 1;
        hex_finish = FALSE;
        return( 1 );
    } else {
        return( 0 );
    }
}

int extra( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             result;

    result = check_forward( ex_instr, 1 );

    byte = byte;               /* Prevent unused variable warnings */
    text = text;
    class = class;

    if ( result == 0 ) {
        return( 0 );
    } else {
        return( result+1 );
    }
}

int disp8( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             dcheck;
    char            line[60];
    char            *short_text;
    dword   offset;
    dword   dest;
    PUB_T           *pub_rec;

    if ( over_seg != -1 ) {             /* No segment overide allowed */
        return( 0 );
    }

    dcheck = data_check(1);

    if ( dcheck != NORMAL && dcheck != BAD ) {
        return( 0 );
    }

    if ( check_getc( &offset ) ) {
        return( 0 );
    }

    if ( offset == 0xFFFFFFFFL ) {      /* This would generate a label in  */
        return( 0 );                    /* the middle of the jmp statement */
    }                                   /* thereby preventing it anyway    */

    /*
    ** Special 'SHORT' for small jumps 
    */
    short_text = "";
    if ( compatibility != 2 ) {
        if ( byte == 0xEB || segment_mode == 386 ) {
            short_text = "short ";
        }
    }

    dest = inst_offset + (signed char)offset + 2;

    pub_rec = check_public( 1, segment, dest, 'L' );

    if ( pub_rec ) {
        pub_rec->type = NEAR;
        offset = dest - pub_rec->offset;
        if ( offset > 0 ) {
            sprintf( line, "%s%s + 0%04lXh", 
                                short_text, pub_rec->name, offset );
        } else {
            sprintf( line, "%s%s", short_text, pub_rec->name );
        }
    } else {
        if ( compatibility == 2 ) {
            sprintf( text, " %s.0%04lXh", short_text, offset );
        } else {
            sprintf( text, "%s[%s:0%04lXh]", short_text, cseg_name, offset );
        }
    }

    instr_opcode( text );
    instr_operand( line );

    byte = byte;               /* Prevent unused variable warnings */
    class = class;
    return( 2 );
}

int disp16( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    dword   offset;
    dword   dest;
    int             dummy;
#if 0
    int             bad_size;
#endif
    char            line[60];
    char            *short_text;
    PUB_T           *pub_rec;
    int             data_size;

    byte = byte;               /* Prevent unused variable warnings */
    class=class;              /* Prevent unused variable warnings */

    data_size = addr_bytes;

    if ( over_seg != -1 )               /* No segment overide allowed */
        return( 0 );

    switch( data_check( 1 ) ) {
    case BAD:
         return( 0 );
    case LABEL:
         return( 0 );
    case FIXUP:
         if ( fix_rec->word_sized && fix_rec->extended == addr_large
                                  && fix_rec->relate == 0 ) {
             get_fix( line, 1, FALSE, size_bytes, NEAR, TRUE, &dummy, CS );
             adjust_assumes();
             instr_opcode( text );
             instr_operand( line );
             return( 1 + data_size );
         } else {
             return( 0 );
         }
    case NORMAL:
         if ( check_getv( &offset, addr_large ) ) {
             return( 0 );
         }
         /* note: the next if actually checks for 1 byte too far in
         ** the negative direction. but this is ok, because it would be
         ** replaced by a short jmp
         */
         if ( offset < 0x80 || offset >= 0xFFFFFF7FL ) {
#if 0
             bad_size = TRUE;
#endif
             short_text = "near ptr ";
         } else {
             short_text = "";
#if 0
             bad_size = FALSE;
#endif
         }

         dest = inst_offset + offset + addr_bytes + 1;
         pub_rec = check_public( 1, segment, dest, 'L' );

         if ( pub_rec ) {
             offset = dest - pub_rec->offset;
             if ( offset > 0L ) {
                 if ( addr_large ) {
                     sprintf( line, "%s%s + 0%08lXh", short_text,
                              pub_rec->name, offset );
                 } else {
                     sprintf( line, "%s%s + 0%04lXh", short_text,
                              pub_rec->name, offset );
                 }
             } else {
                 sprintf( line, "%s%s", short_text, pub_rec->name );
             }
         } else {
             if ( compatibility == 2 ) {
                 if ( addr_large ) {
                     sprintf( line, "%s .0%08lXh", short_text, offset );
                 } else {
                     sprintf( line, "%s .0%04lXh", short_text, offset );
                 }
             } else {
                 if ( addr_large ) {
                     sprintf( line, "%s[%s:0%08lXh]", short_text,
                              cseg_name, offset );
                 } else {
                     sprintf( line, "%s[%s:0%04lXh]", short_text,
                              cseg_name, offset );
                 }
             }
         }

         instr_opcode( text );
         instr_operand( line );
#if 0
         if ( bad_size ) {
             if ( pass == 3 ) {
                 out_comment( "Large jump translated into short" );
             }
             instr_opcode( "nop" );
             if ( pass == 3 ) {
                 out_comment( "Nop inserted to retain proper length" );
             }
         }
#endif
         return( 1 + data_size );
    }
    return(0);
}

int two_bcd( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    dword   offset;

    byte = byte;               /* Prevent unused variable warnings */
    class = class;              /* Prevent unused variable warnings */

    if ( check_getc( &offset ) ) {
        return( 0 );
    }

    if ( offset == 0x0AL ) {
        instr_opcode( text );
        return(2);
    } else {
        return( 0 );
    }
}

int mod0( ref_mode, sib_offset,sib_text, text, r_m, type, size_known )
    int             ref_mode;
    int             sib_offset;
    char            *sib_text;
    char            *text;
    uchar   r_m;
    int             type;
    int             *size_known;
{
    dword   offset;
    int             result;
    PUB_T           *pub_rec;
    STRUC_T         *pub_struct;
    int             ref_seg;
    char            this_member[NAMESIZE+1+1];  /* 1 for ., 1 for \0 */
    char            temp[16];

    this_member[0] = '\0';

    if ( (r_m == 6 && !addr_large) || (r_m == 5 && addr_large) ) {
        if ( over_seg != -1 ) {
            ref_seg = 17+over_seg;
        } else {
            ref_seg = DS;
        }
        switch( data_check( 2+sib_offset ) ) {
            case BAD:
                result = -1;
                return( result );
            case LABEL:
                result = -1;
                return( result );
            case FIXUP:
                if ( fix_rec->word_sized && fix_rec->extended == addr_large ) {
                    get_fix( text, 0, FALSE, size_bytes, 
                                            type, TRUE, size_known, ref_seg );
                    if ( strlen(sib_text) ) {
                        strcat( text, "[" );
                        strcat( text, sib_text );
                        strcat( text, "]" );
                    }
                    result = addr_bytes+sib_offset;
                } else {
                    result = -1;
                }
                return( result );
            case NORMAL:
                if ( check_getv( &offset, size_large ) ) {
                    result = -1;
                    return( result );
                }
                if ( add_labels ) {
                    pub_rec = check_public( 0, 0, offset, 'S' );
                } else {
                    pub_rec = NULL;
                }
                if ( pub_rec ) {
                    offset -= pub_rec->offset;
                    pub_struct = pub_rec->structure;
                    if ( pub_struct ) {
                        find_member( this_member, pub_struct, &offset );
                    }
                    if ( offset != 0 ) {
                        out_hexize( offset, temp, size_bytes );
                        sprintf( text, "%s%s + %s",
                                 pub_rec->name, this_member, temp );
                    } else {
                        sprintf( text, "%s%s", pub_rec->name, this_member );
                    }
                } else {
                    out_hexize( offset, temp, size_bytes );
                    if ( compatibility == 2 ) {
                        sprintf( text, " .%s", temp );
                    } else {
                        if ( over_seg == -1 ) {
                            sprintf( text, "ds:[%s", temp );
                        } else {
                            sprintf( text, "[%s", temp );
                        }
                        if ( strlen( sib_text ) ) {
                            strcat( text, "+" );
                            strcat( text, sib_text );
                        }
                        strcat( text, "]" );
                    }
                }
                result = addr_bytes+sib_offset;
                *size_known = FALSE;
                return( result );
        }
    }
    if ( addr_large ) {
        strcpy( text, addr_386m[r_m] );
    } else {
        strcpy( text, addr_mode[r_m] );
    }
    if ( strlen( sib_text ) ) {
        strcat( text, "+" );
        strcat( text, sib_text );
    }
    strcat( text, "]" );
    result = sib_offset;
    *size_known = TRUE;
    return( result );
}

int mod1( ref_mode, sib_offset, sib_text, text, r_m )
    int             ref_mode;
    int             sib_offset;
    char            *sib_text;
    char            *text;
    uchar   r_m;
{
    char            temp[50];
    dword   offset;
    int             result;
    uchar   zero_rm;

    if ( check_getc( &offset ) ) {
        result = -1;
    } else {
        if ( addr_large ) {
            /*
            ** Assembler should have optimized
            */
            zero_rm = 5;
            strcpy( text, addr_386m[r_m] );
        } else {
            /*
            ** Assembler should have optimized
            */
            zero_rm = 6;
            strcpy( text, addr_mode[r_m] );
        }
        if ( strlen(sib_text) ) {
            strcat( text, "+" );
            strcat( text, sib_text );
        }
        if ( offset >= 0x80 ) {
            sprintf( temp, "-0%02lXh", 0x0100L-offset );
        } else {
            if ( offset == 0L ) {
                if ( r_m == zero_rm ) {
                    temp[0] = '\0';
                } else {
                    return( -1 );
                }
            } else {
                sprintf( temp, "+0%02lXh", offset );
            }
        }
        if ( compatibility == 2 ) {
            strcat( text, "]" );
            strcat( text, temp );
        } else {
            strcat( text, temp );
            strcat( text, "]" );
        }
        result = 1 + sib_offset;
    }
    return( result );
}

int mod2( ref_mode, sib_offset, sib_text, text, r_m, type, size_known )
    int             ref_mode;
    int             sib_offset;
    char            *sib_text;
    char            *text;
    uchar   r_m;
    int             type;
    int             *size_known;
{
    dword   offset;
    int             result;
    char            temp[50];
    char            *sign;
    int             ref_seg;

    if ( r_m == 2 || r_m == 3 ) {
        ref_seg = SS;               /* SS relative addressing modes */
    } else {
        ref_seg = DS;
    }
    if ( over_seg != -1 ) {
        ref_seg = 17+over_seg;
    }

    switch( data_check(2+sib_offset) ) {
        case BAD:
            result = -1;
            break;
        case LABEL:
            result = -1;
            break;
        case FIXUP:
            if ( fix_rec->word_sized && fix_rec->extended == addr_large ) {
                get_fix( temp, 0, FALSE, addr_bytes,
                                            type, TRUE, size_known, ref_seg );
                if ( compatibility == 2 ) {
                    strcpy( text, temp );
                    strcat( text, addr_mode[r_m] );
                } else {
                    if ( addr_large ) {
                        strcpy( text, addr_386m[r_m] );
                    } else {
                        strcpy( text, addr_mode[r_m] );
                    }
                    if ( strlen(sib_text) ) {
                        strcat( text, "+" );
                        strcat( text, sib_text );
                    }
                    strcat( text, "+" );
                    strcat( text, temp );
                }
                strcat( text, "]" );
                result = addr_bytes+sib_offset;
            } else {
                result = -1;
            }
            break;
        case NORMAL:
            if ( check_getv( &offset, addr_large ) ) {
                result = -1;
            } else {
                if ( offset == 0x00000000L ) { 
                    return( -1 );       /* Assembler should have optimized */
                }
                if ( addr_large ) {
                    strcpy( text, addr_386m[r_m] );
                } else {
                    strcpy( text, addr_mode[r_m] );
                }
                if ( strlen(sib_text) ) {
                    strcat( text, "+" );
                    strcat( text, sib_text );
                }
                if ( offset > 0x80000000L ) {
                    offset = -offset;
                    sign = "-";
                } else {
                    sign = "+";
                }
                out_hexize( offset, temp, addr_bytes );
                if ( compatibility == 2 ) {
                    strcat( text, "]" );
                    strcat( text, sign );
                    strcat( text, temp );
                } else {
                    strcat( text, sign );
                    strcat( text, temp );
                    strcat( text, "]" );
                }
                result = addr_bytes+sib_offset;
                *size_known = TRUE;
            }
            break;
    }
    return( result );
}

int mod3( ref_mode, text, r_m, type, type_known )
    int             ref_mode;
    char            *text;
    uchar   r_m;
    int             type;
    int             *type_known;
{
    switch( type ) {
        case BYTE_PTR:  strcpy( text, regs[0][r_m] );   break;
        case WORD_PTR:  strcpy( text, regs[1][r_m] );   break;
        case DWORD_PTR: strcpy( text, regs[2][r_m] );   break;
        default:        strcpy( text, regs[3][r_m] );   break;
    }
    *type_known = TRUE;
    return( 0 );
}

int do_sib( text, base )
    char            *text;
    uchar   *base;
{
    dword   sib_byte;
    int             ss;
    int             idx;

    switch( data_check(2) ) {
        case LABEL:
            return( -1 );
        case FIXUP:
            return( -1 );
        case BAD:
        case NORMAL:
            if ( check_getc( &sib_byte ) ) {
                return( -1 );
            }
            ss    = (int)(sib_byte & 0xC0L) >> 6;
            idx   = (int)(sib_byte & 0x38L) >> 3;
            *base = (uchar)(sib_byte & 0x07L);
            if ( idx == 0x04 ) {            /* For special [ESP] */
                if ( ss != 0 ) {
                    return( -1 );
                } else {
                    strcpy( text, "" );
                }
            } else {
                strcat( text, regs[2][idx] );
                strcat( text, sib_scale[ss] );
            }
            return( 1 );
    }
}

int do_mod_rm( line, mod, r_m, type, size_needed, ref_mode )
    char            *line;
    uchar   mod;
    uchar   r_m;
    int             type;
    int             size_needed;
    int             ref_mode;
{
    int             size_known;
    int             sib_offset;
    char            sib_text[50];
    char            temp2[50];
    int             result;
    
    size_known = FALSE;

    strcpy( sib_text, "" );
    sib_offset = 0;

    if ( addr_large ) {
        if ( r_m == 4 && mod != 3 ) {
            /* Get result and new r_m */
            sib_offset = do_sib( sib_text, &r_m );
            if ( sib_offset == -1 ) {
                return( -1 );
            }
        }
    }
    switch( mod ) {
        case 0: result = mod0( ref_mode, sib_offset, sib_text, temp2,
                                            r_m, type, &size_known );
                break;
        case 1: result = mod1( ref_mode, sib_offset, sib_text, temp2, r_m );
                size_known = TRUE;
                break;
        case 2: result = mod2( ref_mode, sib_offset, sib_text, temp2,
                                            r_m, type, &size_known );
                break;
        case 3: result = mod3( ref_mode, temp2, r_m, type, &size_known );
                size_needed = FALSE;
                break;
    }

    if ( result != -1 ) {
        if ( compatibility == 2 ) {
            if ( over_seg != -1 ) {         /* Any previous segment overide? */
                strcat( line, sregsc[over_seg] );
                strcat( line, " " );
            }
            if ( !size_known || size_needed ) {
                strcat( line, type_to_text(type) );
            }
        } else {
            /*
            ** If the size is already known, then we don't need 'xxxx' ptr text
            */
            if ( !size_known || size_needed ) {
                strcat( line, type_to_text(type) );
            }
            if ( over_seg != -1 ) {         /* Any previous segment overide? */
                strcat( line, sregsc[over_seg] );
            }
        }
        strcat( line, temp2 );
    }
    return( result );
}

int byte_immed( text, additional, sign )
    char            *text;
    int             additional;
    int             sign;
{
    int             dcheck;
    dword   offset;

    dcheck = data_check( additional+2 );

    switch( dcheck ) {
        case LABEL:     return( 1 );
        case FIXUP:     return( 1 );
        case BAD:
        case NORMAL:
            if ( check_getc( &offset ) ) {
                return( 1 );
            }
            if ( sign ) {
                if ( offset <= 0x7F ) {
                    sprintf( text, "+0%02lXh", offset );
                } else {
                    sprintf( text, "-0%02lXh", 0x0100L-offset );
                }
            } else {
                out_hexize( offset, text, 1 );
            }
            break;
    }
    return( 0 );
}

int word_immed( text, additional )
    char            *text;
    int             additional;
{
    int             dcheck;
    int             dummy;
    dword   offset;

    dcheck = data_check( additional+2 );

    switch( dcheck ) {
        case LABEL:
            return( 1 );
        case FIXUP:
            if ( fix_rec->word_sized && fix_rec->extended == addr_large ) {
                get_fix( text, 2, TRUE, size_bytes, NEAR, FALSE, &dummy, DS );
            } else {
                return( 1 );
            }
            break;
        case BAD:
            return( 1 );
        case NORMAL:
            if ( check_getv( &offset, size_large ) ) {
                return( 1 );
            }
            if ( size_large ) {
                out_hexize( offset, text, 4 );
            } else {
                out_hexize( offset, text, 2 );
            }
            break;
    }
    return( 0 );
}


int mod_reg( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    char            *opcode;
    int             dir;
    int             size;
    int             reg_size;
    int             size_needed;
    int             type;
    int             method;
    int             group;
    dword   mod_reg;
    uchar   mod;
    uchar   r_m;
    uchar   reg;
    int             additional;
    char            mem_text[50];
    char            reg_text[50];
    char            third_text[50];
    char            operands[80];
    int             ref_mode;

    if ( get_checkc( &mod_reg ) ) {
        return( 0 );
    }

    mod = (uchar)(((int)mod_reg & 0xC0) >> 6);
    reg = (uchar)(((int)mod_reg & 0x38) >> 3);
    r_m = (uchar)(((int)mod_reg & 0x07) >> 0);

    dir         = modrm_class[class].dir;
    size        = modrm_class[class].size;
    method      = modrm_class[class].method;
    group       = modrm_class[class].group;
    size_needed = modrm_class[class].size_needed;
    type        = modrm_class[class].type;

    ref_mode = 0;

    if ( group == 6 ) {
        if ( mod == 3 ) {
            if ( reg == 0 || reg == 1 || reg == 2 || reg == 3 ) {
                addr_large = FALSE;         /* 286 only instructions */
                size_large = FALSE;
            }
        }
        type = WORD_PTR;
        size_needed = TRUE;
    }

    reg_size = size;                /* 386 Segments effect register names */
    if ( reg_size == 1 && size_large ) {
        reg_size = 2;
    }
    if ( type == UNKNOWN ) {
        type = reg_size_to_type( reg_size );
    }

    if ( group == 0 ) {
        opcode = text;
    } else {
        opcode = op_grp[group-1][reg];
    }
    if ( strlen(opcode) == 0 ) {
        return( 0 );
    }

    if ( class == 26 ) {
        reg_size = 1;
    }

    if ( byte == 0x62 || byte == 0x63 ) {
        if ( byte == 0x63 ) {
            type = WORD_PTR;
        }
        size_needed = TRUE;     /* LAR, LSL, BOUND, ARPL need size */
    }
    if ( byte == 0xC4 || byte == 0xC5
           || byte == 0xB2 || byte == 0xB4 || byte == 0xB5 ) {
        size_needed = TRUE;
    }

    if ( group == 3 ) {         /* Group 3 has a test instruction which    */
        if ( reg == 0 ) {       /* needs an immediate value (byte or word) */
            method = 5 + size;  /* the rest of group 3 needs nothing       */
            dir = 0;
            size_needed = TRUE;
        } else {
            method = 0;
            if ( reg == 2 || reg == 3 ) {
                size_needed = FALSE;
            } else {
                size_needed = TRUE;
            }
        }
    }

    if ( group == 5 ) {                 /* Group 5 has call far and jump far */
        if ( reg == 3 || reg == 5 ) {   /* instructions which are dwords     */
                                        /* the rest of group 5 is words      */
            if ( size_large ) {
                type = FWORD_PTR;
            } else {
                type = DWORD_PTR;
            }
            size_needed = TRUE;
        } else {
            if ( reg == 0 || reg == 1 ) {
                size_needed = TRUE;
            }
        }
    }

    if ( group == 7 ) {                 /* Group 7 has mostly FWORD sized    */
        if ( reg == 4 || reg == 6 ) {   /* EAs, but these two are WORD sized */
            type = WORD_PTR;
        }
    }

    if ( dir == 0 && mod == 3 && (class == 0 || class == 1) ) {
        /*
        ** Ambiguous!   eg. OBJ 20F8 = and al,bh => MASM 22C7
        ** This is because MASM prefers the first operand to be
        ** a register and the other operand to be a mem/reg.
        */
        return(0);
    }

    mem_text[0] = '\0';
    additional = do_mod_rm( mem_text, mod, r_m, type, size_needed, ref_mode );
    if ( additional == -1 ) {
        return( 0 );
    }

    switch( method ) {
        case 0:         /* No additional information (one operand only) */
            break;
        case 1:         /* Other operand is a register */
                        /* LES,LDS,BOUND,LSS,LFS,LGS, ARPL instructions */
                        /* revert to Word */
            if ( byte == 0xC4 || byte == 0xC5 || byte == 0x62 || byte == 0x63
                   || byte == 0xB2 || byte == 0xB4 || byte == 0xB5 ) {
                reg_size = 1;
            }

            strcpy( reg_text, regs[reg_size][reg] );
            break;
        case 2:         /* Other operand is a segment register */
            if ( reg > 5 ) {    /* 0=ES,1=CS,2=SS,3=DS,4=FS,5=GS */
                return( 0 );
            }
            strcpy( reg_text, sregs[reg] );
            break;
        case 3:         /* Other operand is 1 */
            strcpy( reg_text, "1" );
            break;
        case 4:         /* Other operand is register CL */
            strcpy( reg_text, "cl" );
            break;
        case 5:         /* Immediate byte value */
            if ( byte_immed(reg_text, additional,FALSE) ) {
                return( 0 );
            }
            additional++;
            break;
        case 6:         /* Immediate word value (could be fixupp) */
            if ( word_immed(reg_text, additional) ) {
                return( 0 );
            }
            if ( size_large ) {
                additional += 4;
            } else {
                additional += 2;
            }
            break;
        case 7:         /* Immediate signed byte value */
            if ( byte_immed(reg_text,additional,TRUE) ) {
                return( 0 );
            }
            additional++;
            break;
        case 8:         /* IMUL (3 operands [reg,mem,immed word]) */
            strcpy( reg_text, regs[reg_size][reg] );
            if ( word_immed(third_text, additional) ) {
                return(0);
            }
            additional += 2;
            break;
        case 9:         /* IMUL (3 operands [reg,mem,immed signed byte]) */
            strcpy( reg_text, regs[reg_size][reg] );
            if ( byte_immed(third_text, additional,TRUE) ) {
                return(0);
            }
            additional += 1;
            break;
        case 10:        /* Control Register (CR0...) */
            strcpy( reg_text, cr_regs[reg] );
            break;
        case 11:        /* Debug Register (DR0...) */
            strcpy( reg_text, dr_regs[reg] );
            break;
        case 12:
            strcpy( reg_text, tr_regs[reg] );
            break;
        case 13:
            strcpy( reg_text, regs[reg_size][reg] );
            strcpy( third_text, "cl" );
            break;
    }

    if ( dir & 0x01 ) {
        sprintf( operands, dir_fmt[dir], reg_text, mem_text, third_text );
    } else {
        sprintf( operands, dir_fmt[dir], mem_text, reg_text, third_text );
    }

    adjust_assumes();
    instr_opcode( opcode );
    instr_operand( operands );

    return( 2+additional );
}

int esc( byte, text, class )
    uchar   byte;
    char            *text;
    int             class;
{
    int             size;
    int             dir;
    int             type;
    dword   mod_reg;
    uchar   mod;
    uchar   r_m;
    uchar   lll;        /* Described as LLL in 80386 Prog Ref Man. */
    uchar   ttt;        /* Described as TTT in 80386 Prog Ref Man. */
    uchar   esc_byte;
    uchar   mf;
    int             additional;
    int             size_needed;
    char            line[50];
    char            temp[50];
    int             ref_mode;

    text = text;               /* Prevent unused variable warnings */
    class=class;              /* Prevent unused variable warnings */

    size = (byte & 0x01);
    dir  = (byte & 0x02) >> 1;
    ref_mode = 0;

    if ( get_checkc( &mod_reg ) ) {
        return( 0 );
    }
    mod = (uchar)(((int)mod_reg & 0xC0) >> 6);
    lll = (uchar)(((int)mod_reg & 0x38) >> 3);
    r_m = (uchar)(((int)mod_reg & 0x07) >> 0);

    ttt = (uchar)(byte & 0x07);
    esc_byte = (uchar)(ttt << 3) + lll;

    if ( mod == 0x03 ) {
        esc_special( temp, line, esc_byte, r_m );
        additional = 0;
    } else {
        mf = (uchar)((esc_byte & 0x30) >> 4);

        size_needed = TRUE;

        switch( mf ) {
            case 0:     type = DWORD_PTR;   break;
            case 1:     type = DWORD_PTR;   break;
            case 2:     type = QWORD_PTR;   break;
            case 3:     type = WORD_PTR;    break;
        }

        /*
        ** There are always exceptions to the rule (Thank you, Intel)
        */
        if (    esc_byte == 0x1D || esc_byte == 0x1F 
             || esc_byte == 0x3C || esc_byte == 0x3E ) {
            type = TBYTE_PTR;
        }
        if ( esc_byte == 0x2F ) {
            type = WORD_PTR;
        }
        if ( esc_byte == 0x3D || esc_byte == 0x3F ) {
            type = DWORD_PTR;
        }

        if (    (esc_byte >= 0x0C && esc_byte <= 0x0F)
             || (esc_byte >= 0x2C && esc_byte <= 0x2E) ) {
            size_needed = FALSE;
        }
        sprintf( line, "" );
        additional = do_mod_rm( line, mod, r_m, type, size_needed, ref_mode );
        if ( additional == -1 ) {           /* Error in 'do_mod_rm' */
            return( 0 );
        }
        strcpy( temp, esc_inst[esc_byte] );
    }
    if ( strlen( temp ) ) {
        if ( temp[1] == 'n' && fp_wait ) {
            fp_opcode[0] = temp[0];             /* Remove the 'n' */
            strcpy( &fp_opcode[1], &temp[2] );
        } else {
            strcpy( fp_opcode, temp );
        }
        instr_opcode( fp_opcode );
        instr_operand( line );
    } else {
        sprintf( line, "0%02Xh,%s", esc_byte, regs[1][r_m] );
        instr_opcode( "esc" );
        instr_operand( line );
    }

    fp_wait = FALSE;                /* Reset always */
    dir = dir;
    size = size;
    return( 2+additional );
}
