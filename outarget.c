#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "o.h"

/*
** Local Prototypes
*/
char *seg_name( int, word * );
char *grp_name( int );
void get_seg( char *, FIX_T *, int, void *, int );
void assume( int, int, int );

FIXER_T fix_type[] = {
    1, "db",     "low ",      /* LOBYTE  */
    2, "dw",  "offset ",      /* OFFSET  */
    2, "dw",     "seg ",      /* BASE    */
    4, "dd",     "ptr ",      /* POINTER */
    1, "db",    "high ",      /* HIBYTE  */
    2, "dw",  "offset "  };   /* Loader-resolved */
                              /*     OFFSET      */

char *seg_name( index, size )
    int             index;
    word    *size;
{
    SEG_T   *segment_rec;
    NAME_T  *name_rec;

    seg_search.index = index;
    segment_rec = (SEG_T *)find( (char *)&seg_search, segment_tree, 
                                                        TC seg_compare, NULL );
    if ( segment_rec == NULL ) fmt_error( "Undefinded segment" );
    *size = segment_rec->length;
    name_search.index = segment_rec->name;
    name_rec = (NAME_T *)find( (char *)&name_search, name_tree, 
                                                        TC name_compare, NULL );
    if ( name_rec == NULL ) fmt_error( "Undefined segment name" );
    return( name_rec->name );
}

char *grp_name( index )
    int     index;
{
    GRP_T   *grp_rec;
    NAME_T  *name_rec;

    grp_search.index = index;
    grp_rec = (GRP_T *)find( (char *)&grp_search, group_tree, 
                                                        TC grp_compare, NULL );
    if ( grp_rec == NULL ) fmt_error( "Undefined group" );
    name_search.index = grp_rec->name;
    name_rec = (NAME_T *)find( (char *)&name_search, name_tree, 
                                                        TC name_compare, NULL );
    if ( name_rec == NULL ) fmt_error( "Undefined group name" );
    return( name_rec->name );
}

char *mode_name( mode, index )
    int             mode;
    int             index;
{
    word    dummy;

    if ( index == 0 ) {
        return( "NOTHING" );
    }

    switch( mode ) {
        case SEGMENT:   return( seg_name(index,&dummy) );
        case GROUP:     return( grp_name(index) );
        case LOCATION:  return( cseg_name );

        case EXTERNAL:
        case FRAME:
        case TARGET:
        case NONE:      return( NULL );
    }
    return(NULL);
}

void assume( seg_reg, data_mode, data_index )
    int             seg_reg;
    int             data_mode;
    int             data_index;
{
    char            *seg_name;
    extern  char    *sregsc[];
    char            operand[100];

    seg_name = mode_name( data_mode, data_index );
    if (seg_name && compatibility != 2 && pass == 3 ) {
        out_opcode( "assume" );
        sprintf( operand, "%s %s", sregsc[seg_reg], seg_name );
        out_operand( operand );
        out_endline();
    }
}

void adjust_assumes()
{
    int     seg_reg;
    int     new_mode;
    int     new_index;

    for ( seg_reg = 0; seg_reg < MAX_SEG_REGS; seg_reg++ ) {
        new_mode  = seg_rec->new_mode [seg_reg];
        new_index = seg_rec->new_index[seg_reg];
        if ( new_mode  != seg_rec->prv_mode [seg_reg] ||
             new_index != seg_rec->prv_index[seg_reg] ) {
            assume( seg_reg, new_mode, new_index );
            seg_rec->prv_mode [seg_reg] = new_mode;
            seg_rec->prv_index[seg_reg] = new_index;
        }
    }
}

void abort_assumes()
{
    int     seg_reg;

    for ( seg_reg = 0; seg_reg < MAX_SEG_REGS; seg_reg++ ) {
        seg_rec->new_mode [seg_reg] = seg_rec->prv_mode [seg_reg];
        seg_rec->new_index[seg_reg] = seg_rec->prv_index[seg_reg];
    }
}

