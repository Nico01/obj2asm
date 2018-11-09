#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "o.h"

#define LINE_SIZE   80

/*
** Local Prototypes
*/
SEG_T *find_seg_by_name( char * );
char *ignore_whitespace( char * );
char *get_text( char *, char * );
char *get_num( char *, dword * );

struct hint_word_s {
    char    *text;
    int     type;
};

struct hint_word_s hint_words[] = {
    { "DB", BYTE_PTR },
    { "DW", WORD_PTR },
    { "DD", DWORD_PTR },
    { "DF", FWORD_PTR },
    { "DQ", QWORD_PTR },
    { "DT", TBYTE_PTR },
};

int hint_nwords = sizeof(hint_words)/sizeof(char *);

int hint_compare( rec_1, rec_2 )
    HINT_T  *rec_1;
    HINT_T  *rec_2;
{
    if ( rec_1->seg_idx > rec_2->seg_idx ) {
        return( LEFT );
    } else {
        if ( rec_1->seg_idx < rec_2->seg_idx ) {
            return( RIGHT );
        } else {
            if ( rec_1->offset > rec_2->offset ) {
                return( LEFT );
            } else {
                if ( rec_1->offset < rec_2->offset ) {
                    return( RIGHT );
                } else {
                    return( EQUAL );
                }
            }
        }
    }
}



void hint_insert( int seg_idx, dword offset, int hint_type, dword length )
{
    HINT_T          *hint_rec;

    hint_rec = (HINT_T *)o_malloc( sizeof(HINT_T) );
    hint_rec->seg_idx   = seg_idx;
    hint_rec->offset    = offset;
    hint_rec->hint_type = hint_type;
    hint_rec->length    = length;
    insert( (char *)hint_rec, hint_tree, TC hint_compare );
}

SEG_T *find_seg_by_name( seg_text )
    char    *seg_text;
{
    SEG_T   *seg_rec;
    NODE_T  *seg_node;
    NAME_T  search;
    NAME_T  *name;

    seg_node = start( segment_tree, RIGHT );
    while ( seg_node != segment_tree ) {
        seg_rec = (SEG_T *)seg_node->data;
        search.index = seg_rec->name;
        name = (NAME_T *)find( (char *)&search, name_tree,
                                            TC name_compare, NULL );
        if ( name == NULL ) fmt_error( "Undefined segment name" );

        if ( strcmp(name->name,seg_text) == 0 ) {
            return( seg_rec );
        }
        seg_node = traverse( seg_node, RIGHT );
    }
    return( NULL );
}

char *ignore_whitespace( start )
    char    *start;
{
    char    *pc;
    char    ch;

    pc = start;
    while ( (ch=*pc) != '\0' ) {
        if ( ch != ' ' && ch != '\t' && ch != '\n' ) {
            break;
        }
        pc++;
    }
    return( pc );
}

char *get_text( destination, start )
    char    *destination;
    char    *start;
{
    char    *pc;
    char    ch;
    int     len;

    len = 0;
    pc = start;
    while ( (ch=*pc) != ' ' && ch != '\t' && ch != '=' && ch != '\0'
            && ch != ':' && ch != '\n' ) {
        *destination++ = ch;
        pc++;
        len++;
        if ( len == NAMESIZE ) {
            break;
        }
    }
    *destination = '\0';
    return( pc );
}

char *get_num( start, value )
    char            *start;
    dword   *value;
{
    char            *pc;
    char            ch;
    dword   dec_form;
    dword   hex_form;
    int             is_hex;
    int             decable;
    int             char_ok;

    dec_form = 0L;
    hex_form = 0L;

    pc = start;
    is_hex = FALSE;
    decable = TRUE;

    while ( (ch=*pc) != '\0' ) {
        char_ok = FALSE;
        if ( ch >= '0' && ch <= '9' ) {
            dec_form *= 10;
            dec_form += ch - '0';
            hex_form *= 16;
            hex_form += ch - '0';
            char_ok = TRUE;
        }
        if ( ch >= 'a' && ch <= 'f' ) {
            decable = FALSE;
            hex_form *= 10;
            hex_form += ch - 'a' + 10;
            char_ok = TRUE;
        }
        if ( ch >= 'A' && ch <= 'F' ) {
            decable = FALSE;
            hex_form *= 10;
            hex_form += ch - 'A' + 10;
            char_ok = TRUE;
        }
        if ( ch == 'h' || ch == 'H' ) {
            is_hex = TRUE;
            pc++;
            break;
        }
        if ( !char_ok ) {
            break;
        }
        pc++;
    }
    if ( is_hex ) {
        *value = hex_form;
    } else {
        if ( decable ) {
            *value = dec_form;
        } else {
            *value = 0L;
        }
    }
    return( pc );
}

