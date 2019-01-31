#include <string.h>
#include "o.h"


int sex_compare(SEX_T *rec_1, SEX_T *rec_2)
{
    if (rec_1->seg_index > rec_2->seg_index) {
        return LEFT;
    }
    else {
        if (rec_1->seg_index < rec_2->seg_index) {
            return RIGHT;
        }
        else {
            int result = strcmp(rec_1->ext_rec->name, rec_2->ext_rec->name);
            if (result < 0) {
                return LEFT;
            }
            else {
                if (result > 0) {
                    return RIGHT;
                }
                else {
                    return EQUAL;
                }
            }
        }
    }
}

void sex_insert(int seg_index, EXT_T *ext_rec)
{
    SEX_T *sex_rec = o_malloc(sizeof(SEX_T));
    sex_rec->seg_index = seg_index;
    sex_rec->ext_rec = ext_rec;

    insert((char *)sex_rec, sex_tree, TC sex_compare);
}
