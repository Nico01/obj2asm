#include <stdio.h>
#include "o.h"

NODE_T *new_tree( void *data_ptr, int dup_allowed )
{
    NODE_T  *tree;

    tree = (NODE_T *)o_malloc(sizeof(NODE_T));
    tree->data          = data_ptr;
    if ( dup_allowed ) {            /* Hide the duplicates allowed indicator */
        tree->balance = LEFT;       /* in the tree root's balance!           */
    } else {
        tree->balance = RIGHT;
    }
    tree->ptr   [LEFT ] = 0;        /* Hide the tree depth in left pointer */
    tree->ptr   [RIGHT] = (NODE_T *)tree;
    tree->thread[LEFT ] = TRUE;
    tree->thread[RIGHT] = TRUE;
    return( tree );
}

