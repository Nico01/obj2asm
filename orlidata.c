#include <stdio.h>
#include "o.h"

extern int data_seg_idx;
extern dword data_offset;

void lidata( length, extension )
    word    length;
    int             extension;
{
    SEG_T           seg_search;
    SEG_T           *seg;
    dword   offset;
    long            position;

    position = o_position + length;     /* Position assumed at end of rec */

    length -= get_index( &seg_search.index );
    seg = (SEG_T *)find( (char *)&seg_search, segment_tree, TC seg_compare, NULL );
    if ( seg == NULL ) fmt_error("Undefined segment" );

    if ( extension == REGULAR ) {
        offset = (dword)get_word(); /* Spot to begin within segment */
        length -= 2;
    } else {
        offset = get_long();
        length -= 4;
    }

    data_seg_idx = seg_search.index;    /* Needed by FIXUPPs */
    data_offset  = offset;              /* Ditto */

    position -= length;                 /* Move back remainder of bytes */
    --length;                           /* Ignore checksum in length */

    dat_insert( seg_search.index, offset, position, length, 
                                                    extension, ITERATED );
}