void get_seg( result, fixup, ref_mode, data, seg_reg )
    char    *result;
    FIX_T   *fixup;
    int     ref_mode;
    void    *data;
    int     seg_reg;
{
    char    *this_name;
    int     form;
    int     mode;
    int     index;
    int     ref_seg;
    EXT_T   *ext_rec;
    PUB_T   *pub_rec;

    form  = fixup->form;
    mode  = fixup->a_mode;
    index = fixup->a_index;

    strcpy( result, "" );

    if ( mode == TARGET || mode == NONE ) {
        return;
    }

    /*
    ** Check for Externals with implied or lacking segment
    **  This means that the fixup tells that the external was defined in
    **  that segment (for implied, SI:EI). Or that the external was defined
    **  outside any segment (for lacking, EI:EI).
    */
    if ( fixup->b_mode == EXTERNAL ) {
        ext_rec = data;
        if ( mode == EXTERNAL ) {
            /* Must be EI:EI */
            ext_rec->used = -1;         /* Remove from any segments! */
            return;
        }
        if ( mode == SEGMENT ) {
            ext_rec->pos_abs = FALSE;
            if ( ref_mode != 0 ) {
                /* Must be Segment:EI */
                if ( pass == 1 && ext_rec->used == 0 ) {
                    sex_insert( index, ext_rec );
                    ext_rec->used = index;
                }
                if ( ext_rec->used == index ) {
                    return;
                }
            }
        }
    } else {
        if ( fixup->b_mode == SEGMENT ) {
            pub_rec = data;
            if ( ref_mode == 2 && (ref_mode == 1 && form == POINTER) ) {
                if ( pub_rec->seg_idx == index ) {
                    return;
                }
            }
        }
    }

    /*
    ** On first important fixup, assign the "ASSIGN xx:" values
    */
    if ( ref_mode == 0 || (ref_mode == 1 && form != POINTER) ) {
        ref_seg = seg_reg - ES;                 /* Convert to 0,1,2,3,etc */
        seg_rec->new_mode [ref_seg] = mode;
        seg_rec->new_index[ref_seg] = index;
        if ( mode == SEGMENT ) {
            if ( fixup->b_mode == EXTERNAL && ext_rec->used == index ) {
                return;
            }
        }
        if ( mode == SEGMENT || mode == GROUP ) {
            if ( fixup->b_mode == SEGMENT || fixup->b_mode == EXTERNAL ) {
                return;
            }
        }
        /*
        ** Frame on a code reference is always taken from the assume?
        */
    }

    if ( form == BASE ) {
        if ( fixup->b_mode == mode && fixup->b_index == index ) {
            return;
        }
    }

    this_name = mode_name( mode, index );
    if ( this_name ) {
        sprintf( result, "%s:", this_name );
        return;
    } else {
        return;
    }
}

