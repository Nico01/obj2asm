#include <string.h>
#include "o.h"

NODE_T *new_tree(void *data_ptr, int dup_allowed)
{
    NODE_T *tree = o_malloc(sizeof(NODE_T));

    tree->data = data_ptr;

    if (dup_allowed) {        /* Hide the duplicates allowed indicator */
        tree->balance = LEFT; /* in the tree root's balance!           */
    }
    else {
        tree->balance = RIGHT;
    }

    tree->ptr[LEFT] = 0; /* Hide the tree depth in left pointer */
    tree->ptr[RIGHT] = (NODE_T *)tree;
    tree->thread[LEFT] = TRUE;
    tree->thread[RIGHT] = TRUE;

    return tree;
}

void init_trees(void)
{
    NAME_T *name_root;
    SEG_T *segment_root;
    GRP_T *group_root;
    PUB_T *public_root;
    EXT_T *extern_root;
    SEX_T *sex_root;
    DAT_T *data_root;
    STRUC_T *struc_root;
    FIX_T *fix_root;
    HINT_T *hint_root;
    LINE_T *line_root;
    SCOPE_T *arg_scope_root;
    SCOPE_T *loc_scope_root;
    SCOPE_T *end_scope_root;

    line_root = o_malloc(sizeof(LINE_T));
    line_root->hex_offset = -1;
    line_tree = new_tree((char *)line_root, FALSE);

    loc_scope_root = o_malloc(sizeof(SCOPE_T));
    loc_scope_root->hex_offset = -1;
    loc_scope_root->head = NULL;
    loc_scope_tree = new_tree(loc_scope_root, FALSE);

    arg_scope_root = o_malloc(sizeof(SCOPE_T));
    arg_scope_root->hex_offset = -1;
    arg_scope_root->head = NULL;
    arg_scope_tree = new_tree(arg_scope_root, FALSE);

    end_scope_root = o_malloc(sizeof(SCOPE_T));
    end_scope_root->hex_offset = -1;
    end_scope_root->head = NULL;                     /* field not used in TCENDSCP */
    end_scope_tree = new_tree(end_scope_root, TRUE); /* allow duplicates */

    /* Make an empty name tree node */
    name_root = (NAME_T *)o_malloc(sizeof(NAME_T));
    name_root->index = 0;
    strcpy(name_root->name, "Root NAME Node");

    name_tree = new_tree((char *)name_root, FALSE);

    /* Make an empty segment tree node */
    segment_root = (SEG_T *)o_malloc(sizeof(SEG_T));
    segment_root->index = -1; /* Zero reserved to some equates */
    segment_root->name = 0;
    segment_root->class = NULL;
    segment_root->length = 0;
    segment_root->code = FALSE;
    segment_root->bit32 = FALSE;

    segment_tree = new_tree((char *)segment_root, FALSE);

    /* Make an empty group tree node */
    group_root = (GRP_T *)o_malloc(sizeof(GRP_T));
    group_root->index = 0;
    group_root->name = 0;

    group_tree = new_tree((char *)group_root, FALSE);

    /* Make an empty public tree node */
    public_root = (PUB_T *)o_malloc(sizeof(PUB_T));
    public_root->seg_idx = -1; /* Zero reserved to some equates */
    public_root->offset = 0;
    public_root->domain = 0;
    public_root->scope = FALSE;
    strcpy(public_root->name, "Root PUBLIC Node");

    public_tree = new_tree((char *)public_root, TRUE);

    /* Make an empty external tree node */
    extern_root = (EXT_T *)o_malloc(sizeof(EXT_T));
    extern_root->index = 0;
    extern_root->type = 0;
    extern_root->com_ext = 0;
    extern_root->var_type = 0;
    extern_root->size = 0L;
    extern_root->used = 0;
    extern_root->scope = 0;
    strcpy(extern_root->name, "Root EXTERN Node");

    extern_tree = new_tree((char *)extern_root, FALSE);

    /* Make an empty external tree node */
    sex_root = (SEX_T *)o_malloc(sizeof(SEX_T));
    sex_root->seg_index = 0;

    sex_tree = new_tree((char *)sex_root, FALSE);

    /* Make an empty data record tree node */
    data_root = (DAT_T *)o_malloc(sizeof(DAT_T));
    data_root->seg_idx = -1; /* Zero reserved to some equates */
    data_root->offset = 0;
    data_root->file_pos = 0L;
    data_root->length = 0;

    data_tree = new_tree((char *)data_root, FALSE);

    /* Make an empty structure tree node */
    struc_root = (STRUC_T *)o_malloc(sizeof(STRUC_T));
    struc_root->form = "";

    struc_tree = new_tree((char *)struc_root, FALSE);

    /* Make an empty fixup record tree node */
    fix_root = (FIX_T *)o_malloc(sizeof(FIX_T));
    fix_root->seg_idx = -1; /* Zero reserved to some equates */
    fix_root->offset = 0;
    fix_root->relate = 0;
    fix_root->form = 0;
    fix_root->a_mode = 0;
    fix_root->a_index = 0;
    fix_root->b_mode = 0;
    fix_root->b_index = 0;
    fix_root->displacement = 0L;
    fix_root->extended = FALSE;
    fix_root->word_sized = FALSE;

    fix_tree = new_tree((char *)fix_root, FALSE);

    hint_root = (HINT_T *)o_malloc(sizeof(HINT_T));
    hint_root->seg_idx = 0;
    hint_root->offset = 0L;
    hint_root->hint_type = 0;
    hint_root->length = 0L;
    hint_tree = new_tree((char *)hint_root, FALSE);
}

