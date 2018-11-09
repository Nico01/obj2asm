#include <stdio.h>
#include "o.h"

void swap( NODE_T *, int, NODE_T *, int );

void swap( node_A, direct_A, node_B, direct_B )
    NODE_T  *node_A;
    int     direct_A;
    NODE_T  *node_B;
    int     direct_B;
{
    /* Swap the pointers (unless they are already threads) */
    if ( node_A->thread[direct_A] ) {
        node_B->ptr[direct_B] = node_A;
        node_B->thread[direct_B] = TRUE;
        node_A->thread[direct_A] = FALSE;
    } else {
        node_B->ptr[direct_B] = node_A->ptr[direct_A];
        node_A->ptr[direct_A] = node_B;
    }
}

/*
  void   *data;                       pointer to data struct to save
  NODE_T *root_node;                  pointer to root node
  int    (*cmp_routine)(void*,void*); routine used to compare values at 2 nodes
*/

NODE_T *insert( void *data, NODE_T *root_node, int (*cmp_routine)(void*,void*))
{
    NODE_T      *insert_node;
    NODE_T      *curr_node;         /* Current node we are visiting      */
    NODE_T      *son_node;
    NODE_T      *grand_son;
    NODE_T      *path[32];          /* 32 levels (2^32 records!) */
    int         direct[32];
    int         level;
    int         curr_direct;
    int         con_direct;

    /* Prepare initial value for tree traversal */
    level    = 0;
    son_node = root_node;

    /*
    **  Traverse tree looking for the place to insert this new node.  If
    **  we find a node which gives a comparison result of 0 (equal), then
    **  we cannot insert this new node.  To indicate this duplicate value,
    **  a pointer to the node which contains the duplicate value is returned.
    */
    do  {
        curr_node = son_node;               /* Advance to new son_node */

        /*  Compare this new node with the node at this position in the
            tree and determine which direction to proceed from here.
        */
        curr_direct = (* cmp_routine)( curr_node->data, data );

        /*  is this node is a duplicate node?
        */
        if ( curr_direct == EQUAL ) {
            if ( root_node->balance == LEFT ) { /* Duplicates allowed? */
                curr_direct = LEFT;             /* see new_tree()!!!!  */
            } else {
                return ( curr_node );
            }
        }

        path[level] = curr_node;
        direct[level] = curr_direct;
        level++;

        son_node = curr_node->ptr[curr_direct];

    } while ( !curr_node->thread[curr_direct]);

    /*  Now, we have found the place to insert this node,
        make the last visited node point to this new node.
    */
    con_direct = 1-curr_direct;

    /* Set up default values for this node which we are going to insert */
    insert_node = (NODE_T *)o_malloc( sizeof(NODE_T) );

    insert_node->data                = data;
    insert_node->balance             = BALANCED;
    insert_node->thread[curr_direct] = TRUE;
    insert_node->thread[con_direct ] = TRUE;
    insert_node->ptr   [curr_direct] = curr_node->ptr[curr_direct];
    insert_node->ptr   [con_direct ] = curr_node;

    curr_node->thread  [curr_direct] = FALSE;
    curr_node->ptr     [curr_direct] = insert_node;


    /*  Now loop through 'un-stacking' the path which we took on our
        way to get this guy inserted.  Keep in mind that this process
        is done in stack-order, so the last node visited will be the
        first node processed, etc.  Also keep in mind that the loop
        may be terminated before back-tracking completely to the root
        node (ie. the entire path is not processed).
    */
    do {
        son_node = curr_node;

        level--;

        /*  If we have gotten back out to the root of the tree,
            then the height of the tree has increase.
        */
        if ( level == 0 ) {         /* Increment depth (see new_tree()!!!) */
            root_node->ptr[LEFT] = (NODE_T *) (((int)root_node->ptr[LEFT])+1);
            break;
        }

        /*  Get path information at this stack level
        */
        curr_node = path[level];
        curr_direct = direct[level];

        /*  If the node at this position along the path was BALANCED,
            then make it lean in the direction we took from it on our
            way down.
        */
        if ( curr_node->balance == BALANCED ) {
            curr_node->balance = curr_direct;
        } else {
            /*  Otherwise, two cases may arise.
                1.  The node was unbalanced in the opposite
                    direction to the direction we took.
                    (In this case that node will become balanced)
            */
            if ( curr_node->balance != curr_direct ) {
                curr_node->balance = BALANCED;
                break;
            } else {
                /* or 2.  The node was already unbalanced in the same
                          direction we took. If this happens we may need to
                          do single or double rotation.  Single rotation
                          happens if the node (which is in the opposite
                          direction to the direction we took) is leaning
                          the direction we took (from the current node).
                          Otherwise to double rotation.
                */
                con_direct = 1 - curr_direct;
                if ( son_node->balance == curr_direct ) {
                    swap( son_node, con_direct, curr_node, curr_direct );
                    curr_node->balance = BALANCED;
                } else {
                    grand_son = son_node->ptr[con_direct];
                    swap( grand_son, curr_direct,  son_node,  con_direct );
                    swap( grand_son,  con_direct, curr_node, curr_direct );
                    son_node->balance = (grand_son->balance == con_direct) ?
                                            curr_direct : BALANCED;
                    curr_node->balance = (grand_son->balance == curr_direct) ?
                                            con_direct : BALANCED;
                    son_node = grand_son;
                }
                son_node->balance = BALANCED;

                /* now make parent point to new rotated node */
                path[level-1]->ptr[direct[level-1]] = son_node;
                break;
            }
        }
    } while ( TRUE );

    /*  Return a pointer to the node inserted */
    return ( insert_node );
}

