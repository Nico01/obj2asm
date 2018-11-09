#include <stdio.h>
#include "o.h"

void theadr()
{
    int     length;
    char    mod_name[41];
    char    mod_temp[43];

    length = get_name( mod_name );
    length = length;

    if ( compatibility == 2 ) {
        sprintf( mod_temp, "'%s'", mod_name );
        out_line( "", "TITLE", mod_temp, "" );
    } else {
        out_line( "", "TITLE", mod_name, "" );
    }
    out_newline();
}

