#include "m/MMat_palette.h"
#include "m/MMat_view.h"
#include "o/OTask.h"
#include "m/flt.h"
#include "m/dbl.h"
#include "m/int.h"
#include "m/utils/color.h"


#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"



//
// Indexed palettes
//

void MMat_palette_apply_into(oobj obj, oobj into, oobj palette, int palette_row)
{
    MMat_assert(obj);
    MMat *indexed = obj;
    MMat_assert(into);
    MMat *res = into;
    MMat_assert(palette);
    MMat *pal = palette;

    oi32 primitive = m_format_primitive(indexed->format);
    assert(primitive == M_FORMAT__PRIMITIVE_U8 || primitive == M_FORMAT__PRIMITIVE_UR8);
    assert(ivec2_equals_v(res->size, indexed->size));
    assert(res->format == pal->format);

    osize cols = pal->size.x;
    palette_row = o_mod(palette_row, pal->size.y);
    osize format_size = m_format_size(res->format);

    osize num = MMat_num(indexed);
    for (osize i = 0; i < num; i++) {
        ou8 index = *(ou8*) MMat_at_idx_raw(indexed, i);
        index %= cols;
        void *restrict dst_data = MMat_at_idx_raw(res, i);
        const void *restrict src_data = MMat_at_raw(pal, index, palette_row);
        o_memcpy(dst_data, src_data, format_size, 1);
    }
}
struct oobj_opt MMat_palette_apply_try(oobj obj, oobj palette, int palette_row)
{
    ivec2 size = MMat_size_int(obj);
    oi32 format = MMat_format(palette);
    struct oobj_opt res = MMat_new_try(obj, NULL, m_2(size), format);
    if(!res.o) {
        return res;
    }
    MMat_palette_apply_into(obj, res.o, palette, palette_row);
    return res;
}


void MMat_palette_nearest_into(oobj obj, oobj into, oobj palette, int palette_row)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat_assert(into);
    MMat *indexed = into;
    MMat_assert(palette);
    MMat *pal = palette;

    assert(self->format == pal->format);
    assert(ivec2_equals_v(self->size, indexed->size));
    assert(indexed->format == M_FORMAT_1_UR8);

    oobj container = OObj_new(obj);

    oi32 format = self->format;
    int channels = m_format_channels(format);
    oi32 format_dbl = M_FORMAT_X(channels, M_FORMAT__PRIMITIVE_F64);

    // the palette shouldnt be that big so no need to try...
    MMat *pal_dbl = MMat_cast(pal, format_dbl);
    o_move(pal_dbl, container);
    double *values = o_new(container, *values, channels);

    osize num = MMat_num(self);
    osize cols = o_clamp(pal->size.x, 0, 255);
    palette_row = o_mod(palette_row, pal->size.y);

    for (osize i=0; i<num; i++) {
        ou8 *restrict idx_data = MMat_at_idx_raw(indexed, i);
        const void *restrict self_data = MMat_at_idx_raw(self, i);
        m_format_cast_to_double(values, self_data, format);

        double min_dist = md_MAX;
        int min_dist_p = 0;
        for (osize p=0; p<cols; p++) {
            const double *restrict pal_values = MMat_at_raw(pal_dbl, p, palette_row);
            double dist = dvecn_distance(values, pal_values, channels);
            if (dist < min_dist) {
                min_dist = dist;
                min_dist_p = p;
            }
        }
        *idx_data = min_dist_p;
    }

    o_del(container);
}
struct oobj_opt MMat_palette_nearest_try(oobj obj, oobj palette, int palette_row)
{
    ivec2 size = MMat_size_int(obj);
    struct oobj_opt res = MMat_new_try(obj, NULL, m_2(size), M_FORMAT_1_UR8);
    if(!res.o) {
        return res;
    }
    MMat_palette_nearest_into(obj, res.o, palette, palette_row);
    return res;
}

