#include <stdio.h>
#include <string.h>
#include "o.h"

extern dword data_offset;
extern char *buff_end;
extern char *buff_cur;
extern char *buff_beg;
extern char buff[];

/*
** Local Variables
*/
static char     hex_comment[COMSIZE] = {0};
static char     idb_buff[256];
static char     dup_buff[256];
static char     struc_buff[256];
static char     structure_form[128];
static NODE_T   *sex_node;
static NODE_T   *data_node;
static DAT_T    *data_rec;
static int      record_overused = 0;

/*
** Local Prototypes
*/
void pub_advance( void );
void fix_advance( void );
void lone_public( void );
void data_byte( int );
void pre_dups( long, int );
int proc_idb( int, dword, int *, int, word *, dword *);
int proc_label( void );
int proc_fixup( void );
int proc_normal( int );
void iterated( DAT_T *, DAT_T * );
void enumerated( DAT_T *, DAT_T *, int );
void proc_drec( DAT_T *, DAT_T *, int );
NODE_T *list_sex( NODE_T *, int );

void pub_advance()
{
    /*
    ** Advance to next Publics record
    */
    pub_node = traverse( pub_node, RIGHT );
    pub_rec = (PUB_T *)pub_node->data;
}

void fix_advance()
{
    /*
    ** Advance to next Fixup record
    */
    fix_node = traverse( fix_node, RIGHT );
    fix_rec = (FIX_T *)fix_node->data;
}

void hint_advance(void)
{
    /*
    ** Advance to next Hint record
    */
    hint_node = traverse( hint_node, RIGHT );
    hint_rec = (HINT_T *)hint_node->data;
}

void lone_public()
{
    char    operand[80];
    long    relative;

    if ( pass == 3 ) {
        out_label( pub_rec->name );
        out_directive( "=" );
        if ( segment == 0 ) {
            sprintf( operand, "0%04Xh", pub_rec->offset );
        } else {
            relative = (long)pub_rec->offset - inst_offset;
            if ( relative < 0L ) {
                sprintf( operand, "$ - 0%04lXh", -relative );
            } else {
                sprintf( operand, "$ + 0%04lXh", relative );
            }
        }
        out_operand( operand );
        out_endline();
    }
}

int data_check( offset )
    word    offset;
{
    int             direct;

    /*
    ** Are we at a public symbol?
    */
    pub_search.seg_idx = segment;
    pub_search.offset  = inst_offset + offset;
    direct = RIGHT;
    while ( pub_node != public_tree
            && (direct = pub_compare( &pub_search, pub_rec )) == LEFT ) {
        lone_public();
        pub_advance();
    }
    if ( direct == EQUAL ) {
        return( LABEL );
    }

    /*
    ** Are we at a fix-up position?
    */
    fix_search.seg_idx    = segment;
    fix_search.dat_offset = data_offset;
    fix_search.offset     = (int)(inst_offset - data_offset) + offset;
    direct = RIGHT;
    while ( fix_node != fix_tree 
            && (direct = fix_compare( &fix_search, fix_rec )) == LEFT ) {
        fix_advance();
    }
    /* Are we on a fixup that is in the right position?  We can't just test
    ** whether the "direct==EQUAL" because we could have fixups from the
    ** next data record showing up.
    */
    if ( fix_rec->seg_idx == segment
      && fix_rec->dat_offset + fix_rec->offset == inst_offset + offset ) {
        return( FIXUP );
    }
    /*
    ** Are we close to a public symbol? (prevents 2 byte operands)
    */
    if ( pub_node != public_tree 
            && pub_rec->seg_idx == pub_search.seg_idx 
            && pub_rec->offset  == pub_search.offset + 1 ) {
        return( BAD );
    }
    /*
    ** Are we close to a fix-up? (prevents 2 byte operands)
    */
    if ( fix_node != fix_tree
      && fix_rec->seg_idx == segment
      && fix_rec->dat_offset + fix_rec->offset == inst_offset + offset + 1 ) {
        return( BAD );
    }
    return( 0 );
}

void data_byte( size )
    int     size;
{
    char    *opcode;
    int     out_count;
    int     times;
    char    operand[50];
    char    rev_buff[10];
    char    temp[4];

    if ( last_pub_rec && pass == 3 ) {
        if ( last_pub_rec->type == FAR ) {
            out_directive("=");
            out_operand("$");
        }
        if ( last_pub_rec->type == NEAR ) {
            out_endline();
        }
    }

    opcode = size_to_opcode( size, &times );

    while ( times ) {
        out_count = 0;
        strcpy( operand, "0" );
        while ( out_count < size ) {
            rev_buff[ out_count ] = (uchar)buff_regetc();
            out_count++;
        }
        out_count = size;
        while ( out_count ) {
            --out_count;
            sprintf( temp, "%02X", (uchar)rev_buff[out_count] );
            strcat( operand, temp );
        }
        strcat( operand, "h" );
        if ( pass == 3 ) {
            out_directive( opcode );
            out_operand( operand );
            out_endline();
        }
        --times;
    }
    last_pub_rec = NULL;
}

