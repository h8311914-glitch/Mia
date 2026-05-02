#include "m/MMat_imgproc.h"
#include "m/int.h"


#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"


//
// Imgproc
//


void MMat_distance_transform_into(oobj obj, oobj into, bool full)
{
    MMat *res = into;
    assert(res->format == M_FORMAT_1_UR16);
    MMat_cast_into(obj, res);

    if (!full) {
        // first pass, left top -> right bottom
        for (int r = 0; r < res->size.y; r++) {
            for (int c = 0; c < res->size.x; c++) {
                ou16 down = r == 0 ? 0 : *(ou16 *) MMat_at(res, c, r - 1);
                ou16 left = c == 0 ? 0 : *(ou16 *) MMat_at(res, c - 1, r);
                ou16 *set = MMat_at(res, c, r);
                ou16 value = o_min(down, left) + 1;
                *set = o_min(*set, value);
            }
        }

        // second pass, right bottom -> left top
        for (int r = res->size.y - 1; r >= 0; r--) {
            for (int c = res->size.x - 1; c >= 0; c--) {
                ou16 up = r == res->size.y - 1 ? 0 : *(ou16 *) MMat_at(res, c, r + 1);
                ou16 right = c == res->size.x - 1 ? 0 : *(ou16 *) MMat_at(res, c + 1, r);
                ou16 *set = MMat_at(res, c, r);
                ou16 value = o_min(up, right) + 1;
                *set = o_min(*set, value);
            }
        }
    } else {
        // full:

        // first pass, left top -> right bottom
        for (int r = 0; r < res->size.y; r++) {
            for (int c = 0; c < res->size.x; c++) {
                ou16 down = r == 0 ? 0 : *(ou16 *) MMat_at(res, c, r - 1);
                ou16 left = c == 0 ? 0 : *(ou16 *) MMat_at(res, c - 1, r);
                ou16 down_left = r == 0 ? 0 : (c == 0 ? 0 : *(ou16 *) MMat_at(res, c - 1, r - 1));
                ou16 down_right = r == 0 ? 0 : (c == res->size.x - 1 ? 0 : *(ou16 *) MMat_at(res, c + 1, r - 1));
                ou16 *set = MMat_at(res, c, r);
                ou16 value = o_min(o_min(o_min(down, left), down_left), down_right) + 1;
                *set = o_min(*set, value);
            }
        }

        // second pass, right bottom -> left top
        for (int r = res->size.y - 1; r >= 0; r--) {
            for (int c = res->size.x - 1; c >= 0; c--) {
                ou16 up = r == res->size.y - 1 ? 0 : *(ou16 *) MMat_at(res, c, r + 1);
                ou16 right = c == res->size.x - 1 ? 0 : *(ou16 *) MMat_at(res, c + 1, r);
                ou16 up_left = r == res->size.y - 1 ? 0 : (c == 0 ? 0 : *(ou16 *) MMat_at(res, c - 1, r + 1));
                ou16 up_right =
                        r == res->size.y - 1 ? 0 : (c == res->size.x - 1 ? 0 : *(ou16 *) MMat_at(res, c + 1, r + 1));
                ou16 *set = MMat_at(res, c, r);
                ou16 value = o_min(o_min(o_min(up, right), up_left), up_right) + 1;
                *set = o_min(*set, value);
            }
        }
    }
}

struct oobj_opt MMat_distance_transform_try(oobj obj, bool full)
{
    ivec2 size = MMat_size_int(obj);
    struct oobj_opt res = MMat_new_try(obj, NULL, m_2(size), M_FORMAT_1_UR16);
    if(!res.o) {
        return res;
    }
    MMat_distance_transform_into(obj, res.o, full);
    return res;
}
