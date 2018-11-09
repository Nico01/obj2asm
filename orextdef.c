#include <stdio.h>
#include <string.h>
#include "o.h"

int ext_compare( EXT_T *, EXT_T * );

int ext_compare( rec_1, rec_2 )
    EXT_T  *rec_1;
    EXT_T  *rec_2;
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

void ext_insert( this_name, com_ext, var_type, count, size, scope )
    char            *this_name;
    int             com_ext;
    int             var_type;
    dword   count;
    word    size;
    int             scope;
{
    static  extern_count = 0;
    EXT_T   *extern_rec;

    extern_count++;
    extern_rec = (EXT_T *)o_malloc( sizeof(EXT_T) );
    extern_rec->index    = extern_count;
    extern_rec->type     = size_to_type(size);
    extern_rec->pos_abs  = TRUE;
    extern_rec->size     = count;
    extern_rec->com_ext  = com_ext;
    extern_rec->var_type = var_type;
    extern_rec->used     = 0;                   /* Default to never used */
    extern_rec->scope    = scope;
    strcpy( extern_rec->name, this_name );

    insert( (char *)extern_rec, extern_tree, TC ext_compare );
}

void extdef( length, scope )
    word length;
    int          scope;
{
    char    name[41];
    int     typ_idx;
    char    cksum;

    --length;
    while( length ) {
        length -= get_name( name );
        length -= get_index( &typ_idx );

#ifdef DEBUG
printf("%s\n", name );
#endif

        ext_insert( name, 0, 0, 0L, 0, scope );
    }
    cksum = get_byte();
    cksum = cksum;
}
