#include <stdio.h>
#include <string.h>
#include "o.h"

int struc_compare( STRUC_T *, STRUC_T * );

int struc_compare( rec_1, rec_2 )
    STRUC_T *rec_1;
    STRUC_T *rec_2;
{
    int     result;

    result = strcmp(rec_1->form, rec_2->form);

    if ( result == 1 ) {
        result = LEFT;
    } else {
        if ( result == 0 ) {
            return( EQUAL );
        } else {
            return( RIGHT );
        }
    }
    return( LEFT );     /* ;* */
}

STRUC_T *struc_insert( form )
    char    *form;
{
    STRUC_T *struc_rec;
    NODE_T  *struc_node;
    char    *form_copy;
    int     length;

    struc_rec = (STRUC_T *)o_malloc( sizeof(STRUC_T) );
    length = strlen(form) + 1;
    form_copy = (char *)o_malloc( length );
    strcpy( form_copy, form );
    struc_rec->form  = form_copy;
    struc_rec->index = 0;               /* Not determined yet */

    struc_node = insert( (char *)struc_rec, struc_tree, TC struc_compare );
    return( (STRUC_T *)struc_node->data );
}