void load_extra( exename, filename )
    char            *exename;
    char            *filename;
{
    FILE            *e_file;
    char            temp_name[50];
    char            e_line[LINE_SIZE+1];
    int             line_num;
    char            *pc;
    char            *semicolon;
    char            *equal;
    char            *colon;
    char            seg_text[NAMESIZE+1];
    char            lab_text[NAMESIZE+1];
    int             segment_type;
    dword   disp;
    int             count;

#ifdef DEBUG
    printf("Loading from extra file %s\n", filename );
#endif

    strcpy( temp_name, filename );
    if ( strchr(temp_name,'.') == NULL ) {  /* Append ".add" if not extension */
        strcat( temp_name, ".add" );        /* is supplied                    */
    }
    e_file = fopen( temp_name, "r" );
    if ( e_file == NULL ) {
        fprintf( stderr, "%s: Cannot open %s\n", exename, temp_name );
        exit(6);
    }

    line_num = 1;

    /*
    ** Process lines of format:
    **
    **  SEG segname CODE/DATA                 Pick segment type
    **  labname = segname : offset            Create label
    **  segname : offset : DB/DW/DD/DF/DQ/DT  Control dis-assembly
    */

    while ( fgets(e_line,LINE_SIZE,e_file) != NULL ) {
        semicolon = strchr( e_line, ';' );
        if ( semicolon ) {
            *semicolon = '\0';
        }
        pc = ignore_whitespace( e_line );
#ifdef DEBUG
fprintf(stderr, "%s", pc );
#endif
        if ( strnicmp(pc,"SEG ",4) == 0 ) {
            pc = ignore_whitespace( pc+4 );
            pc = get_text( seg_text, pc );
            pc = ignore_whitespace( pc );
            segment_type = 0;
            if ( strnicmp(pc,"CODE",4) == 0 ) {
                pc += 4;
                segment_type = 1;
            }
            if ( strnicmp(pc,"DATA",4) == 0 ) {
                pc += 4;
                segment_type = 2;
            }
            if ( segment_type == 0 ) {
                fprintf( stderr,
                  "%s: Syntax error on line %d of %s (should be CODE/DATA).\n",
                  exename, line_num, temp_name );
                exit(7);
            }
            seg_rec = find_seg_by_name( seg_text );
            if ( !seg_rec ) {
                fprintf( stderr,
            "%s: Syntax error on line %d of %s (segment '%s' not found)\n",
                    exename, line_num, temp_name, seg_text );
                exit(7);
            }
            if ( segment_type == 1 ) {
                seg_rec->code = TRUE;
            } else {
                seg_rec->code = FALSE;
            }
        }
        equal = strchr(pc, '=' );
        if ( equal ) {
            pc = get_text(lab_text, pc );
            pc = ignore_whitespace(pc);
            if ( strnicmp(pc,"=",1) != 0 ) {
                fprintf( stderr,
            "%s: Syntax error on line %d of %s (expecting '=' after label)\n",
                    exename, line_num, temp_name );
                exit(7);
            }
            pc=ignore_whitespace(pc+1);
            colon=strchr(pc,':');
            if ( colon == NULL ) {
                fprintf( stderr,
            "%s: Syntax error on line %d of %s (expecting ':')\n",
                    exename, line_num, temp_name );
                exit(7);
            }
            pc = get_text( seg_text, pc );
            pc = ignore_whitespace( colon+1 );
            pc = get_num( pc, &disp );
            seg_rec = find_seg_by_name( seg_text );
            if ( !seg_rec ) {
                fprintf( stderr,
            "%s: Syntax error on line %d of %s (segment '%s' not found)\n",
                    exename, line_num, temp_name, seg_text );
                exit(7);
            }
            pub_insert( seg_rec->index, disp, lab_text, LOCAL, FALSE );
        }
        colon = strchr(pc,':');
        if ( colon ) {
            pc = get_text( seg_text, pc );
            seg_rec = find_seg_by_name( seg_text );
            if ( !seg_rec ) {
                fprintf( stderr,
            "%s: Syntax error on line %d of %s (segment '%s' not found)\n",
                    exename, line_num, temp_name, seg_text );
                exit(7);
            }
            pc = ignore_whitespace( pc );
            if ( strnicmp(pc,":",1) != 0 ) {
                fprintf( stderr,
            "%s: Syntax error on line %d of %s (expecting ':' seperator)\n",
                    exename, line_num, temp_name );
                exit(7);
            }
            pc = get_num( pc+1, &disp );
            pc = ignore_whitespace( pc );
            if ( strnicmp(pc,":",1) != 0 ) {
                fprintf( stderr,
            "%s: Syntax error on line %d of %s (expecting ':' seperator)\n",
                    exename, line_num, temp_name );
                exit(7);
            }
            pc = ignore_whitespace( pc+1 );
            pc = get_text( lab_text, pc );  /* Is really data type text */
            count = 0;
            while ( count < hint_nwords ) {
                if ( stricmp(lab_text,hint_words[count].text) == 0 ) {
                    break;
                }
                count++;
            }
            if ( count == hint_nwords ) {
                fprintf( stderr,
            "%s: Syntax error on line %d of %s (expecting DB/DW/DD/DF/DQ/DT)\n",
                    exename, line_num, temp_name );
                exit(7);
            }
            hint_insert(seg_rec->index,disp,hint_words[count].type,1);
        }
        pc = ignore_whitespace( pc );
        if ( strlen(pc) != 0 ) {
                fprintf( stderr,
       "%s: Syntax error on line %d of %s (too many characters on line '%s')\n",
                    exename, line_num, temp_name, pc );
                exit(7);
        }
        line_num++;
    }

#ifdef DEBUG
fprintf(stderr, "\n", e_line );
#endif

    fclose( e_file );
}
