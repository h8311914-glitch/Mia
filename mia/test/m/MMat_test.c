#include "m/MMat_ex.h"
#include "m/flt.h"


#define test(expr) o_assume(expr, "test failed")

static int mat_sort_cmp(oobj mat, const void *a, const void *b)
{
    const of32 *af = a;
    const of32 *bf = b;
    return *af - *bf;
}

static void mat_sort(oobj obj)
{
    oobj mat = MMat_new(obj, NULL, 8, 8, M_FORMAT_1_F32);
    for(osize i=0; i<MMat_num(mat); i++) {
        of32 *at = MMat_at_idx(mat, i);
        *at = o_rand() % 128;
    }
    
    MMat_sort(mat, mat_sort_cmp);
    
    of32 prev = *(of32*) MMat_at_idx(mat, 0);
    for(osize i=1; i<MMat_num(mat); i++) {
        of32 *at = MMat_at_idx(mat, i);
        test(*at >= prev);
        prev = *at;
    }
}

static void mat_normalize(oobj obj)
{
    ou8 buf[4] = {51, 102, 153, 204};
    ou8 buf_norm[4] = {0, 85, 170, 255};

    oobj mat;

    mat = MMat_new(obj, buf, 2, 2, M_FORMAT_1_U8);
    test(MMat_data_size(mat) == 4);
    MMat_normalize(mat);
    ou8 *mat_data = MMat_data(mat);
    test(o_equals(mat_data, buf_norm, 1, 4));
}

static void mat_add_and_scale(oobj obj)
{
    ou8 val = 32;
    oobj mat = MMat_new(obj, &val, 1, 1, M_FORMAT_1_U8);
    MMat_add_and_scale_vec4(mat, vec4_(0.5), vec4_(2.0));
    //vec4 res = MMat_at_vec
}

static void mat_json_transform_check(oobj mat)
{
    test(MMat_valid(mat));
    oobj json = MMat_json(mat, mat, NULL);
    test(json != NULL);

    oobj mat_2 = MMat_new_json(mat, json, MMat_format(mat)).o;
    test(MMat_valid(mat_2));

    test(MMat_equals(mat, mat_2));
}

static void mat_json(oobj parent)
{
    // 0-dim
    {
        oobj mat = MMat_new(parent, NULL, 1, 1, M_FORMAT_1_UR8);
        *(ou8*)MMat_at_idx(mat, 0) = 7;
        mat_json_transform_check(mat);
    }

    // 1-dim
    {
        oobj mat = MMat_new(parent, NULL, 1, 3, M_FORMAT_1_IR16);
        for(osize i = 0; i < MMat_num(mat); i++) {
            *(oi16*)MMat_at_idx(mat, i) = 10 + i;
        }
        mat_json_transform_check(mat);
    }

    // 2-dim
    {
        oobj mat = MMat_new(parent, NULL, 2, 2, M_FORMAT_1_F32);
        for(osize i = 0; i < MMat_num(mat); i++) {
            *(of32*)MMat_at_idx(mat, i) = 20 + i;
        }
        mat_json_transform_check(mat);
    }

    // 3-dim
    {
        oobj mat = MMat_new(parent, NULL, 2, 2, M_FORMAT_X(2, M_FORMAT__PRIMITIVE_F64));
        for(osize i = 0; i < MMat_num(mat); i++) {
            of64 *px = MMat_at_idx(mat, i);
            px[0] = 30 + i * 2 + 0;
            px[1] = 30 + i * 2 + 1;
        }
        mat_json_transform_check(mat);
    }
}


int MMat__test(oobj obj)
{
    mat_sort(obj);
    mat_normalize(obj);
    mat_add_and_scale(obj);
    mat_json(obj);
    return 0;
}

