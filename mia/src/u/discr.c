#include "u/discr.h"
#include "o/OArray.h"
#include "m/vec/vec2.h"
#include "m/vec/ivec2.h"
#include "m/types/int.h"
#include "u/pose.h"

oobj u_discr_as_boxes(oobj points_array, struct r_box init)
{
    osize num = OArray_num(points_array);
    oobj res = OArray_new(points_array, NULL, sizeof(struct r_box), num);
    const vec2 *src_v = OArray_data(points_array, const vec2);
    struct r_box *dst_v = OArray_data(res, struct r_box);
    for(osize i=0; i<num; i++) {
        const vec2 *src = &src_v[i];
        struct r_box *dst = &dst_v[i];
        *dst = init;
        dst->rect.x = src->x;
        dst->rect.y = src->y;
    }
    return res;
}

oobj u_discr_as_quads(oobj points_array, struct r_quad init)
{
    osize num = OArray_num(points_array);
    oobj res = OArray_new(points_array, NULL, sizeof(struct r_quad), num);
    const vec2 *src_v = OArray_data(points_array, const vec2);
    struct r_quad *dst_v = OArray_data(res, struct r_quad);
    for(osize i=0; i<num; i++) {
        const vec2 *src = &src_v[i];
        struct r_quad *dst = &dst_v[i];
        *dst = init;
        u_pose_xy_set(&dst->pose, src->x, src->y);
    }
    return res;
}
