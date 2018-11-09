#include <stdio.h>
#include <string.h>
#include "o.h"

int name_compare( NAME_T *, NAME_T * );
void name_insert( char * );

int name_compare( rec_1, rec_2 )
    NAME_T  *rec_1;
    NAME_T  *rec_2;
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

void name_insert( this_name )
    char    *this_name;
{
    static  name_count = 0;
    NAME_T  *name_rec;

    name_count++;
    name_rec = (NAME_T *)o_malloc( sizeof(NAME_T) );
    name_rec->index = name_count;
    strcpy( name_rec->name, this_name );

    insert( (char *)name_rec, name_tree, TC name_compare );
}

void lnames( length )
    word length;
{
    char    mod_name[41];
    char    cksum;

    --length;
    while( length ) {
        length -= get_name( mod_name );
        name_insert( mod_name );
    }
    cksum = get_byte();
    cksum = cksum;
}
