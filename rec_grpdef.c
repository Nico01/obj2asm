#include <stdio.h>
#include <string.h>
#include "o.h"

char grp_operands[100] = {0};

#define GRP_SI  0xFF            /* Segment Index */
#define GRP_EI  0xFE            /* External Index */
#define GRP_SCO 0xFD            /* Segment/Class/Overlay Indices */
#define GRP_LTL 0xFB            /* Load Time Locatable */
#define GRP_ABS 0xFA            /* Absolute */

extern int name_compare( NAME_T *, NAME_T * );
extern int seg_compare( SEG_T *, SEG_T * );

int grp_compare( GRP_T *, GRP_T * );
void grp_insert( int );

int grp_compare( rec_1, rec_2 )
    GRP_T  *rec_1;
    GRP_T  *rec_2;
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

void grp_insert( name_idx )
    int     name_idx;
{
    static  group_count = 0;
    GRP_T   *group_rec;

    group_count++;
    group_rec = (GRP_T *)o_malloc( sizeof(GRP_T) );
    group_rec->index = group_count;
    group_rec->name = name_idx;

    insert( (char *)group_rec, group_tree, TC grp_compare );
}

void grpdef(size_t length)
{
    NAME_T  name_search;
    NAME_T  *group;
    NAME_T  *segment;
    SEG_T   seg_search;
    SEG_T   *seg;
    int     gcd;
    int     name_idx;
    char    *group_text;
    char    *group_name;

    length -= get_index( &name_search.index );

    name_idx = name_search.index;

    group = (NAME_T *)find( (char *)&name_search, name_tree, TC name_compare,
                                                                        NULL );

    if ( group == NULL ) {
        fmt_error("Undefined name index");
    }

    group_text = "GROUP";
    group_name = group->name;
    grp_operands[0] = '\0';

    --length;                  /* To account for check sum at bottom */
    while ( length ) {
        --length;
        gcd = get_byte();
        switch( gcd ) {
            case GRP_SI:  
                length -= get_index( &seg_search.index );
                seg = (SEG_T *)find( (char *)&seg_search,
                                             segment_tree, TC seg_compare, NULL );
                if ( seg == NULL ) fmt_error("Undefined segment" );
                name_search.index = seg->name;
                segment = (NAME_T *)find( (char *)&name_search,
                                         name_tree, TC name_compare, NULL );
                if ( strlen(grp_operands) + strlen(segment->name) > 50 ) {
                    strcat( grp_operands, "\\" );
                    if ( strcmp(group_name,"FLAT") != 0 ) {
                        out_line( group_name, group_text, grp_operands, "" );
                    }
                    group_name = "";
                    group_text = "";
                    grp_operands[0] = '\0';
                }

                strcat( grp_operands, segment->name );
                if ( length ) {
                    strcat( grp_operands, ", " );
                }
                break;
          case GRP_EI:
                fprintf( stderr, "Group Component: GRP_EI (External Index)\n");
                fmt_error("Un-implemented GROUP operands");
                break;
          case GRP_SCO:
                fprintf( stderr, "Group Component: GRP_SCO (Seg/Class/Ovly Index)\n");
                fmt_error("Un-implemented GROUP operands");
                break;
          case GRP_LTL:
                fprintf( stderr, "Group Component: GRP_LTL (LTL data)\n");
                fmt_error("Un-implemented GROUP operands");
                break;
          case GRP_ABS:
                fprintf( stderr, "Group Component: GRP_ABS (Absolute)\n");
                fmt_error("Un-implemented GROUP operands");
                break;
          default:
                fprintf( stderr, "Group Component: %02X (?)\n", gcd );
                fmt_error("Un-implemented GROUP operands");
                break;
        }
    }
    if ( strcmp(group_name,"FLAT") != 0 ) {
        out_line( group_name, group_text, grp_operands, "" );
        out_newline();          /* Leave a blank line after Group directive */
    }

    grp_insert( name_idx );
}
