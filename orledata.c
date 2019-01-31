#include <stdio.h>
#include "o.h"

extern int seg_compare( SEG_T *, SEG_T * );

int dat_compare( DAT_T *, DAT_T * );

int data_seg_idx;
dword data_offset;

int dat_compare(DAT_T *rec_1,DAT_T *rec_2)
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

void dat_insert( seg_idx, offset, file_pos, length, extended, type )
    int             seg_idx;
    dword   offset;
    long            file_pos;
    int             length;
    int             extended;
    int             type;
{
    DAT_T           *data_rec;

    data_rec = (DAT_T *)o_malloc( sizeof(DAT_T) );
    data_rec->seg_idx   = seg_idx;
    data_rec->offset    = offset;
    data_rec->type      = type;
    data_rec->size      = 1;             /* Default to DB */
    data_rec->file_pos  = file_pos;
    data_rec->length    = length;
    data_rec->extended  = extended;
    data_rec->structure = NULL;
    insert( (char *)data_rec, data_tree, TC dat_compare );
}

void ledata(size_t length, int extension)
{
    SEG_T seg_search;
    SEG_T *seg;
    uint32_t offset;
    long position;

    position = o_position + length;     /* Position assumed at end of rec */

    length -= get_index( &seg_search.index );

    seg = (SEG_T *)find( (char *)&seg_search, segment_tree, TC seg_compare, NULL );

    if ( seg == NULL ) fmt_error("Undefined segment\n");

    if ( extension == REGULAR ) {
        offset = get_word(); /* Spot to begin within segment */
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
                                            extension, ENUMERATED );
}