void pre_dups( count, orgable )
    long    count;
    int     orgable;
{
    char    *opcode;
    char    *operand;
    char    temp[80];
    int     times;

    if ( last_pub_rec && pass == 3 ) {
        if ( last_pub_rec->type == FAR ) {
            out_directive("=");
            out_operand("$");
        }
        if ( last_pub_rec->type == NEAR ) {
            out_endline();
        }
    }

    if ( count != 0L ) {
        if ( (orgable && inst_offset == 0 && count > 0L) || count < 0L ) {
            opcode = "ORG";
            sprintf( temp, "0%04Xh", inst_offset + count );
            operand = temp;
        } else {
            if ( compatibility == 2 ) {
                operand = "1";                  /* Default operand */
                switch( (int)count ) {
                    case 1:    opcode = "rb";  break;
                    case 2:    opcode = "rw";  break;
                    case 4:    opcode = "rd";  break;
                    default:    opcode = "rs";
                                sprintf( temp, "0%04Xh", count );
                                operand = temp;
                                break;
                }
            } else {
                operand = "1 dup (?)";          /* Default operand */

                opcode = size_to_opcode( (int)count, &times );

                sprintf( temp, "%d dup(?)", times );
                operand = temp;
            }
        }
        if ( pass == 3 && count != 0L ) {
            out_directive( opcode );
            out_operand( operand );
            out_endline();
        }
        inst_offset += count;
        last_pub_rec = NULL;
    }
}

int proc_idb( depth, mult, dup_only, extended, length_used, repeat_cnt )
    int             depth;
    dword   mult;
    int             *dup_only;
    int             extended;
    word    *length_used;
    dword   *repeat_cnt;
{
    dword   repeat;
    word    num_blocks;
    word    blocks;
    word    length;
    int             data;
    word    data_size;
    word    result;
    int             fix_size;
    char            operand[80];
    uchar   rev_buff[10];
    word    out_count;
    int             dcheck;
    int             dummy;

    result = 512;

    repeat = (dword)buff_regetc();
    repeat += (dword)buff_regetc() << 8;
    *length_used += 2;

    if ( extended ) {
        repeat += (dword)buff_regetc() << 16;
        repeat += (dword)buff_regetc() << 24;
        *length_used += 2;
    }
    num_blocks = buff_regetc();
    num_blocks += buff_regetc() << 8;
    *length_used += 2;

    if ( repeat_cnt ) {             /* Return repeat count if needed */
        *repeat_cnt = repeat;
    }

    if ( repeat != 1 || num_blocks != 0 ) {
        sprintf( idb_buff, "%ld dup(", repeat );
        strcat( dup_buff, idb_buff );
    }

    mult *= repeat;

    if ( num_blocks ) {
        blocks = num_blocks;
        while ( blocks ) {
            if ( depth == 2 ) {
                *dup_only = TRUE;
            }
            data_size = proc_idb( depth+1, mult, dup_only, extended,
                                                       length_used, NULL );
            if ( data_size < result ) {
                result = data_size;
            }
            --blocks;
            if ( blocks != 0 ) {
                strcat( dup_buff, "," );
                strcat( struc_buff, "," );
            }
        }
    } else {
        length = buff_regetc();
        result = length;
        *length_used += length + 1;         /* one extra for the length byte */
        data_size = 0;
        while ( length ) {
            do {
                data = buff_getc();
                if ( data == EOF ) {
                    break;
                }
                if ( data < ' ' || data > '~' ) {
                    break;
                }
                data_size++;
                inst_offset += mult;
                dcheck = data_check( 0 );
            } while ( *dup_only && (dcheck == NORMAL || dcheck == BAD) );
            if ( *dup_only && data_size >= data_string ) {
                buff_reseek();
                length -= data_size;
                idb_buff[0] = '\'';
                out_count = 1;
                while ( data_size ) {
                    data = buff_regetc();
                    idb_buff[out_count++] = (char)data;
                    --data_size;
                }
                idb_buff[out_count++] = '\'';
                idb_buff[out_count] = '\0';
                if ( depth != 1 ) {
                    *dup_only = TRUE;
                }
                strcat( dup_buff, idb_buff );
                strcat( struc_buff, idb_buff );
            } else {
                /*
                ** Was it a fixup?
                */
                inst_offset -= data_size * mult;
                buff_reseek();      /* Back to start, string look */
                                    /* may have moved ptr         */
                if ( data_check( 0 ) == FIXUP ) {
                    if ( depth != 1 ) {
                        *dup_only = TRUE;
                    }
                    if ( fix_rec->form == POINTER ) {
                        fix_size = get_fix( operand, 2, FALSE, segment_bytes,
                                                    FAR, FALSE, &dummy, DS );
                    } else {
                        fix_size = get_fix( operand, 2, FALSE, segment_bytes,
                                                    NEAR, FALSE, &dummy, DS );
                    }
                    inst_offset += fix_size * mult;
                    length -= fix_size;
                    if ( pass == 3 ) {
                        strcat( dup_buff, operand );
                        strcat( struc_buff, operand );
                    }
                    buff_empty();
                } else {
                    /*
                    ** Nope, not a string.  Leave one or multiple bytes
                    */
                    data_size = length;
                    if ( data_size != 2 && data_size != 4 && data_size != 6
                                        && data_size != 8 && data_size != 10 ) {
                        data_size = 1;
                    }
                    strcat( dup_buff, "0" );
                    if ( depth == 1 ) {
                        strcat( struc_buff, "0" );
                    }
                    out_count = 0;
                    while ( out_count < data_size ) {
                        rev_buff[ out_count ] = (uchar)buff_regetc();
                        out_count++;
                    }
                    out_count = data_size;
                    while ( out_count ) {
                        --out_count;
                        if ( depth != 1 ) {
                            if ( rev_buff[out_count] != 0 ) {
                                *dup_only = TRUE;
                            }
                        }
                        sprintf( idb_buff, "%02X", rev_buff[out_count] );
                        strcat( dup_buff, idb_buff );
                        if ( depth == 1 ) {
                            strcat( struc_buff, idb_buff );
                        }
                    }
                    strcat( dup_buff, "h" );
                    if ( depth == 1 ) {
                        strcat( struc_buff, "h" );
                    }

                    length -= data_size;
                    inst_offset += mult * data_size;
                }
            }
            if ( length ) {
                strcat( dup_buff, "," );
            }
        }
    }

    if ( repeat != 1 || num_blocks != 0 ) {
        strcat( dup_buff, ")" );
    }

    return( result );
}


