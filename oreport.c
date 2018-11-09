#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "o.h"

/*
** Local Prototypes
*/

char *ext_type[] = {
    "ABS",                  /* Or Unknown */
    "NEAR",
    "FAR",
    "BYTE",
    "WORD",
    "DWORD",
    "FWORD",
    "QWORD",
    "TBYTE"
};

void print_ext( ext_rec )
    EXT_T   *ext_rec;
{
    char            operand[80];
    char            *var_type;
    char            *label;
    int             type;
    NODE_T          *pub_node;
    PUB_T           *pub_rec;

    /*
    ** Make sure name doesn't already exist as a public symbol
    */
    pub_node = start( public_tree, RIGHT );
    while ( pub_node != public_tree ) {
        pub_rec = (PUB_T *)pub_node->data;
        if ( strcmp(pub_rec->name, ext_rec->name) == 0 ) {
            return;
        }
        pub_node = traverse( pub_node, RIGHT );
    }

    if ( ext_rec->scope ) {
        label = "";
    } else {
        label = ";Static";
    }

    if ( ext_rec->com_ext == 0 ) {
        if ( ext_rec->type == UNKNOWN && ext_rec->pos_abs == FALSE ) {
            ext_rec->type = NEAR;
        }
        sprintf( operand, "%s:%s", ext_rec->name, ext_type[ext_rec->type] );
        out_label( label );
        out_opcode( "EXTRN" );
        tab_seek(3);
        out_operand( operand );
    } else {
        switch( ext_rec->var_type ) {
            case 0x61:
                var_type = "COMM FAR";
                break;
            case 0x62:
                var_type = "COMM";
                break;
            default:
                fmt_error( "Unknown Communal Variable Type" );
                break;
        }
        type = ext_rec->type;

        sprintf( operand, "%s:%s:%ld", ext_rec->name, ext_type[type], 
                                                            ext_rec->size );
        out_label( label );
        if ( ext_rec->var_type == 0x62 ) {
            tab_seek(3);
        }
        out_opcode( var_type );
        out_operand( operand );
    }
}

void list_ext()
{
    NODE_T          *seg_node;
    NODE_T          *ext_node;
    EXT_T           *ext_rec;
    int             data_index;

    data_index = 0;
    seg_node = start( segment_tree, RIGHT );
    while ( seg_node != segment_tree ) {
        seg_rec = (SEG_T *)seg_node->data;
        if ( stricmp(seg_rec->class->name, "DATA") == 0 ) {
            data_index = seg_rec->index;
            break;
        }
        seg_node = traverse( seg_node, RIGHT );
    }

    out_newline();
    ext_node = start( extern_tree, LEFT );
    while ( ext_node != extern_tree ) {
        ext_rec = (EXT_T *)ext_node->data;
        if ( ext_rec->used == 0 || ext_rec->used == -1 ) {
            if ( compatibility == 2 && data_index != 0 ) {
                if ( ext_rec->type == 2 || ext_rec->type == 3 ||
                     ext_rec->type == 4 || ext_rec->type == 5 ) {
                    if ( ext_rec->used == 0 ) {
                        sex_insert( data_index, ext_rec );
                        ext_rec->used = data_index;
                    }
                } else {
                    print_ext( ext_rec );
                }
            } else {
                print_ext( ext_rec );
            }
        }
        ext_node = traverse( ext_node, LEFT );
    }
}

void list_pub()
{
    NODE_T          *pub_node;
    PUB_T           *pub_rec;
    char            comment[50];
    char            *label;

    out_newline();
    pub_node = start( public_tree, RIGHT );
    while ( pub_node != public_tree ) {
        pub_rec = (PUB_T *)pub_node->data;
        if ( pub_rec->scope ) {
            label = "";
        } else {
            label = ";Static";
        }
        if ( pub_rec->domain == PUBLIC ) {
            sprintf( comment, "Located at %d:%04lXh Type = %d", pub_rec->seg_idx, pub_rec->offset, pub_rec->type );
            out_label( label );
            out_opcode( "PUBLIC" );
            tab_seek(3);
            out_operand( pub_rec->name );
            out_comment( comment );
        } else {
#ifdef DEBUG
            sprintf( comment, "Located at %d:%04lXh Type = %d", pub_rec->seg_idx, pub_rec->offset, pub_rec->type );
            out_label( label );
            out_opcode( "PRIV" );
            tab_seek(3);
            out_operand( pub_rec->name );
            out_comment( comment );
#endif
        }
        pub_node = traverse( pub_node, RIGHT );
    }
}

void list_struc()
{
    NODE_T          *struc_node;
    STRUC_T         *struc_rec;
    char            temp[50];
    int             index;
    char            *cp;
    char            ch;
    dword   dup_cnt;
    int             type;
    int             mem_cnt;
    int             times;

    index = 0;
    out_newline();
    struc_node = start( struc_tree, RIGHT );
    while ( struc_node != struc_tree ) {
        struc_rec = (STRUC_T *)struc_node->data;

        struc_rec->index = index;
        cp = struc_rec->form;

        sprintf( temp, "struct_%d", index );
        out_line( temp, "struc", "", "" );

        mem_cnt = 0;
        while ( (ch=*cp) != '\0' ) {
            switch( ch ) {
                case '(':
                    dup_cnt = atol( cp+1 );
                    break;
                case ',':
                    type = atoi( cp+1 );
                    break;
                case ')':
                    sprintf( temp, "s%dm_%d", index, mem_cnt );
                    out_label( temp );
                    out_directive( size_to_opcode(type_to_size(type), &times) );
                    if ( dup_cnt == 1L ) {
                        strcpy( temp, "?" );
                    } else {
                        sprintf( temp, "%ld dup (?)", dup_cnt );
                    }
                    out_operand( temp );
                    out_endline();
                    mem_cnt++;
                default:
                    break;
            }
            cp++;
        }

        sprintf( temp, "struct_%d", index );
        out_line( temp, "ends", "", "" );

        out_newline();

        index++;
        struc_node = traverse( struc_node, RIGHT );
    }
}

void list_fix()
{
    NODE_T          *fix_node;
    FIX_T           *fix_rec;
    char            comment[50];

    out_newline();
    fix_node = start( fix_tree, RIGHT );
    while ( fix_node != fix_tree ) {
        fix_rec = (FIX_T *)fix_node->data;
        sprintf( comment, "Fixup at 0%02Xh:0%04Xh:0%04Xh", fix_rec->seg_idx,
                fix_rec->dat_offset, fix_rec->offset );
        out_line( "", ";", comment, "" );
        fix_node = traverse( fix_node, RIGHT );
    }
}
