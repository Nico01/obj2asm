#include <stdio.h>
#include "o.h"

NODE_T *start( root_node, direct )
    NODE_T  *root_node;
    int     direct;
{
    NODE_T  *curr_node;
    int     con_direct;

    curr_node = root_node->ptr[RIGHT];
    con_direct = 1 - direct;
    while ( !curr_node->thread[1-direct] ) {
        curr_node = curr_node->ptr[con_direct];
    }
    return( curr_node );
}

NODE_T *traverse( curr_node, direct )
    NODE_T   *curr_node;
    int     direct;
{
    int     con_direct;

    if ( curr_node->thread[direct] ) {
        return( curr_node->ptr[direct] );
    } else {
        curr_node = curr_node->ptr[direct];
        con_direct = 1 - direct;
        while ( !curr_node->thread[1-direct] ) {
            curr_node = curr_node->ptr[con_direct];
        }
        return( curr_node );
    }
}

