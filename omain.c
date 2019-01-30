/*
** OBJASM - Utility to create .ASM files out of .OBJ files.
**          Options are:
**
**              -4 = Make compatible with MASM v4.0 (no retf)
**              -a = Add labels for un-named data references
**              -h = Hex output as comments
**              -v = 486 instructions
**             -s# = Minimum string size in data segment
**             -c# = Minimum string size in code segment
**    -f(filename) = Additional information filename (w/paren.)
**
**    Includes 8086/80186/80286/80386/80486
**    and 8087/80287/80387 coprocessor instructions
**    See OBJASM.DOC for a more detailed description.
**    (C) Copyright 1988,1989,1990,1991 by Robert F. Day, All rights reserved.
**
**          Send/phone questions, comments, and bugs to:
**
**              Robert F. Day
**              19906 Filbert Dr.
**              Bothell, WA 98012
**              (206) 481-8431
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <alloc.h>
#include "o.h"

                                    /*--- Expanded INTEL OMF record types --*/
#define RHEADR  0x6E                /*   R-Module Header Record             */
#define REGINT  0x70                /*   Register Initialization Record     */
#define REDATA  0x72                /*   Relocatable Enumerated Data Record */
#define RIDATA  0x74                /*   Relocatable Iterated Data Record   */
#define OVLDEF  0x76                /*   Overlay Definition Record          */
#define ENDREC  0x78                /*   End Record                         */
#define BLKREC  0x7A                /*   Block Definition Record            */
#define BKLEND  0x7C                /*   Block End Record                   */
#define DEBSYM  0x7E                /*   Debug Symbols Record               */
#define THEADR  0x80                /* x T-Module Header Record             */
#define LHEADR  0x82                /*   L-Module Header Record             */
#define PEDATA  0x84                /*   Physical Enumerated Data (?)       */
#define PIDATA  0x86                /*   Physical Iterated Data (?)         */
#define COMENT  0x88                /* x Comment Record                     */
#define MODEND  0x8A                /* x Module End Record                  */
#define MODENDL 0x8B                /* l Module End Record                  */
#define EXTDEF  0x8C                /* x External Names Definition Record   */
#define TYPDEF  0x8E                /*   Type Definitions Record            */
#define PUBDEF  0x90                /* x Public Names Definition Record     */
#define PUBDEFL 0x91                /* l Public Names Definition Record     */
#define LOCSYM  0x92                /*   Local Symbols Record               */
#define LINNUM  0x94                /*   Line Numbers Record                */
#define LNAMES  0x96                /* x List of Names Record               */
#define SEGDEF  0x98                /* x Segment Definition Record          */
#define SEGDEFL 0x99                /* l Segment Definition Record          */
#define GRPDEF  0x9A                /* x Group Definition Record            */
#define FIXUPP  0x9C                /* x Fix-Up Record                      */
#define FIXUPPL 0x9D                /* l Fix-Up Record                      */
#define LEDATA  0xA0                /* x Logical Enumerated Data            */
#define LEDATAL 0xA1                /* l Logical Enumerated Data            */
#define LIDATA  0xA2                /* x Logical Iterated Data              */
#define LIDATAL 0xA3                /* l Logical Iterated Data              */
#define LIBHED  0xA4                /*   Library Header Record              */
#define LIBNAM  0xA6                /*   Library Module Names Record        */
#define LIBLOC  0xA8                /*   Library Module Locations Record    */
#define LIBDIC  0xAA                /*   Library Dictionary Record          */
#define COMDEF  0xB0                /* m Communal Data Definition Record    */
#define LEXTDEF 0xB4                /* m Local External Definition          */
#define LPUBDEF 0xB6                /* m Local Public Definition            */
#define LPUBDF2 0xB7                /* m Local Public Definition (2nd case?)*/
#define LCOMDEF 0xB8                /* m Local Communal Data Definition     */
#define LIBHDR  0xF0                /* m Library Header Record              */
#define LIBEND  0xF1                /* m Library Trailer Record             */
                                    /* x = Intel OMF used by Microsoft      */
                                    /* m = Microsoft Additions to Intel OMF */
                                    /* l = Later extensions by Microsoft    */
/*
** Local Prototypes
*/

FILE    *o_file;                    /* .OBJ file that we are processing */
long    o_position;                 /* Position in file of current data */

NODE_T  *line_tree;
NODE_T  *arg_scope_tree;
NODE_T  *loc_scope_tree;
NODE_T  *end_scope_tree;

NODE_T  *name_tree;
NODE_T  *segment_tree;
NODE_T  *group_tree;
NODE_T  *public_tree;
NODE_T  *extern_tree;
NODE_T  *sex_tree;
NODE_T  *data_tree;
NODE_T  *struc_tree;
NODE_T  *fix_tree;
NODE_T  *hint_tree;
NODE_T  *type_tree;
NODE_T  *block_tree;

SEG_T   *seg_rec;
SEG_T   seg_search;
GRP_T   grp_search;

NODE_T  *pub_node;
PUB_T   *pub_rec;
PUB_T   *last_pub_rec;
PUB_T   pub_search;