static void swap(NODE_T *node_A, int direct_A, NODE_T *node_B, int direct_B)
{
    /* Swap the pointers (unless they are already threads) */
    if (node_A->thread[direct_A]) {
        node_B->ptr[direct_B] = node_A;
        node_B->thread[direct_B] = TRUE;
        node_A->thread[direct_A] = FALSE;
    }
    else {
        node_B->ptr[direct_B] = node_A->ptr[direct_A];
        node_A->ptr[direct_A] = node_B;
    }
}

/*
    void   *data;                       pointer to data struct to save
    NODE_T *root_node;                  pointer to root node
    int    (*cmp_routine)(void*,void*); routine used to compare values at 2 nodes
*/

NODE_T *insert(void *data, NODE_T *root_node, int (*cmp_routine)(void *, void *))
{
    NODE_T *insert_node;
    NODE_T *curr_node; /* Current node we are visiting      */
    NODE_T *son_node;
    NODE_T *grand_son;
    NODE_T *path[32]; /* 32 levels (2^32 records!) */
    int direct[32];
    int level;
    int curr_direct;
    int con_direct;

    /* Prepare initial value for tree traversal */
    level = 0;
    son_node = root_node;

    /*
    **  Traverse tree looking for the place to insert this new node.  If
    **  we find a node which gives a comparison result of 0 (equal), then
    **  we cannot insert this new node.  To indicate this duplicate value,
    **  a pointer to the node which contains the duplicate value is returned.
    */
    do {
        curr_node = son_node; /* Advance to new son_node */

        /*  Compare this new node with the node at this position in the
            tree and determine which direction to proceed from here.
        */
        curr_direct = (*cmp_routine)(curr_node->data, data);

        /*  is this node is a duplicate node?
         */
        if (curr_direct == EQUAL) {
            if (root_node->balance == LEFT) { /* Duplicates allowed? */
                curr_direct = LEFT;           /* see new_tree()!!!!  */
            }
            else {
                return (curr_node);
            }
        }

        path[level] = curr_node;
        direct[level] = curr_direct;
        level++;

        son_node = curr_node->ptr[curr_direct];

    } while (!curr_node->thread[curr_direct]);

    /*  Now, we have found the place to insert this node,
        make the last visited node point to this new node.
    */
    con_direct = 1 - curr_direct;

    /* Set up default values for this node which we are going to insert */
    insert_node = (NODE_T *)o_malloc(sizeof(NODE_T));

    insert_node->data = data;
    insert_node->balance = BALANCED;
    insert_node->thread[curr_direct] = TRUE;
    insert_node->thread[con_direct] = TRUE;
    insert_node->ptr[curr_direct] = curr_node->ptr[curr_direct];
    insert_node->ptr[con_direct] = curr_node;

    curr_node->thread[curr_direct] = FALSE;
    curr_node->ptr[curr_direct] = insert_node;

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
        if (level == 0) { /* Increment depth (see new_tree()!!!) */
            root_node->ptr[LEFT] = (NODE_T *)(((int)root_node->ptr[LEFT]) + 1);
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
        if (curr_node->balance == BALANCED) {
            curr_node->balance = curr_direct;
        }
        else {
            /*  Otherwise, two cases may arise.
                1.  The node was unbalanced in the opposite
                    direction to the direction we took.
                    (In this case that node will become balanced)
            */
            if (curr_node->balance != curr_direct) {
                curr_node->balance = BALANCED;
                break;
            }
            else {
                /* or 2.  The node was already unbalanced in the same
                          direction we took. If this happens we may need to
                          do single or double rotation.  Single rotation
                          happens if the node (which is in the opposite
                          direction to the direction we took) is leaning
                          the direction we took (from the current node).
                          Otherwise to double rotation.
                */
                con_direct = 1 - curr_direct;
                if (son_node->balance == curr_direct) {
                    swap(son_node, con_direct, curr_node, curr_direct);
                    curr_node->balance = BALANCED;
                }
                else {
                    grand_son = son_node->ptr[con_direct];
                    swap(grand_son, curr_direct, son_node, con_direct);
                    swap(grand_son, con_direct, curr_node, curr_direct);
                    son_node->balance = (grand_son->balance == con_direct) ? curr_direct : BALANCED;
                    curr_node->balance = (grand_son->balance == curr_direct) ? con_direct : BALANCED;
                    son_node = grand_son;
                }
                son_node->balance = BALANCED;

                /* now make parent point to new rotated node */
                path[level - 1]->ptr[direct[level - 1]] = son_node;
                break;
            }
        }
    } while (TRUE);

    /*  Return a pointer to the node inserted */
    return (insert_node);
}

