#include <stdio.h>
#include "o.h"

word get_val( dword * );

word get_val( value )
    dword   *value;
{   
    word    byte;
    word    result;

    byte = get_byte();

    switch( byte ) {
        case 0x81:
            *value = (dword)get_word();
            result = 3;
            break;
        case 0x84:
            *value = (dword)get_word()
                     + ((dword)get_byte() << 8);
            result = 4;
            break;
        case 0x88:
            *value = get_long();
            result = 5;
            break;
        default:
            *value = (dword)byte;
            result = 1;
            break;
    }
    return( result );
}

void comdef(size_t length, int scope)
{
    char            name[41];
    int             typ_idx;
    int             var_type;
    dword   num_elements;
    dword   element_size;
    char            cksum;

    --length;
    while( length ) {
        length -= get_name( name );
        length -= get_index( &typ_idx );
        --length;
        var_type = get_byte();
        if ( var_type == 0x61 ) {
            /* MSC v4.0 actually has these values reversed, so you may
               have to reverse the two lines below to generate resonable
               .asm code */
            length -= get_val( &num_elements );
            length -= get_val( &element_size );
        } else {
            length -= get_val( &element_size );
            num_elements = 1L;
        }
        ext_insert( name, 1, var_type, num_elements, 
                                        (word)element_size, scope );
    }
    cksum = get_byte();
    cksum = cksum;
}