int proc_label()
{
    char            text[50];
    dword   past_seg;
    char            *size_text;

    if ( pub_rec->type == FAR ) { 
        size_text = "THIS FAR";
    } else {
        size_text = "THIS NEAR";
    }

    if ( pass == 3 ) {
        if ( pub_rec->offset >= seg_rec->length ) {
            past_seg = pub_rec->offset - seg_rec->length;
            if ( past_seg == 0 ) {
                strcpy( text, "$" );
            } else {
                sprintf( text, "$ + 0%04Xh", size_text, past_seg );
            }
            out_line( pub_rec->name, "=", text, "" );
        } else {
            if ( pub_rec->type == FAR ) {
                out_line( pub_rec->name, "equ", "THIS FAR", "" );
            } else {
                if ( pub_rec->type == NEAR ) {
                    /* Put a colon ':' on labels within code segments */
                    sprintf( text, "%s:", pub_rec->name );
                    out_label( text );
                } else {
                    out_label( pub_rec->name );
                }
            }
        }
    }
    last_pub_rec = pub_rec;
    pub_advance();
    if ( pub_rec->seg_idx == last_pub_rec->seg_idx 
            && pub_rec->offset == last_pub_rec->offset ) {
        if ( pass == 3 ) {
            if ( last_pub_rec->type != NEAR ) {
                out_directive( "equ" );
                sprintf( text, "%s", size_text );
                out_operand( text );
            }
            out_endline();
        }
        proc_label();
    }
    return( 0 );
}