void MMat_palette_as_fixed_transparency(oobj obj, int palette_row)
{
    MMat_assert(obj);
    MMat *self = obj;
    oi32 format = self->format;
    osize format_size = m_format_size(format);
    assert(m_format_channels(format) >= 4);
    assert(format_size <= M_FORMAT_MAX_SIZE);

    // find most transparent
    float min_alpha = md_MAX;
    int index = 0;
    for (osize i=0; i<self->size.x; i++) {
        const void *restrict data = MMat_at_raw(self, i, palette_row);
        vec4 value = m_format_cast_to_vec4(data, format);
        if (value.a < min_alpha) {
            min_alpha = value.a;
            index = i;
        }
    }

    // swap most transparent to front
    if (index != 0) {
        obyte tmp[M_FORMAT_MAX_SIZE];
        void *restrict a = MMat_at_raw(self, 0, palette_row);
        void *restrict b = MMat_at_raw(self, index, palette_row);
        o_memcpy(tmp, a, format_size, 1);
        o_memcpy(a, b, format_size, 1);
        o_memcpy(b, tmp, format_size, 1);
    }

    // [0]  -> transparent
    // [1:] -> opaque
    for (osize i=0; i<self->size.x; i++) {
        double tmp[M_FORMAT_MAX_SIZE];
        void *restrict data = MMat_at_raw(self, i, palette_row);
        m_format_cast_to_double(tmp, data, format);
        tmp[3] = i==0? 0.0 : 1.0;
        m_format_cast_from_double(data, format, tmp);
    }
}


static int mmat_palette_alpha_cmp(void *obj, const void *a, const void *b)
{
    MMat *self = obj;
    vec4 va = m_format_cast_to_vec4(a, self->format);
    vec4 vb = m_format_cast_to_vec4(b, self->format);
    return m_sign(va.a - vb.a);
}
static int mmat_palette_hue_cmp(void *obj, const void *a, const void *b)
{
    MMat *self = obj;
    vec3 va = vec3_rgb2hsv(m_format_cast_to_vec4(a, self->format).xyz);
    vec3 vb = vec3_rgb2hsv(m_format_cast_to_vec4(b, self->format).xyz);
    return m_sign(va.v0 - vb.v0);
}
static int mmat_palette_sat_cmp(void *obj, const void *a, const void *b)
{
    MMat *self = obj;
    vec3 va = vec3_rgb2hsv(m_format_cast_to_vec4(a, self->format).xyz);
    vec3 vb = vec3_rgb2hsv(m_format_cast_to_vec4(b, self->format).xyz);
    return m_sign(va.v1 - vb.v1);
}
static int mmat_palette_val_cmp(void *obj, const void *a, const void *b)
{
    MMat *self = obj;
    vec3 va = vec3_rgb2hsv(m_format_cast_to_vec4(a, self->format).xyz);
    vec3 vb = vec3_rgb2hsv(m_format_cast_to_vec4(b, self->format).xyz);
    return m_sign(va.v2 - vb.v2);
}

void MMat_palette_sort_alpha(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(m_format_channels(self->format) >= 4);
    MMat_sort(self, mmat_palette_alpha_cmp);
}

void MMat_palette_sort_hue(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(m_format_channels(self->format) >= 3);
    MMat_sort(self, mmat_palette_hue_cmp);
}

void MMat_palette_sort_sat(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(m_format_channels(self->format) >= 3);
    MMat_sort(self, mmat_palette_sat_cmp);
}

void MMat_palette_sort_val(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(m_format_channels(self->format) >= 3);
    MMat_sort(self, mmat_palette_val_cmp);
}



void MMat_palette_sort_hsv(oobj obj, int palette_row, m_ivec2 splits_vh)
{
    MMat_assert(obj);
    MMat *self = obj;
    oi32 format = self->format;
    osize format_size = m_format_size(format);
    assert(m_format_channels(format) >= 3);
    assert(format_size <= M_FORMAT_MAX_SIZE);

    if (splits_vh.v0 <= 1) {
        splits_vh.v0 = 2;
    }
    if (splits_vh.v1 <= 1) {
        splits_vh.v1 = 2;
    }
    const int val_splits = splits_vh.v0;
    const int hue_splits = splits_vh.v1;

    if (self->size.x <= 1) {
        return;
    }

    MMat *window_sat = MMat_window(self, MMat_at(self, 0, palette_row), ivec2_(self->size.x, 1), ivec2_(0));
    MMat_palette_sort_sat(window_sat);

    int val_size = window_sat->size.x / val_splits;
    for (int val=0; val<val_splits; val++) {
        int val_size_window = val_size;
        if (val == val_splits-1) {
            val_size_window = window_sat->size.x - val_size*3;
        }
        if (val_size_window <= 1) {
            continue;
        }
        MMat *window_val = MMat_window(window_sat, MMat_at(window_sat, val_size*val, 0), ivec2_(val_size_window, 1), ivec2_(0));
        MMat_palette_sort_val(window_val);

        int hue_size = window_val->size.x / hue_splits;
        for (int hue=0; hue<hue_splits; hue++) {
            int hue_size_window = hue_size;
            if (hue == hue_splits-1) {
                hue_size_window = window_val->size.x - hue_size*3;
            }
            if (hue_size_window <= 1) {
                continue;
            }
            MMat *window_hue = MMat_window(window_val, MMat_at(window_val, hue_size*hue, 0), ivec2_(hue_size_window, 1), ivec2_(0));
            MMat_palette_sort_hue(window_hue);
        }
    }


    o_del(window_sat);
}