PUB_T *check_public( mode, index, displacement, label_char )
    int             mode;
    int             index;
    long            displacement;
    char            label_char;
{
    PUB_T           *new_pub_rec;
    NODE_T          *new_node;
    DAT_T           dat_search;
    DAT_T           *dat_rec;
    NODE_T          *dat_node;
    FIX_T           *fix_rec;
    NODE_T          *fix_node;
    int             ref_seg;
    char            label_name[10];
    dword   disp;
    int             fix_count;

#ifdef DEBUG
printf("--> check_public for [%04X:%04lX]\n", index, displacement );
#endif

    if ( over_seg == -1 ) {
        ref_seg = 2;
    } else {
        ref_seg = over_seg;
    }

    if ( displacement < 0L ) {
        disp = 0L;
    } else {
        disp = displacement;
    }

    if ( mode == 2 ) {
        index = seg_rec->prv_index[ref_seg];
    }

    pub_search.seg_idx = index;
    pub_search.offset  = disp;

    new_pub_rec = (PUB_T *)find( (char *)&pub_search, public_tree,
                                                    TC pub_compare, &new_node );
    if ( new_pub_rec ) {
        return( new_pub_rec );
    }
    /*
    ** If the address is just slightly past another public symbol, and
    ** the address is less than the public symbols address + its size,
    ** then it is probably a "public+x" type fixup.
    */
    if ( new_node != public_tree ) {
        new_pub_rec = (PUB_T *)new_node->data;
        if ( new_pub_rec->seg_idx == index ) {
            if ( new_pub_rec->offset + type_to_size(new_pub_rec->type) > disp ) {
                return( new_pub_rec );
            }
        }
    }
    /*
    ** Find the data record that that symbol probably belongs in
    */
    dat_search.seg_idx = index;
    dat_search.offset = disp;
    dat_rec = (DAT_T *)find( (char *)&dat_search, data_tree, 
                                                    TC dat_compare, &dat_node );
    if ( dat_node && dat_node != data_tree ) {
        dat_rec = (DAT_T *)dat_node->data;
        if ( dat_rec->seg_idx == index 
             && dat_rec->offset+dat_rec->length > disp ) {
            /*
            ** Make sure the symbol is not in the middle of a fixup
            */
            fix_search.seg_idx    = index;
            fix_search.dat_offset = dat_rec->offset;
            fix_search.offset     = (int)(disp - (dword)dat_rec->offset);
            fix_rec = (FIX_T *)find( (char *)&fix_search, fix_tree,
                                                      TC fix_compare, &fix_node );
            if ( !fix_rec ) {
                if ( fix_node != fix_tree ) {
                    fix_rec = (FIX_T *)fix_node->data;
                    if ( fix_rec->seg_idx == index
                         && fix_rec->dat_offset == dat_rec->offset ) {

                        fix_count = fix_type[fix_rec->form].num_bytes;

                        /*
                        ** BASE types do not get larger
                        */
                        if ( fix_rec->extended && fix_rec->form != BASE ) {
                            fix_count += 2;
                        }
                        if ( fix_search.offset > fix_rec->offset
                          && fix_search.offset < fix_rec->offset+fix_count ) {
                            return( new_pub_rec );
                        }
                    }
                }
            }
            /*
            ** Make sure that public is not being placed inside an
            ** iterated data block.  If it is, then retreat to the
            ** start of the iterated data block.  In this way, labels
            ** will never be generated inside iterated data blocks.
            */
            if ( dat_rec->type == ITERATED ) {
                if ( new_node == public_tree ) {
                    return( NULL );
                }
                if ( new_pub_rec->seg_idx != index ) {
                    return( NULL );
                }
                if ( new_pub_rec->offset >= dat_rec->offset ) {
                    return( new_pub_rec );
                }
                disp = dat_rec->offset;
            }
        }
    }

    /*
    ** Create a local label (only happens on 1st pass!)
    */
    label_count++;
    if ( compatibility == 2 ) {
        sprintf( label_name, "@%c%d", label_char, label_count );
    } else {
        sprintf( label_name, "$%c%d", label_char, label_count );
    }
#ifdef DEBUG
printf("Creating a new symbol <%s> at %d:%04X\n", label_name, index, disp);
#endif

    new_node = pub_insert( index, disp, label_name, LOCAL, FALSE );

    new_pub_rec = (PUB_T *)find( (char *)&pub_search, public_tree,
                                                        TC pub_compare, NULL );
    if ( (pub_node == public_tree || index < pub_rec->seg_idx || 
          (index == pub_rec->seg_idx && disp < pub_rec->offset)) &&
           (index > segment || (index == segment && disp > inst_offset)) ) {
        pub_node = new_node;
        pub_rec  = new_pub_rec;
    }
    if ( index == segment && disp == inst_offset ) {
        last_pub_rec = new_pub_rec;
    }

    return( new_pub_rec );
}