int proc_fixup()
{
    int     fix_size;
    int     fix_form;
    int     fix_extended;
    char    operand[100];
    int     dummy;

    fix_form     = fix_rec->form;
    fix_extended = fix_rec->extended;

    if ( fix_form == POINTER ) {
        fix_size = get_fix( operand, 2, FALSE, segment_bytes,
                                                    FAR, FALSE, &dummy, DS );
    } else {
        fix_size = get_fix( operand, 2, FALSE, segment_bytes,
                                                    NEAR, FALSE, &dummy, DS );
    }

    if ( pass == 3 ) {
        if ( fix_form == OFFSET && fix_extended ) {
            out_directive( "dd" );
        } else {
            if ( fix_form == POINTER && fix_extended ) {
                out_directive( "df" );
            } else {
                out_directive( fix_type[fix_form].form );
            }
        }
    }
    inst_offset += fix_size;

    if ( pass == 3 ) {
        out_operand( operand );
        out_endline();
    }
    buff_empty();
    if ( last_pub_rec ) {
        last_pub_rec->type = size_to_type(fix_size);
        last_pub_rec = NULL;
    }
    return( fix_size );
}


static void out_var(LOCAL_VAR*pVar)
{
  char *p;
  char temp[50];
  char text[80];

  sprintf(text, "\"%s\"",pVar->vname);
  switch (pVar->class) {
  case 7:
       out_line("; (instance of typed variable)",text,"","");
       return;
  case 6:                           /* local typedef ? */
       out_line("; (local typedef)", text, "","");
       return;
  case 4:                          /* register variable */
       switch (pVar->bInfo1) {
       case 0x06: p = "si"; break;
       case 0x07: p = "di"; break;
       default  : p = "[unknown]"; break;
       }
       sprintf(temp,": variable in register %s",p);
       strcat(text, temp);
       out_line(";",text,"","");
       break;
  case 2:                        /* variable */
       if ((pVar->bInfo1&8)==8)
         p = "argument";
       else
         p = "local";
       sprintf(temp,": (var) %s stored in ",p);
       strcat(text,temp);
       switch (pVar->bInfo2) {
       case 1 :                          /* stored on stack */
            if ((int)pVar->wInfo1<0)
              sprintf(temp,"[bp-%04X]",-pVar->wInfo1);
            else
              sprintf(temp,"[bp+%04X]",pVar->wInfo1);
            break;
       default :
            strcpy(temp,"[unknown]");
            break;
       }
       strcat(text,temp);
       out_line(";",text,"","");
       return;
  case 0:                  /* static function */
       out_line(";",temp," (static function) ","");
  }
}