NODE_T  *hint_node;
HINT_T  *hint_rec;
HINT_T  hint_search;

THREAD_T threads[2][4] = {0};

NODE_T  *fix_node;
FIX_T   *fix_rec;
FIX_T   fix_search;

NAME_T  name_search;

int      label_count = 0;

int             segment;
dword   inst_offset;
int             processor_mode = 0;
int             segment_mode;
int             segment_bytes;

char            *cseg_name;
PUB_T           *start_pub = NULL;

word    code_string = 40;   /* Stringizing Limit in Code */
word    data_string =  3;   /* Stringizing Limit in Data */

int     pass;                       /* 1=Building Labels, 2=Generating Output */
int     processor_type_comment_occurred = FALSE;    /* No processor spec yet */

int     hex_finish;                 /* For hex comments */

int     tab_offset = 0;             /* Current tab offset, 0 = none */

int     compatibility = 0;          /* 0 = MASM 5.0+ (Microsoft) */
                                    /* 1 = MASM 4.0  (Microsoft) */

int     add_labels = FALSE;         /* No, treat as .obj translator       */
                                    /* Otherwise, treat as .asm creator   */  
                                    /* Translator is a strict translation */

int     hex_output = FALSE;         /* Display hex code as comments? */

char    extra_filename[65] = {'\0'};/* Additional information file */

/*
** ------- Exit values -------- **
** 0 = Successful operation     **
** 1 = Improper Usage           **
** 2 = Unable to open OBJ file  **
** 3 = Premature end of file    **
** 4 = Out of memory            **
** 5 = .OBJ Format error        **
** 6 = Unable to open ADD file  **
** 7 = Syntax error in ADD file **
** ------- Exit values -------- **
*/