/*
    char   *data;                       pointer to data struct to find
    NODE_T *root_node;                  pointer to root node
    int    (*cmp_routine)(void*,void*); routine used to compare values at 2 nodes
    NODE_T **node_ptr;                  Ptr to node found
*/

void *find(void *data, NODE_T *root_node, int (*cmp_routine)(void *, void *), NODE_T **node_ptr)
{
    NODE_T *curr_node;
    int curr_direct;

    /* Prepare initial value for tree traversal */
    NODE_T *child_node = root_node;

    /*  Traverse tree looking for the place to insert this new node.  If
    **  we find a node which gives a comparison result of 0 (equal), then
    **  we cannot insert this new node.  To indicate this duplicate value,
    **  a non-zero value is returned (the value happens to be a pointer to
    **  node which contains the duplicate value). */
    do {
        curr_node = child_node;

        /*  Compare this new node with the node at this position in the
        **  tree and determine which direction to proceed from here. */
        curr_direct = (*cmp_routine)(curr_node->data, data);

        //  is this node is a duplicate node?
        if (node_ptr && curr_direct == RIGHT) {
            *node_ptr = curr_node;
        }

        if (curr_direct == EQUAL) {
            if (node_ptr) {
                *node_ptr = curr_node;
            }
            return curr_node->data;
        }

        child_node = curr_node->ptr[curr_direct];

    } while (!curr_node->thread[curr_direct]);

    return NULL;
}

NODE_T *start(NODE_T *root_node, int direct)
{
    NODE_T *curr_node = root_node->ptr[RIGHT];
    int con_direct = 1 - direct;

    while (!curr_node->thread[1 - direct]) {
        curr_node = curr_node->ptr[con_direct];
    }

    return curr_node;
}

NODE_T *traverse(NODE_T *curr_node, int direct)
{
    if (curr_node->thread[direct]) {
        return curr_node->ptr[direct];
    }
    else {
        curr_node = curr_node->ptr[direct];
        int con_direct = 1 - direct;

        while (!curr_node->thread[1 - direct]) {
            curr_node = curr_node->ptr[con_direct];
        }

        return curr_node;
    }
}