void MMat_palette_sort_hsva(oobj obj, int palette_row, m_ivec3 splits_svh)
{
    MMat_assert(obj);
    MMat *self = obj;
    oi32 format = self->format;
    osize format_size = m_format_size(format);
    assert(m_format_channels(format) >= 4);
    assert(format_size <= M_FORMAT_MAX_SIZE);

    if (splits_svh.v0 <= 1) {
        splits_svh.v0 = 2;
    }
    const int sat_splits = splits_svh.v0;

    if (self->size.x <= 1) {
        return;
    }

    MMat *window_alpha = MMat_window(self, MMat_at(self, 0, palette_row), ivec2_(self->size.x, 1), ivec2_(0));
    MMat_palette_sort_alpha(window_alpha);

    int sat_size = window_alpha->size.x / sat_splits;
    for (int sat=0; sat<sat_splits; sat++) {
        int sat_size_window = sat_size;
        if (sat == sat_splits-1) {
            sat_size_window = window_alpha->size.x - sat_size*3;
        }
        if (sat_size_window <= 1) {
            continue;
        }
        MMat *window_sat = MMat_window(window_alpha, MMat_at(window_alpha, sat_size*sat, 0), ivec2_(sat_size_window, 1), ivec2_(0));
        MMat_palette_sort_hsv(window_sat, 0, splits_svh.yz);
    }


    o_del(window_alpha);
}




struct quantize_context {
    MMat *palette;
    double *img;
    double *pal;
    double *sum;
    int *cnts;
    int channels;
    osize img_num;
    int pal_num;
    double prev_avg_error;
    double econd_avg_error_change;
    int econd_max_runs;
};

static float quantize_task(oobj task, int run)
{
    struct quantize_context *restrict C = o_user(task);

    //
    // clear cached tun data
    //
    o_clear(C->cnts, sizeof(int), C->pal_num);
    o_clear(C->sum, sizeof(double) * C->channels, C->pal_num);

    double max_error = 0;
    osize max_error_idx = -1;
    double avg_error = 0;

    // find nearest neighbor and calc avg color and error
    for (osize i=0; i<C->img_num; i++) {
        const double *restrict img_vals = C->img + i*C->channels;
        double min_error = md_MAX;
        int min_error_p = 0;
        for (int p=0; p<C->pal_num; p++) {
            const double *restrict pal_vals = C->pal + p*C->channels;
            double dist = dvecn_distance(img_vals, pal_vals, C->channels);
            if (dist < min_error) {
                min_error = dist;
                min_error_p = p;
            }
        }
        if (min_error > max_error) {
            max_error = min_error;
            max_error_idx = i;
        }
        avg_error += min_error;
        C->cnts[min_error_p]++;
        double *restrict sum_vals = C->sum + min_error_p*C->channels;
        dvecn_add_v(sum_vals, sum_vals, img_vals, C->channels);
    }
    
    avg_error /= C->img_num;

    // recalc palette accoriding to avg neighbor color, if no neighbor found, use the farest away color
    for(int p=0; p<C->pal_num; p++) {
        double *restrict pal_data = C->pal + p*C->channels;
        if(C->cnts[p] <= 0) {
            // use the max_error color instead
            const double *restrict img_data = C->img + max_error_idx*C->channels;
            o_memcpy(pal_data, img_data, sizeof(double), C->channels);
            continue;
        }
        double *restrict sum_vals = C->sum + p*C->channels;
        dvecn_div(pal_data, sum_vals, C->cnts[p], C->channels);
    }

    o_log_debug_s("MMat_palette_quantize", "avg_error: %f", avg_error);

    double avg_error_change = m_abs(avg_error - C->prev_avg_error);
    C->prev_avg_error = avg_error;

    float progress = (float) (run+1) / (float) C->econd_max_runs;
    if (avg_error_change <= C->econd_avg_error_change) {
        progress = OTask_FINISH;
    }

    // if finished, cast back to original palette format
    if (progress >= OTask_FINISH) {
        o_log_debug_s("MMat_palette_quantize", "done");
        oi32 format = MMat_format(C->palette);
        for(int p=0; p<C->pal_num; p++) {
            const double *restrict pal_data = C->pal + p*C->channels;
            void *restrict palette_data = MMat_at_idx_raw(C->palette, p);
            m_format_cast_from_double(palette_data, format, pal_data);
        }
    }

    return progress;
}

