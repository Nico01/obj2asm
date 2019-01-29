#include <stdio.h>
#include <string.h>
#include "o.h"

int pub_compare( PUB_T *, PUB_T * );

int pub_compare( rec_1, rec_2 )
    PUB_T  *rec_1;
    PUB_T  *rec_2;
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

NODE_T *pub_insert( seg_idx, offset, this_name, domain, scope )
    int             seg_idx;
    dword   offset;
    char            *this_name;
    int             domain;
    int             scope;
{
    PUB_T   *public_rec;

    public_rec = (PUB_T *)o_malloc( sizeof(PUB_T) );
    public_rec->seg_idx   = seg_idx;
    public_rec->offset    = offset;
    public_rec->type      = UNKNOWN;                /* Default to UNKNOWN */
    public_rec->domain    = domain;
    public_rec->scope     = scope;
    public_rec->structure = NULL;
    strcpy( public_rec->name, this_name );

    return ( insert( (char *)public_rec, public_tree, TC pub_compare ) );
}

void pubdef(size_t length, int scope)
{
    int             grp_idx;
    int             seg_idx;
    int             frame;
    int             typ_idx;
    dword   offset;
    char            pub_name[41];

    length -= get_index( &grp_idx );
    length -= get_index( &seg_idx );

    if ( seg_idx == 0 ) {
        frame = get_word();
        length -= 2;
    }

    --length;
    while ( length ) {
        length -= get_name( pub_name );

#ifdef DEBUG
printf("%s\n", pub_name );
#endif

        offset = (dword)get_word();

        pub_insert( seg_idx, offset, pub_name, PUBLIC, scope );
        length -= 2;
        length -= get_index( &typ_idx );
    }
    frame = frame;
}
