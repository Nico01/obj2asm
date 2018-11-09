#include <stdio.h>
#include <string.h>
#include "o.h"

void init_trees()
{
    NAME_T  *name_root;
    SEG_T   *segment_root;
    GRP_T   *group_root;
    PUB_T   *public_root;
    EXT_T   *extern_root;
    SEX_T   *sex_root;
    DAT_T   *data_root;
    STRUC_T *struc_root;
    FIX_T   *fix_root;
    HINT_T  *hint_root;
    LINE_T  *line_root;
    SCOPE_T *arg_scope_root;
    SCOPE_T *loc_scope_root;
    SCOPE_T *end_scope_root;

    line_root = o_malloc(sizeof(LINE_T));
    line_root->hex_offset = -1;
    line_tree = new_tree((char*)line_root,FALSE);

    loc_scope_root = o_malloc(sizeof(SCOPE_T));
    loc_scope_root->hex_offset = -1;
    loc_scope_root->head = NULL;
    loc_scope_tree = new_tree(loc_scope_root,FALSE);

    arg_scope_root = o_malloc(sizeof(SCOPE_T));
    arg_scope_root->hex_offset = -1;
    arg_scope_root->head = NULL;
    arg_scope_tree = new_tree(arg_scope_root,FALSE);

    end_scope_root = o_malloc(sizeof(SCOPE_T));
    end_scope_root->hex_offset = -1;
    end_scope_root->head = NULL; /* field not used in TCENDSCP */
    end_scope_tree = new_tree(end_scope_root,TRUE);    /* allow duplicates */

    /* Make an empty name tree node */
    name_root = (NAME_T *)o_malloc( sizeof(NAME_T) );
    name_root->index = 0;
    strcpy( name_root->name, "Root NAME Node" );

    name_tree = new_tree( (char *)name_root, FALSE );

    /* Make an empty segment tree node */
    segment_root = (SEG_T *)o_malloc( sizeof(SEG_T) );
    segment_root->index      = -1;      /* Zero reserved to some equates */
    segment_root->name       = 0;
    segment_root->class      = NULL;
    segment_root->length     = 0;
    segment_root->code       = FALSE;
    segment_root->bit32      = FALSE;

    segment_tree = new_tree( (char *)segment_root, FALSE );

    /* Make an empty group tree node */
    group_root = (GRP_T *)o_malloc( sizeof(GRP_T) );
    group_root->index = 0;
    group_root->name = 0;

    group_tree = new_tree( (char *)group_root, FALSE );

    /* Make an empty public tree node */
    public_root = (PUB_T *)o_malloc( sizeof(PUB_T) );
    public_root->seg_idx = -1;      /* Zero reserved to some equates */
    public_root->offset  = 0;
    public_root->domain  = 0;
    public_root->scope   = FALSE;
    strcpy( public_root->name, "Root PUBLIC Node" );

    public_tree = new_tree( (char *)public_root, TRUE );

    /* Make an empty external tree node */
    extern_root = (EXT_T *)o_malloc( sizeof(EXT_T) );
    extern_root->index    = 0;
    extern_root->type     = 0;
    extern_root->com_ext  = 0;
    extern_root->var_type = 0;
    extern_root->size     = 0L;
    extern_root->used     = 0;
    extern_root->scope    = 0;
    strcpy( extern_root->name, "Root EXTERN Node" );

    extern_tree = new_tree( (char *)extern_root, FALSE );

    /* Make an empty external tree node */
    sex_root = (SEX_T *)o_malloc( sizeof(SEX_T) );
    sex_root->seg_index = 0;

    sex_tree = new_tree( (char *)sex_root, FALSE );

    /* Make an empty data record tree node */
    data_root = (DAT_T *)o_malloc( sizeof(DAT_T) );
    data_root->seg_idx  = -1;           /* Zero reserved to some equates */
    data_root->offset   = 0;
    data_root->file_pos = 0L;
    data_root->length   = 0;

    data_tree = new_tree( (char *)data_root, FALSE );

    /* Make an empty structure tree node */
    struc_root = (STRUC_T *)o_malloc( sizeof(STRUC_T) );
    struc_root->form = "";

    struc_tree = new_tree( (char *)struc_root, FALSE );

    /* Make an empty fixup record tree node */
    fix_root = (FIX_T *)o_malloc( sizeof(FIX_T) );
    fix_root->seg_idx      = -1;        /* Zero reserved to some equates */
    fix_root->offset       = 0;
    fix_root->relate       = 0;
    fix_root->form         = 0;
    fix_root->a_mode       = 0;
    fix_root->a_index      = 0;
    fix_root->b_mode       = 0;
    fix_root->b_index      = 0;
    fix_root->displacement = 0L;
    fix_root->extended     = FALSE;
    fix_root->word_sized   = FALSE;

    fix_tree = new_tree( (char *)fix_root, FALSE );

    hint_root = (HINT_T *)o_malloc( sizeof(HINT_T) );
    hint_root->seg_idx   = 0;
    hint_root->offset    = 0L;
    hint_root->hint_type = 0;
    hint_root->length    = 0L;
    hint_tree = new_tree( (char *)hint_root, FALSE );
}
