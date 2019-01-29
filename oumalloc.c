#include <stdio.h>
#include <stdlib.h>
#include "o.h"

void *o_malloc( word size )
{
    char    *result;

    result = (char *)calloc( 1, size );
    if( result == NULL ) {
        fprintf(stderr, "Out of memory!    [Crrraassshhhhh....]\n" );
        exit(4);
    }
    return( result );
}