int proc_normal( inst_proc )
    int     inst_proc;
{
    int      data_size;
    int      data;
    int      inst_length;
    int      zeros;
    int      string_limit;
    int      dcheck;
    char     temp[3];
    int      this_size;
    int      data_type;
    int      direct;
    LINE_T   Line, *pLine;
    SCOPE_T  Scope, *pEndScope, *pArgScope, *pLocScope, *pScope;
    NODE_T  *pNode;
    LOCAL_VAR *pVar;

    data_type = UNKNOWN;
    /*
    ** Step 0, determine if there is a hint record at this position
    */
    hint_search.seg_idx    = segment;
    hint_search.offset     = inst_offset;
    direct = RIGHT;
    while ( hint_node != hint_tree
            && (direct = hint_compare( &hint_search, hint_rec )) == LEFT ) {
        hint_advance();
        direct = RIGHT;
    }
    if ( direct == EQUAL ) {
        data_type = hint_rec->hint_type;
    }

    if ( data_type == UNKNOWN ) {       /* Default, search for string */
        /*
        ** Step 1, Is it a string?
        */
        data_size = 0;
        zeros = TRUE;
        do {
            data = buff_getc();
            if ( data == EOF ) {
                break;
            }
            if ( data != 0 && data != 0x0A && data != 0x0D && data != 0x1B
                  && (data < ' ' || data > '~') ) {
                break;
            }
            if ( data != 0 ) {
                zeros = FALSE;
            }
            data_size++;
            inst_offset++;
            dcheck = data_check( 0 );
        } while ( dcheck == NORMAL || dcheck == BAD );

        if ( inst_proc ) {
            string_limit = code_string;
        } else {
            string_limit = data_string;
        }
        if ( data_size >= string_limit || (zeros && data_size > 1) ) {
            if ( last_pub_rec && last_pub_rec->type > FAR ) {
                this_size = type_to_size(last_pub_rec->type);
                if ( data_size < this_size ) {
                    last_pub_rec->type = BYTE_PTR;
                }
                data_byte( this_size );
                buff_reseek();
                inst_offset -= data_size;
                inst_offset += this_size;
                last_pub_rec = NULL;
                return( this_size );
            } else {
                /*
                ** Either near, far, or unknown
                */
                if ( last_pub_rec && pass == 3 ) {
                    if ( last_pub_rec->type == FAR ) {
                        out_directive("=");
                        out_operand("$");
                    }
                    if ( last_pub_rec->type == NEAR ) {
                        out_endline();
                    }
                }
                empty_string( data_size );
                buff_reseek();
                last_pub_rec = NULL;
                return( data_size );
            }
        }
        buff_reseek();
        inst_offset -= data_size;
    }

    if ( data_type == NEAR || (inst_proc && data_type == UNKNOWN) ) {
        data = (uchar)buff_getc();
#ifdef DEBUG
        printf("Disassembly instruction lookup [%02X]\n", data );
#endif
        hex_finish = TRUE;

        inst_length = (*instr[data].rtn)
                        ((uchar)data,
                        instr[data].text,
                        instr[data].special );
        if ( inst_length == 0 ) {       /* Wrong, it wasn't an instruction */
            data_type = UNKNOWN;
        }
    } else {
        inst_length = 0;
    }

    if ( inst_length ) {
        /*
        ** Check for instruction routine returning the proper number
        ** of bytes eaten from buffer.
        */
        if ( buff_cur - buff_beg != inst_length ) {
#ifdef DEBUG
            printf("PN: Buffer remaining: %04d Length remaining: %04d\n",
                                        buff_cur - buff_beg, inst_length );
#endif
            fmt_error("Disassembly Internal Instruction Processing" );
        }

        /*
        ** Yes, instruction, throw away buffer
        */
        if ( !*hex_comment ) {
            Line.hex_offset = inst_offset;
            pLine = find(&Line, line_tree, linnum_compare, NULL);
            Scope.hex_offset = inst_offset+inst_length;
            if (pass==3) {
              pScope = pEndScope = find(&Scope, end_scope_tree, scope_compare, &pNode);
              pArgScope = find(&Scope, arg_scope_tree, scope_compare, NULL);
              pLocScope = find(&Scope, loc_scope_tree, scope_compare, NULL);
              while (pScope && pEndScope->hex_offset == pScope->hex_offset) {
                out_comment("End of scope");
                pScope = pNode->ptr[LEFT]->data;
                pNode = pNode->ptr[LEFT];
              }
              if (pLocScope) {
                out_line("; Start of scope (locals)","","","");
                pVar = pLocScope->head;
                while (pVar) {
                  out_var(pVar);
                  pVar = pVar->next;
                }
              }
              if (pArgScope) {
                out_line("; Start of scope (arguments)","","","");
                pVar = pArgScope->head;
                while (pVar) {
                  out_var(pVar);
                  pVar = pVar->next;
                }
              }
            }
            if (pLine)
              sprintf(hex_comment, "[%05d] %04X: ", pLine->line_number, inst_offset);
            else
              sprintf(hex_comment, "        %04X: ", inst_offset );
        }
        this_size = inst_length;
        while ( this_size-- ) {
            sprintf( temp, "%02X", (uchar)buff_regetc() );
            strcat( hex_comment, temp );
        }
        if ( hex_finish ) {
            if ( pass == 3 ) {
                if ( hex_output ) {
                    out_comment( hex_comment );
                }
                out_endline();
            }
            *hex_comment = 0;
        }
        inst_offset += inst_length;
        if ( last_pub_rec ) {
            if ( last_pub_rec->type != FAR ) {
                last_pub_rec->type = NEAR;
            }
            last_pub_rec = NULL;
        }
        return( inst_length );
    }

    /*
    ** Ingore any assumes that might have been pending
    */
    abort_assumes();

    /*
    ** Nope, not instruction.  Leave byte
    */
    buff_reseek();      /* Back to start, instruction */
                        /* look may have moved ptr    */
    if ( last_pub_rec && last_pub_rec->type > FAR ) {
        if ( last_pub_rec->type == UNKNOWN ) {
            last_pub_rec->type = BYTE_PTR;      /* Default if not known */
        }
        if ( data_type == UNKNOWN ) {           /* Default if no hint */
            data_type = last_pub_rec->type;
        }
        last_pub_rec->type = data_type;         /* Assign actual type */
    } else {
        if ( data_type == UNKNOWN ) {
            data_type = BYTE_PTR;
        }
    }
    data_size = type_to_size( data_type );
    data_byte( data_size );
    last_pub_rec = NULL;
    inst_offset += data_size;
    return( data_size );
}