int main(int argc, char *argv[])
{
    char            *argp;
    int             name_arg;
    int             argi;
    int             bad_args;
    char            *fnamep;
    dword   position;
    int             at_eof;
    int             rec_type;
    word    rec_length;
    char            temp_name[50];
    char            ch;
    int             i486;

    bad_args = FALSE;
    argi = 1;
    name_arg = 0;
    i486 = FALSE;

    while ( argi < argc ) {
        argp = argv[argi];
        if ( *argp++ == '-' ) {
            while ( *argp ) {
                switch ( *argp ) {
                    case 'a':
                        add_labels = TRUE;
                        break;
                    case '4':
                        compatibility = 1;
                        break;
                    case 'h':
                        hex_output = TRUE;
                        break;
                    case 'v':
                        i486 = TRUE;
                        break;
                    case 'c':
                        code_string = atoi(argp+1);
                        while ( (ch = *(argp+1)) != '\0' ) {
                            if ( ch < '0' || ch > '9' ) {
                                break;
                            }
                            argp++;
                        }
                        break;
                    case 's':
                        code_string = atoi(argp+1);
                        while ( (ch = *(argp+1)) != '\0' ) {
                            if ( ch < '0' || ch > '9' ) {
                                break;
                            }
                            argp++;
                        }
                        break;
                    case 'f':
                        argp++;
                        if ( *argp != '(' ) {
                            bad_args = TRUE;
                            break;
                        }
                        fnamep = extra_filename;
                        while ( (ch = *(argp+1)) != '\0' ) {
                            argp++;
                            if ( ch == ')' ) {
                                *fnamep = '\0';
                                break;
                            }
                            *fnamep++ = ch;
                        }
                        if ( ch == '\0' ) {
                            bad_args = TRUE;
                        }
                        break;
                    default:
                        bad_args = TRUE;
                        break;
                }
                argp++;
            }
        } else {
            if ( name_arg == 0 ) {
                name_arg = argi;
            } else {
                bad_args = TRUE;
            }
        }
        argi++;
    }

    if ( name_arg == 0 ) {
        bad_args = TRUE;
    }

    if ( bad_args ) {
        fprintf( stderr, "Usage:  %s [-options] [objfilename]\n", argv[0] );
        fprintf( stderr, "where -options are:\n" );
        fprintf( stderr, "           -4 Make MASM 4.0 compatible (no RETF)\n" );
        fprintf( stderr, "           -a Add labels for un-named data references\n");
        fprintf( stderr, "           -h Hex output in comments\n");
        fprintf( stderr, "           -v Include 486 instructions\n" );
        fprintf( stderr, "          -c# Minimum string size in a code segment (default=40)\n");
        fprintf( stderr, "          -s# Mimimum string size in a data segment (default=3)\n");
        fprintf( stderr, " -f(filename) Additional information filename (w/paren.)\n");
        fprintf( stderr, "\n" );
        fprintf( stderr, "Additional information file lines:\n");
        fprintf( stderr, "SEG sname CODE                named segment is a code segment\n");
        fprintf( stderr, "SEG sname DATA                named segment is a data segment\n");
        fprintf( stderr, "var=sname:####                creates a local label in segment name\n");
        fprintf( stderr, "sname:####:DB/DW/DD/DF/DQ/DT  directs dis-assembly into data\n");
        fprintf( stderr, "... (for more info, read OBJASM.DOC)\n");
        exit(1);
    }

    switch( compatibility ) {
        case 0:             /* MASM 5.0+ */
            if ( i486 ) {
                ex_instr[0x08].text = "invd";
                ex_instr[0x09].text = "wbinvd";
                op_grp[6][7] = "invlpg";
                ex_instr[0xA6].text = "cmpxchg";
                ex_instr[0xA7].text = "cmpxchg";
                ex_instr[0xC0].text = "xadd";
                ex_instr[0xC1].text = "xadd";
                ex_instr[0xC8].text = "bswap";
                ex_instr[0xC9].text = "bswap";
                ex_instr[0xCA].text = "bswap";
                ex_instr[0xCB].text = "bswap";
                ex_instr[0xCC].text = "bswap";
                ex_instr[0xCD].text = "bswap";
                ex_instr[0xCE].text = "bswap";
                ex_instr[0xCF].text = "bswap";

            }
            break;
        case 1:             /* MASM 4.0  */
            /*
            ** Make far return only a comment
            */
            instr[0xCB].text = "ret\t; (retf)";
            break;
    }

    strcpy( temp_name, argv[name_arg] );
    if ( strchr(temp_name,'.') == NULL ) {  /* Append ".obj" if no extension */
        strcat( temp_name, ".obj" );        /* is supplied                   */
    }

    o_file = fopen( temp_name, "rb" );
    if ( o_file == NULL ) {
        fprintf( stderr, "%s: Cannot open %s\n", argv[0], temp_name );
        exit(2);
    }

    /* Print copyright message */
    printf("; OBJASM version 2.0 released on Jan 3, 1991\n");
    printf("; (C) Copyright 1988,1989,1990,1991 by Robert F. Day.  All rights reserved\n\n");

    init_trees();                   /* Initialize all trees */

    at_eof = FALSE;
    position = 0;

    while ( !at_eof ) {
        fseek( o_file, position, L_SET );

        rec_type = fgetc( o_file );

        if ( rec_type == EOF ) {
            at_eof = TRUE;
        } else {
            //size_t rec_length = getw( o_file );
            fread(&rec_length, 1, 2, o_file);

            o_position = position + 3;      /* (1 rec_type) + (2 rec_length) */

#ifdef DEBUG
            printf("PROCESSING rec_type [%02X] rec_length [%04X]\n",
                                                    rec_type, rec_length );
#endif

            switch( rec_type ) {
              case THEADR:  theadr();                       break;
              case LNAMES:  lnames( rec_length );           break;
              case GRPDEF:  grpdef( rec_length );           break;
              case SEGDEF:  segdef();                       break;
              case SEGDEFL: segdef();                       break;
              case PUBDEF:  pubdef( rec_length, TRUE );     break;
              case PUBDEFL: pubdef( rec_length, TRUE );     break;
              case LPUBDEF: pubdef( rec_length, FALSE );    break;
              case LPUBDF2: pubdef( rec_length, FALSE );    break;
              case EXTDEF:  extdef( rec_length, TRUE );     break;
              case LEXTDEF: extdef( rec_length, FALSE );    break;
              case LEDATA:  ledata( rec_length, REGULAR );  break;
              case LEDATAL: ledata( rec_length, LARGER );   break;
              case LIDATA:  lidata( rec_length, REGULAR );  break;
              case LIDATAL: lidata( rec_length, LARGER );   break;
              case FIXUPP:  fixupp( rec_length, REGULAR );  break;
              case FIXUPPL: fixupp( rec_length, LARGER );   break;
              case COMDEF:  comdef( rec_length, TRUE );     break;
              case LCOMDEF: comdef( rec_length, FALSE );    break;
              case MODEND:  modend( rec_length, REGULAR );
                            at_eof = TRUE;                  break;
              case MODENDL: modend( rec_length, LARGER );
                            at_eof = TRUE;                  break;
              case TYPDEF:                                  break;
              case COMENT:  printf("; [%04X]",position);
                            coment( rec_length );           break;
              case LINNUM:  linnum( rec_length );           break;
              default:  printf( "Bad record type: [%08lX:%02X:%04X]\n",
                                position, rec_type, rec_length          );
                        break;
            }
            position += 3 + rec_length;
        }
    }

#ifdef DEBUG
    printf("PROCESSING dis-assembly, pass 1\n");
#endif

    if ( strlen(extra_filename) != 0 ) {
        load_extra( argv[0], extra_filename );
    }

    pass = 1;
    process();                      /* First pass: Processing */

    pass = 2;
    process();                      /* Second pass: Figure out labels */

#ifdef DEBUG
    printf("PROCESSING listing externals\n");
#endif

    list_ext();                     /* Show EXTERNAL's */

#ifdef DEBUG
    printf("PROCESSING listing publics and communal definitions\n");
#endif

    list_pub();                     /* Show PUBLIC's */

    list_struc();                   /* Show STRUC's */

#ifdef DEBUG
    printf("PROCESSING dis-assembly, pass 2 (last)\n");
#endif

    pass = 3;
    process();                      /* Third pass: Output */

    fclose( o_file );

#ifdef DEBUG
    printf("END OF RUN\n");
#endif

    return(0);
}
