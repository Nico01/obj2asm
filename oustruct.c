#include <string.h>
#include "o.h"


int struc_compare(STRUC_T *rec_1, STRUC_T *rec_2)
{
    int result = strcmp(rec_1->form, rec_2->form);

    if ( result == 1 ) {
        result = LEFT;
    } else {
        if ( result == 0 ) {
            return EQUAL;
        } else {
            return RIGHT;
        }
    }
    return LEFT;
}

STRUC_T *struc_insert(char *form)
{
    STRUC_T *struc_rec = o_malloc(sizeof(STRUC_T));

    size_t length = strlen(form) + 1;

    char *form_copy = o_malloc(length);

    strncpy(form_copy, form, length);
    struc_rec->form  = form_copy;
    struc_rec->index = 0;               /* Not determined yet */

    NODE_T *struc_node = insert( (char *)struc_rec, struc_tree, TC struc_compare );

    return( (STRUC_T *)struc_node->data );
}
