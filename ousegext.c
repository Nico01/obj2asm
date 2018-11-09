#include <stdio.h>
#include <string.h>
#include "o.h"

int sex_compare( SEX_T *, SEX_T * );

int sex_compare( rec_1, rec_2 )
    SEX_T   *rec_1;
    SEX_T   *rec_2;
{
    int     result;

    if ( rec_1->seg_index > rec_2->seg_index ) {
        return( LEFT );
    } else {
        if ( rec_1->seg_index < rec_2->seg_index ) {
            return( RIGHT );
        } else {
            result = strcmp( rec_1->ext_rec->name, rec_2->ext_rec->name );
            if ( result < 0 ) {
                return( LEFT );
            } else {
                if ( result > 0 ) {
                    return( RIGHT );
                } else {
                    return( EQUAL );
                }
            }
        }
    }
}

void sex_insert( seg_index, ext_rec)
    int     seg_index;
    EXT_T   *ext_rec;
{
    SEX_T   *sex_rec;

    sex_rec = (SEX_T *)o_malloc( sizeof(SEX_T) );
    sex_rec->seg_index   = seg_index;
    sex_rec->ext_rec     = ext_rec;

    insert( (char *)sex_rec, sex_tree, TC sex_compare );

}