int find_member( text, structure, offset )
    char    *text;
    STRUC_T *structure;
    long    *offset;
{
    int     type;
    int     mem_cnt;
    long    dup_cnt;
    long     this_size;
    char    *cp;
    char    ch;

    mem_cnt = 0;
    cp = structure->form;

    while ( (ch=*cp) != '\0' ) {
        switch( ch ) {
            case '(':
                dup_cnt = atol( cp+1 );
                break;
            case ',':
                type = atoi( cp+1 );
                break;
            case ')':
                this_size = type_to_size(type)*dup_cnt;
                if ( *offset < this_size ) {
                    sprintf( text, ".s%dm_%d", structure->index, mem_cnt );
                    return( type );
                }
                *offset -= this_size;
                mem_cnt++;
            default:
                break;
        }
        cp++;
    }

    strcpy( text, ".past struct" );

    return( UNKNOWN );
}

void get_target( result, fixup, mode, data, pre_name, type, assign,
                                                        type_known, seg_reg )
    char        *result;        /* Output string                            */
    FIX_T       *fixup;         /* Fixup to figure out                      */
    int         mode;           /* 0 = Data Deref, 1 = Code, 2 = Data Ref   */
    long        data;           /* Value added (fixups get added onto this) */
    int         pre_name;       /* Are the fixup type words needed?         */
    int         type;           /* type of value pointed to (word ptr, etc) */
    int         assign;         /* Assign type if needed?                   */
    int         *type_known;    /* Is the "word ptr" text needed? (result)  */
    int         seg_reg;        /* Segment register used                    */
{
    int         index;
    PUB_T       *pub_rec;
    EXT_T       ext_search;
    EXT_T       *ext_rec;
    STRUC_T     *pub_struct;
    char        *prefix;
    char        this_seg[NAMESIZE+1+1];     /* 1 for ':', 1 for \0 */
    char        this_member[NAMESIZE+1+1];  /* 1 for ., 1 for \0 */
    word    seg_size;
    char        *this_name;
    char        *this_comment;
    int         this_type;
    char        sign;
    long        disp;

#ifdef DEBUG
printf("-> get_target for [%04X:%04X]\n", fix_rec->seg_idx, fix_rec->dat_offset );
#endif

    disp = data;

    this_seg[0] = '\0';
    this_member[0] = '\0';
    this_comment = "";

    /*
    ** Fixups cannot have a displacement value (stored with the fixup)
    ** AND a value added (stored in the image to which the fixup is added)
    */
    if ( fixup->displacement != 0L ) {
        if ( disp != 0L ) {
            this_comment = "[MULTIPLE FIXUP]";
        }
        disp += fixup->displacement;
    }

    if ( pre_name ) {
        prefix = fix_type[fixup->form].prefix;
    } else {
        prefix = "";
    }

    index = fixup->b_index;

    switch( fixup->b_mode ) {
        case SEGMENT:           /* Segment Index and Displacement */
            if ( fixup->form != BASE ) {
                /* PUBLIC names will be stored as segment + offset */
                pub_rec = check_public( 0, index, disp, 'S' );
                if ( pub_rec ) {
                    this_name = pub_rec->name;
                    disp -= (long)pub_rec->offset;
                    pub_struct = pub_rec->structure;
                    if ( pub_struct ) {
                        this_type = find_member( this_member, pub_struct,
                                                                      &disp );
                    } else {
                        if ( assign && ((type <= FAR && pub_rec->type > FAR)
                                || pub_rec->type == UNKNOWN) ) {
                            pub_rec->type = type;
                        }
                        this_type = pub_rec->type;
                    }
                    if ( this_type != type ) {
                        *type_known = FALSE;
                    } else {
                        *type_known = TRUE;
                    }
                    get_seg( this_seg, fixup, mode, pub_rec, seg_reg );
                    break;
                }
            }

            this_name = seg_name(index,&seg_size);
            if ( fixup->form == BASE && fixup->displacement != 0 ) {
                prefix = "";        /* MASM generates seg length, if */
                disp -= seg_size;   /* "SEG" is missing, do the same */
            }
            this_seg[0] = '\0';
            *type_known = TRUE;
            break;
        case GROUP:                     /* Group Index and Displacement */
            this_name = grp_name(index);
            *type_known = TRUE;
            get_seg( this_seg, fixup, mode, NULL, seg_reg );
            break;
        case EXTERNAL:                  /* External Index and Displacement */
            ext_search.index = index;
            ext_rec = (EXT_T *)find( (char *)&ext_search,
                                        extern_tree, TC ext_compare, NULL );
            if ( ext_rec == NULL ) fmt_error( "Undefined external" );

            this_name = ext_rec->name;

            if ( assign && ((type <= FAR && ext_rec->type > FAR)
                            || ext_rec->type == UNKNOWN) ) {
                ext_rec->type = type;
            }
            if ( ext_rec->type != type ) {
                *type_known = FALSE;
            } else {
                *type_known = TRUE;
            }
            get_seg( this_seg, fixup, mode, ext_rec, seg_reg );
            break;
        case FRAME:                     /* Frame Number and Displacement */
            get_seg( this_seg, fixup, mode, NULL, seg_reg );
            break;
    }

    if ( disp == 0 ) {
/* ;* */ sprintf( result, "%s%s%s%s%s", prefix, this_seg,
                     this_name, this_member, this_comment );
    } else {
        if ( disp > 0L ) {
            sign = '+';
        } else {
            sign = '-';
            disp = -disp;
        }
        if ( disp > 32767L || disp < -32768L ) {
            sprintf( result, "%s%s%s%s %c 0%08lXh%s",
                                prefix, this_seg,
                                this_name, this_member,
                                sign, disp,
                                this_comment );
        } else {
            sprintf( result, "%s%s%s%s %c 0%04lXh%s",
                                prefix, this_seg,
                                this_name, this_member,
                                sign, disp,
                                this_comment );
        }
    }
#ifdef DEBUG
printf("<- get_target [%04X:%04X]\n", fix_rec->seg_idx, fix_rec->dat_offset );
#endif

}