void iterated( data_rec, next_rec )
    DAT_T   *data_rec;
    DAT_T   *next_rec;
{
    char            temp[20];
    word    length;
    word    length_used;
    PUB_T           *save_pub_rec;
    int             dup_only;
    int             dummy;
    dword   repeat;
    STRUC_T         *this_struc;

    length = data_rec->length;
    fseek( o_file, data_rec->file_pos, L_SET );
    length = buff_init( length );

    if ( data_check( 0 ) == LABEL ) {
        proc_label();
    }

    save_pub_rec = last_pub_rec;
    if ( last_pub_rec ) {
        if ( pass == 3 ) {
            if ( last_pub_rec->type == FAR ) {
                out_directive("=");
                out_operand("$");
            }
            if ( last_pub_rec->type == NEAR ) {
                out_endline();
            }
        }
    }

    /*
    ** Process iterated data with this record
    */
    strcpy( structure_form, "" );
    if ( data_rec->structure ) {
        /* Processing for structures */
        dup_only = FALSE;
    } else {
        if ( pass == 1 ) {
            dup_only = FALSE;       /* Give 1 chance (pass 1) at structuring */
        } else {
            dup_only = TRUE;
        }
    }

    dup_buff[0]   = '\0';
    strcpy( struc_buff, "<" );

    while ( length > 0 ) {
        length_used = 0;
        data_rec->size = proc_idb( 1, 1L, &dup_only, data_rec->extended,
                                           &length_used, &repeat );
        sprintf( temp, "(%ld,%1d)", repeat, size_to_type(data_rec->size) );
        strcat( structure_form, temp );
        length -= length_used;
        if ( length ) {
            strcat( dup_buff, "," );
            strcat( struc_buff, "," );
        }
    }

    if ( dup_only ) {
        if ( save_pub_rec ) {
            save_pub_rec->type = size_to_type( data_rec->size );
        }
        if ( pass == 3 ) {
            out_directive( size_to_opcode(data_rec->size, &dummy) );
            out_operand( dup_buff );
            out_endline();
        }
    } else {
        /*
        ** Its a structure!
        */
        strcat( struc_buff, ">" );

        if ( pass == 1 ) {
            data_rec->structure = struc_insert( structure_form );
        }

        this_struc = data_rec->structure;

        if ( save_pub_rec ) {
            save_pub_rec->structure = this_struc;
        }

        if ( pass == 3 ) {
            sprintf( temp, "struct_%d", this_struc->index );
            out_directive( temp );
            out_operand( struc_buff );
            out_endline();
        }
    }

    next_rec = next_rec;       /* Prevent unused variable warnings */
}


void enumerated( data_rec, next_rec, inst_proc )
    DAT_T           *data_rec;
    DAT_T           *next_rec;
    int             inst_proc;
{
    int             length;
    int             result;
    dword   prev_offset;
    int             addition;
    int             dcheck;

    /*
    ** Process enumerated data within this record
    */
    length = data_rec->length;
    fseek( o_file, data_rec->file_pos+record_overused, L_SET );
    length = buff_init( length-record_overused );
    addition = 0;
    if ( next_rec ) {
        fseek( o_file, next_rec->file_pos, L_SET );
        addition = 0x10;      /* Longest instruction can be 0x10 bytes long */
        if ( addition > next_rec->length ) {
            addition = next_rec->length;
        }
        length += buff_add( addition );
    }
    while ( length > addition ) {
#ifdef DEBUG
        printf( "DIS-ASSEMBLY processing enumerated %08lX %04X %04X %04X %04X\n", 
                    data_rec, data_rec->offset, data_rec->length, length, 
                    inst_offset );
#endif
        if ( length < 0 ) {
            fmt_error( "Dis-assembly Record Over-run" );
        }

        prev_offset = inst_offset;

        dcheck = data_check( 0 );

        switch ( dcheck ) {
            case BAD:       result = proc_normal( inst_proc );      break;
            case LABEL:     result = proc_label();                  break;
            case FIXUP:     result = proc_fixup();                  break;
            case NORMAL:    result = proc_normal( inst_proc );      break;
        }

        length -= result;

        if ( (dword)result != inst_offset - prev_offset ) {
#ifdef DEBUG
            printf("Instruction Length was %ld, delta IP = %ld\n",
                                   (long)result, inst_offset - prev_offset );
#endif
            fmt_error("Dis-assembly Instruction Size Error");
        }

        if ( data_rec->offset + data_rec->length - length + addition 
                                                        != inst_offset ) {
            fmt_error("Dis-assembly Instruction Mis-alignment");
        }

        if ( buff_end - buff_cur != length ) {
#ifdef DEBUG
            printf("Buffer remaining: %04d Length remaining: %04d\n", 
                                            buff_end - buff_cur, length );
#endif
            fmt_error("Dis-assembly Internal Instruction Processing" );
        }
    }
    record_overused = addition - length;
}

