#include <stdio.h>
#include "o.h"

extern int data_seg_idx;
extern dword data_offset;

void modend(size_t length, int extension)
{
    int             type;

    /*
    ** Processing for MODEND records
    */
    type = get_byte();

    if ( type & 0x40 ) {
        data_seg_idx = 0;
        data_offset  = 0L;
        length -= decode_fixup( 1, 0, 1, 1, extension );    /* Seg 0 offset 1 */
    }
}