int get_fix( result, mode, pre_name, size_bytes, size, assign,
                                                    size_known, seg_reg )
    char            *result;        /* Resulting name will go here            */
    int             mode;           /* 0 = Data Deref, 1 = Code, 2 = Data Ref */
    int             pre_name;       /* Fixup type words needed?               */
    int             size_bytes;     /* Word size (386=4 others=2)             */
    int             size;           /* Size of expected value                 */
    int             assign;         /* TRUE to set size, FALSE to ignore      */
    int             *size_known;    /* Is "word ptr" text needed?             */
    int             seg_reg;        /* Segment register used                  */
{
    dword   data;
    dword   more;
    int             fix_count;

    size_bytes = size_bytes;
    fix_count = fix_type[fix_rec->form].num_bytes;

    /*
    ** BASE types do not get larger
    */
    if ( fix_rec->extended && fix_rec->form != BASE ) {
        fix_count += 2;
    }

    switch( fix_count ) {
        case 1: data = buff_getc();
                break;
        case 2: data = buff_getc();
                data += (buff_getc() << 8);
                break;
        case 4: data = buff_getc();
                data += (buff_getc() << 8);
                data += (buff_getc() << 16);
                data += (buff_getc() << 24);
                break;
        case 6: data = buff_getc();
                data += (buff_getc() << 8);
                data += (buff_getc() << 16);
                data += (buff_getc() << 24);
                more = buff_getc();
                more += (buff_getc() << 8);
                break;
        default:
                fmt_error( "Invalid fixup type" );
                break;
    }

    get_target( result, fix_rec, mode, data, pre_name, size, assign,
                                                        size_known, seg_reg );
    fix_advance();

    return( fix_count );
}