void proc_drec( data_rec, next_rec, inst_proc )
    DAT_T   *data_rec;
    DAT_T   *next_rec;
    int     inst_proc;
{
    int     orgable;

#ifdef DEBUG
    printf("DIS-ASSEMBLY offset %04X\n", data_rec->offset );
#endif
    if ( pass == 3 ) {
        out_newline();
    }
    /*
    ** Do we need an ORG statement or dup(?)'s ?
    */
    if ( inst_offset != data_rec->offset+record_overused ) {
        pub_search.seg_idx = segment;
        pub_search.offset = data_rec->offset+record_overused;
        orgable = TRUE;
        while ( pub_node != public_tree 
                && pub_compare(&pub_search,pub_rec) == LEFT ) {
            pre_dups( pub_rec->offset - inst_offset, orgable );
            proc_label();
            orgable = FALSE;
        }
        pre_dups( data_rec->offset+record_overused - inst_offset, orgable );
    }
    data_offset = data_rec->offset;

    switch( data_rec->type ) {
        case ENUMERATED:
            enumerated( data_rec, next_rec, inst_proc );
            break;
        case ITERATED:
            iterated( data_rec, next_rec );
            break;
    }
}


NODE_T *list_sex( node, seg_num )
    NODE_T  *node;
    int     seg_num;
{
    SEX_T   *sex_rec;
    EXT_T   *ext_rec;

    sex_rec = (SEX_T *)node->data;
    while ( node != sex_tree && sex_rec->seg_index < seg_num ) {
        node = traverse( node, RIGHT );
        sex_rec = (SEX_T *)node->data;
    }
    out_newline();
    while ( node != sex_tree && sex_rec->seg_index == seg_num ) {
        ext_rec = sex_rec->ext_rec;
        if ( ext_rec->used == seg_num ) {       /* Still in this segment? */
            print_ext( sex_rec->ext_rec );
        }
        node = traverse( node, RIGHT );
        sex_rec = (SEX_T *)node->data;
    }
    return( node );
}

void process_segment(void)
{
    NAME_T          *name_rec;
    DAT_T           *next_rec;
    int             inst_proc;
    int             seg_reg;

    segment = seg_rec->index;

    /*
    ** Skip segments which have no data, no external definitions,
    ** and no public symbols
    */
    if ( seg_rec->length == 0
  && (sex_node == sex_tree || ((SEX_T *)sex_node->data)->seg_index > segment)
  && (pub_node == public_tree || ((PUB_T *)pub_node->data)->seg_idx > segment)
        ) {
        return;
    }

    for ( seg_reg = 0; seg_reg < MAX_SEG_REGS; seg_reg++ ) {
        seg_rec->new_mode [seg_reg] = 0;
        seg_rec->new_index[seg_reg] = 0;
        seg_rec->prv_mode [seg_reg] = 0;
        seg_rec->prv_index[seg_reg] = 0;
    }

    /* Move to the hint records for this segment */
    while ( hint_node != hint_tree && hint_rec->seg_idx < segment ) {
        hint_node = traverse( hint_node, RIGHT );
    }

    name_search.index = seg_rec->name;      /* Look up segment name */
    name_rec = find( &name_search, name_tree, TC name_compare, NULL );
    if ( name_rec == NULL ) fmt_error( "Lost Code Segment Name (Oops!)" );
    cseg_name = name_rec->name;

#ifdef DEBUG
    printf("DIS-ASSEMBLY pass %d: segment %s\n", pass, cseg_name);
#endif

    if ( seg_rec->bit32 ) {
        segment_mode = 386;
    } else {
        segment_mode = 286;
    }

    if ( segment_mode == 386 ) {
        segment_bytes = 4;
    } else {
        segment_bytes = 2;
    }

    if ( pass == 3 ) {
        out_newline();
        if ( seg_rec->code ) {
            if ( compatibility == 2 ) {
                out_line( cseg_name, "CSEG", "", "" );
            } else {
                out_line( cseg_name, "SEGMENT", "", "" );
                seg_rec->new_mode[1] = SEGMENT;     /* Default assume for CS: */
                seg_rec->new_index[1] = segment;
                adjust_assumes();
            }
        } else {
            if ( compatibility == 2 ) {
                if ( stricmp(seg_rec->class->name,"CODE") == 0 ) {
                    out_line( cseg_name, "CSEG", "", "" );
                } else
                if ( stricmp(seg_rec->class->name,"DATA") == 0 ) {
                    out_line( cseg_name, "DSEG", "", "" );
                } else
                if ( stricmp(seg_rec->class->name,"STACK") == 0 ) {
                    out_line( cseg_name, "SSEG", "", "" );
                } else
                if ( stricmp(seg_rec->class->name,"EXTRA") == 0 ) {
                    out_line( cseg_name, "ESEG", "", "" );
                } else {
                    out_line( cseg_name, seg_rec->class->name, "", "" );
                }
            } else {
                out_line( cseg_name, "SEGMENT", "", "" );
            }
        }
        sex_node = list_sex( sex_node, segment );   /* List local extrns */
    }

#if 0
    sprintf( operand, "; Segment mode is %d\n", segment_mode );
    if ( pass == 3 ) {
        out_line( "", operand, "", "" );
    }
#endif

    inst_proc = seg_rec->code;

    last_pub_rec = NULL;
    inst_offset = 0;                        /* Start at beginning of seg */

    inst_init();

    /*
    ** Loop through all data records for this segment
    */
    if ( data_node != data_tree ) {
        while ( data_rec->seg_idx == segment ) {
            /*
            ** Advance to next data record
            */
            data_node = traverse( data_node, RIGHT );
            /*
            ** Check if we can process this record with extra bytes from
            ** the next record.  This is to catch any instructions which
            ** span the record edges. (Only for ENUMERATED records)
            */
            next_rec = (DAT_T *)data_node->data;
            if ( data_node != data_tree && data_rec->type == ENUMERATED &&
                    next_rec->type == ENUMERATED &&
                    next_rec->seg_idx == segment && 
                    data_rec->offset + data_rec->length == next_rec->offset ) {
                proc_drec( data_rec, next_rec, inst_proc );
            } else {
                proc_drec( data_rec, NULL, inst_proc );
            }
            data_rec = next_rec;
        }
    }

    if ( pass == 3 && pub_rec->seg_idx == segment ) {
        out_newline();                  /* Blank line before trailings */
    }
    while ( pub_rec->seg_idx == segment ) {
        pre_dups( pub_rec->offset - inst_offset, FALSE );
        proc_label();
    }

    pre_dups( seg_rec->length - inst_offset, FALSE );
        
    if ( pass == 3 ) {
        out_newline();
        if ( compatibility == 2 ) {
        } else {
            out_line( cseg_name, "ENDS", "", "" );
        }
    }
}


