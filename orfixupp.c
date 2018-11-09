#include <stdio.h>
#include "o.h"

extern int data_seg_idx;
extern dword data_offset;

extern THREAD_T threads[2][4];

void fix_insert( int, int, int, long, int, int, int, int, int );

int fix_compare( rec_1, rec_2 )
    FIX_T  *rec_1;
    FIX_T  *rec_2;
{
    if ( rec_1->seg_idx > rec_2->seg_idx ) {
        return( LEFT );
    } else {
        if ( rec_1->seg_idx < rec_2->seg_idx ) {
            return( RIGHT );
        } else {
            if ( rec_1->dat_offset > rec_2->dat_offset ) {
                return( LEFT );
            } else {
                if ( rec_1->dat_offset < rec_2->dat_offset ) {
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
    }
}

void fix_insert( offset, relate, form, displacement, a_mode, a_index, 
                                                    b_mode, b_index, extension )
    int     offset;
    int     relate;
    int     form;
    long    displacement;
    int     a_mode;
    int     a_index;
    int     b_mode;
    int     b_index;
    int     extension;
{
    FIX_T   *fixup_rec;

    fixup_rec = (FIX_T *)o_malloc( sizeof(FIX_T) );
    fixup_rec->seg_idx      = data_seg_idx;
    fixup_rec->dat_offset   = data_offset;
    fixup_rec->offset       = offset;
    fixup_rec->relate       = relate;
    fixup_rec->form         = form;
    fixup_rec->displacement = displacement;
    fixup_rec->a_mode       = a_mode;           /* Mode of frame   */
    fixup_rec->a_index      = a_index;          /* Index of frame  */
    fixup_rec->b_mode       = b_mode;           /* Mode of target  */
    fixup_rec->b_index      = b_index;          /* Index of target */
    fixup_rec->extended     = extension;

#ifdef DEBUG
printf("FIXUP:%04X:%08lX:%04X displacement=%08lX\n",
            data_seg_idx,data_offset,offset, displacement );
#endif
    if ( form == OFFSET || form == BASE || form == LOADOFF ) {
        fixup_rec->word_sized = TRUE;
    } else {
        fixup_rec->word_sized = FALSE;
    }

    insert( (char *)fixup_rec, fix_tree, TC fix_compare );
}

int decode_fixup( relate, size, form, offset, extension )
    int             relate;                 /* mode 0=self rel, 1=segment rel */
    int             size;                   /* zero */
    int             form;                   /* LOC value [0-4] */
    int             offset;                 /* offset of data item */
    int             extension;
{
    int             length;
    int             fix;
    int             thread;
    int             a_mode;                 /* Mode of Frame   */
    int             a_index;                /* Index of Frame  */
    int             b_mode;                 /* Mode of Target  */
    int             b_index;                /* Index of Target */
    int             fbit;
    int             tbit;
    int             pbit;
    dword   displacement;
    int             extended;

    extended = FALSE;

    fix = get_byte();

    length = 1;
    fbit   = ( fix & 0x80 ) >> 7;   /* F-bit field */
    a_mode = ( fix & 0x70 ) >> 4;   /* frame index */
    if ( fbit == 1 ) {
        /* Reference a previously loaded thread fixup for location */
        thread = ( a_mode & 0x03 );
        a_index = threads[1][thread].index; /* mode is a thread # */
        a_mode  = threads[1][thread].mode;
    } else {
        /* otherwise, frame is explicitly specified */
        if ( a_mode < 4 ) {                 /* mode is not a thread # */
            length += get_index( &a_index );
        }
    }
    tbit   = ( fix & 0x08 ) >> 3;       /* T-bit field */
    thread = ( fix & 0x07 );
    pbit   = ( thread & 0x04 ) >> 2;    /* P-bit field */
    b_mode = ( thread & 0x03 );         /* Target mode */
    if ( tbit == 1 ) {
        b_index = threads[0][b_mode].index; /* mode is a thread # */
        b_mode  = threads[0][b_mode].mode & 0x03;
    } else {
        length += get_index( &b_index );    /* mode is not a thread # */
    }

    if ( size != 0 && extension ) { /* size is always 0 for 16bit MS langs */
        extended = TRUE;
    }

    if ( pbit == 0 ) {
        displacement = (dword)get_int();
        length += 2;
        if ( extended ) {
            displacement += (dword)get_byte() << 16;
            displacement += (dword)get_byte() << 24;
            length += 2;
        }
    } else {
        displacement = 0L;
    }

    if ( a_mode == 5 ) {    /* don't process this type of fixupp */
        a_mode = b_mode;
        a_index = b_index;
    }

    fix_insert( offset, relate, form, displacement, a_mode, a_index,
                                                b_mode, b_index, extended );
    return( length );
}


void fixupp( length, extension )
    word    length;
    int             extension;
{
    uchar   type;           /* First byte of a FIXUPP packet */
    int             kind;
    int             thread;
    int             locat;
    int             relate;
    int             size;
    int             form;
    int             offset;
    int             mode;
    int             index;

    --length;                           /* Subtract 1 for checksum */

    while ( length ) {
        type = get_byte();
        --length;
        if ( type & 0x80 ) {
            /*
            ** True FixUp aka Explicit FixUp Record
            */
            --length;
            locat = ((int)type << 8) + get_byte();
            relate = ( locat & 0x4000 ) >> 14;  /* mode */
            size   = ( locat & 0x2000 ) >> 13;  /* zero */
            form   = ( locat & 0x1C00 ) >> 10;  /* LOC [0,4] */
            offset = ( locat & 0x03FF );

            length -= decode_fixup( relate, size, form, offset, extension );
        } else {
            /*
            ** Thread FixUp
            */
            kind   = ( type & 0x40 ) >> 6;
            mode   = ( type & 0x1C ) >> 2;
            thread = ( type & 0x03 );
            threads[kind][thread].mode  = mode;
            if ( mode == 0 || thread < 4 ) {
                length -= get_index( &index );
                threads[kind][thread].index = index;
            }
        }
    }
}
