#include <stdio.h>
#include <stdlib.h>


// ex oufmterr.c
void fmt_error(char *error_msg)
{
    fprintf( stderr, "Obj format error: %s", error_msg );
    exit(5);
}

// ex oumalloc.c
void *o_malloc(size_t size)
{
    char *result = calloc(1, size);

    if (result == NULL) {
        fprintf(stderr, "Out of memory!    [Crrraassshhhhh....]\n");
        exit(4);
    }

    return result;
}
