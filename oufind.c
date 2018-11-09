#include <stdio.h>
#include "o.h"

/*
 char   *data;                       pointer to data struct to find
 NODE_T *root_node;                  pointer to root node
 int    (*cmp_routine)(void*,void*); routine used to compare values at 2 nodes
 NODE_T **node_ptr;                  Ptr to node found
*/

void *find( void *data, NODE_T *root_node, int(*cmp_routine)(void*,void*), NODE_T **node_ptr)
{
    NODE_T  *curr_node;
    NODE_T  *child_node;
    int     curr_direct;

    /* Prepare initial value for tree traversal */
    child_node = root_node;

    /*
    **  Traverse tree looking for the place to insert this new node.  If
    **  we find a node which gives a comparison result of 0 (equal), then
    **  we cannot insert this new node.  To indicate this duplicate value,
    **  a non-zero value is returned (the value happens to be a pointer to
    **  node which contains the duplicate value).
    */
    do  {
        curr_node = child_node;

        /*  Compare this new node with the node at this position in the
        **  tree and determine which direction to proceed from here.
        */
        curr_direct = (* cmp_routine)( curr_node->data, data );

        /*  is this node is a duplicate node?
        */
        if ( node_ptr && curr_direct == RIGHT ) {
            *node_ptr = curr_node;
        }
        if ( curr_direct == EQUAL ) {
            if ( node_ptr ) {
                *node_ptr = curr_node;
            }
            return ( curr_node->data );
        }

        child_node = curr_node->ptr[curr_direct];

    } while ( !curr_node->thread[curr_direct]);

    return( NULL );
}
