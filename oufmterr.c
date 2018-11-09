#include <stdio.h>
#include <stdlib.h>
#include "o.h"

void fmt_error( error_msg )
    char    *error_msg;
{
    fprintf( stderr, "Obj format error: %s", error_msg );
    exit( 5 );
}