void process()
{
    NODE_T          *seg_node;
    char            text[80];
    int             dummy;

    pub_node  = start( public_tree, RIGHT );    /* Start at beginning of */
    pub_rec   = (PUB_T *)pub_node->data;        /* Public symbols list   */

    fix_node  = start( fix_tree, RIGHT );       /* Start at beginning of */
    fix_rec   = (FIX_T *)fix_node->data;        /* Fixup records list    */

    data_node = start( data_tree, RIGHT );      /* Start at beginning of */
    data_rec = (DAT_T *)data_node->data;        /* Data record list      */

    hint_node  = start( hint_tree, RIGHT );     /* Start at beginning of */
    hint_rec   = (HINT_T *)hint_node->data;     /* Hints list            */

    sex_node = start( sex_tree, RIGHT );        /* Start at beginning of */

    if ( pass == 3 && !processor_type_comment_occurred ) {
        out_line( "", ".286p", "", "" );
        out_line( "", ".287", "", "" );
        processor_mode = 286;
    }

    /*
    ** Display all defined public values ( segment == 0 )
    */
    segment = 0;
    if ( pass == 3 && pub_rec->seg_idx == segment ) {
        out_newline();                  /* Blank line before trailings */
    }
    while ( pub_rec->seg_idx == segment ) {
        lone_public();                  /* Finish off any trailing */
        pub_advance();                  /* lone public labels      */
    }

    /*
    ** Loop through all segments
    */
    segment_mode = processor_mode;
    seg_node = start( segment_tree, RIGHT );
    while ( seg_node != segment_tree ) {
        seg_rec = (SEG_T *)seg_node->data;
        process_segment();
        seg_node = traverse( seg_node, RIGHT );
    }

    /*
    ** Finish off this module
    */
    fix_search.seg_idx    = 0;
    fix_search.dat_offset = 0;
    fix_search.offset     = 1;          /* Look for MODEND record */

    fix_rec = (FIX_T *)find( (char *)&fix_search, fix_tree, TC fix_compare, NULL );
    if ( fix_rec == NULL ) {
        if ( pass == 3 ) {
            out_newline();
            out_opcode("END");
            out_newline();
        }
    } else {
        get_target( text, fix_rec, 0, 0L, FALSE, 0, FALSE, &dummy, CS );
        if ( pass == 3 ) {
            out_newline();
            out_line( "", "END", text, "Module starting address" );
        }
    }
}
