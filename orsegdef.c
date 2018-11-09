#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "o.h"

extern int name_compare( NAME_T *, NAME_T * );
void upstr( char *, char * );

int seg_compare( SEG_T *, SEG_T * );
void seg_insert( int, NAME_T *, int, int, int );

char    *al_text[] = { "?ALIGN",   "BYTE",   "WORD",   "PARA", 
                         "PAGE",  "DWORD", "?ALIGN", "?ALIGN"  };

char    *cb_text[] = {          "", "MEMORY", "PUBLIC", "?COMBINE",
                        "?COMBINE",  "STACK", "COMMON", "?COMBINE"  };

char    *use_text[] = { "USE16", "USE32" };

int seg_compare( rec_1, rec_2 )
    SEG_T  *rec_1;
    SEG_T  *rec_2;
{
    if ( rec_1->index > rec_2->index ) {
        return( LEFT );
    } else {
        if ( rec_1->index < rec_2->index ) {
            return( RIGHT );
        } else {
            return( EQUAL );
        }
    }
}

void seg_insert( name_idx, class, length, bit32, code )
    int     name_idx;
    NAME_T  *class;
    int     length;
    int     bit32;
    int     code;
{
    static  segment_count = 0;
    SEG_T   *segment_rec;

    segment_count++;
    segment_rec = (SEG_T *)o_malloc( sizeof(SEG_T) );
    segment_rec->index      = segment_count;
    segment_rec->name       = name_idx;
    segment_rec->class      = class;
    segment_rec->length     = length;       /* To infer extra dups          */
    segment_rec->code       = code;
    segment_rec->bit32      = bit32;        /* USE32 segment                */

    insert( (char *)segment_rec, segment_tree, TC seg_compare );
}

void upstr( dest, source )
    char     *dest;
    char     *source;
{
    char     ch;
    while ( (ch = *source) != '\0' ) {
        *dest++ = (char)toupper( ch );
        source++;
    }
    *dest = '\0';
}

void segdef()
{
    NAME_T          search;
    NAME_T          *segment;
    NAME_T          *class;
    NAME_T          *overlay;
    char            operands[80];
    uchar           acbp;
    int             align;
    int             combine;
    int             big;
    int             page;
    int             frame;
    int             ltl;
    uchar           offset;
    int             max_len;
    int             grp_off;
    word            length;
    int             name_idx;
    int             code;

    acbp = get_byte();

    align   = (acbp & 0xE0) >> 5;
    combine = (acbp & 0x1C) >> 2;
    big     = (acbp & 0x02) >> 1;
    page    = (acbp & 0x01);

    if ( align == 0 ) {
        frame  = get_word();
        offset = get_byte();
    }

    if ( align == 6 ) {
        ltl = get_byte();
        max_len = get_word();
        grp_off = get_word();
    }

    length = get_word();                    /* Length of segment */

#ifdef DEBUG
printf("Length of segment = %04X\n",length );
#endif

    operands[0] = '\0';                     /* No operands yet */

    get_index( &search.index );
    segment = (NAME_T *)find( (char *)&search, name_tree, TC name_compare, NULL );
    if ( segment == NULL ) fmt_error( "Undefined segment name" );
    name_idx = search.index;

    get_index( &search.index );
    class = (NAME_T *)find( (char *)&search, name_tree, TC name_compare, NULL );
    if ( class == NULL ) fmt_error( "Undefined class name" );

    get_index( &search.index );
    overlay = (NAME_T *)find( (char *)&search, name_tree, TC name_compare, NULL );
    if ( overlay == NULL ) fmt_error( "Undefined overlay name" );

    if ( compatibility == 0 ) {
        if ( !processor_type_comment_occurred ) {
            out_line( "", ".386p", "", "Enable USE32/USE16 usage" );
            out_line( "", ".387", "", "Enable floating point also" );
            out_newline();
            processor_type_comment_occurred = TRUE;
            processor_mode = 386;
        }

        /* page will now be 0 only for 16-bit segments */
        /* before 32-bit segments                      */

        if ( strlen(class->name) ) {
            if ( processor_mode == 386 ) {
                sprintf( operands, "%s %s %s '%s'", al_text[align],
                        cb_text[combine], use_text[page], class->name );
            } else {
                sprintf( operands, "%s %s '%s'", al_text[align],
                            cb_text[combine], class->name );
            }
        } else {
            if ( processor_mode == 386 ) {
                sprintf( operands, "%s %s %s", al_text[align],
                            cb_text[combine], use_text[page] );
            } else {
                sprintf( operands, "%s %s", al_text[align],
                                              cb_text[combine] );
            }
        }
    } else {
        if ( strlen(class->name) ) {
            sprintf( operands, "%s %s '%s'", al_text[align], 
                                            cb_text[combine], class->name );
        } else {
            sprintf( operands, "%s %s", al_text[align], cb_text[combine] );
        }
    }

    if ( compatibility == 2 ) {
        /*
        ** No pre-output segment listing for RASM86 files
        */
    } else {
        out_line( segment->name, "SEGMENT", operands, "" );
        out_line( segment->name, "ENDS", "", "" );
        out_newline();
    }

    code = FALSE;
    upstr( operands, segment->name );
    if ( strstr(operands,"CODE") ) {
        code = TRUE;
    }
    if ( strstr(operands, "DRIVER") ) {
        code = TRUE;
    }

    upstr( operands, class->name );
    if ( strstr(operands,"CODE") ) {
        code = TRUE;
    }
    if ( strstr(operands,"TEXT") ) {
        code = TRUE;
    }

    seg_insert( name_idx, class, length, page, code );
    big     = big;
    frame = frame;
    offset = offset;
    ltl = ltl;
    max_len = max_len;
    grp_off = grp_off;
}