struct oobj_opt MMat_palette_quantize_into_task(oobj obj, oobj into, double econd_avg_error_change, int econd_max_runs)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat_assert(into);
    MMat *pal = into;

    if (econd_avg_error_change <= 0) {
        econd_avg_error_change = 0.0005;
    }
    if (econd_max_runs <= 0) {
        econd_max_runs = 20;
    }

    assert(pal->format == self->format);
    int channels = m_format_channels(self->format);
    oi32 sum_format = M_FORMAT_X(channels, M_FORMAT__PRIMITIVE_F64);
    assert(m_format_size(sum_format) <= M_FORMAT_MAX_SIZE && "to many channels!");
    int pal_num = pal->size.x;
    osize self_num = MMat_num(self);
    pal_num = o_min(self_num, pal_num);
    osize format_size = m_format_size(pal->format);
    oi32 format_dbl = M_FORMAT_X(channels, M_FORMAT__PRIMITIVE_F64);

    
    // get init colors
    int *init = o_new(obj, *init, pal_num);
    for(int i=0; i<pal_num; i++) {
        init[i] = o_rand() % self_num;
    }
    for(int i=0; i<pal_num; i++) {
        const void *restrict src = MMat_at_idx_raw(self, init[i]);
        void *restrict dst = MMat_at_idx_raw(pal, i);
        o_memcpy(dst, src, format_size, 1);
    }
    o_free(obj, init);
    // in the (rare) case that the img is < palette
    for(int i=pal_num; i<pal->size.x; i++) {
        void *restrict dst = MMat_at_idx_raw(pal, i);
        o_clear(dst, format_size, 1);
    }

    oobj task = OTask_new(obj, quantize_task, true);
    // allocations that may fail:
    oobj img_dbl = MMat_cast_try(self, format_dbl).o;
    oobj pal_dbl = MMat_cast_try(pal, format_dbl).o;
    oobj sum_dbl = MMat_clone_try(pal_dbl).o;
    o_move(img_dbl, task);
    o_move(pal_dbl, task);
    o_move(sum_dbl, task);
    if (!sum_dbl || !pal_dbl || !img_dbl) {
        o_log_error_s("MMat_palette_quantize", "Allocation failed");
        o_del(task);
        return oobj_opt(NULL);
    }


    struct quantize_context *C = o_user_new0(task, *C, 1);
    C->palette = pal;
    C->img = MMat_data(img_dbl);
    C->pal = MMat_data(pal_dbl);
    C->sum = MMat_data(sum_dbl);
    C->cnts = o_new(task, *C->cnts, pal_num);
    C->channels = channels;
    C->img_num = self_num;
    C->pal_num = pal_num;
    C->prev_avg_error = md_MAX;
    C->econd_avg_error_change = econd_avg_error_change;
    C->econd_max_runs = econd_max_runs;

    return oobj_opt(task);
}
struct oobj_opt MMat_palette_quantize_try(oobj obj, int num)
{
    struct oobj_opt res = MMat_new_try(obj, NULL, num, 1, MMat_format(obj));
    if(!res.o) {
        return oobj_opt(NULL);
    }
    oobj task = MMat_palette_quantize_into_task(obj, res.o, 0, 0).o;
    if (!task) {
        o_del(res.o);
        return oobj_opt(NULL);
    }
    OTask_run_blocking(task);
    o_del(task);
    return res;
}

