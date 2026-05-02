#include "m/MMat_math.h"
#include "m/flt.h"
#include "m/dbl.h"
#include "m/byte.h"


#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"


//
// Math Operations
//


void MMat_min_x(oobj obj, void *restrict out_min)
{
    MMat_assert(obj);
    MMat *self = obj;
    ou32 format = self->format;
    osize num = MMat_num(self);


    // fast path
    if (format == M_FORMAT_1_U8 || format == M_FORMAT_1_UR8) {
        ou8 value = ou8_MAX;
        for (osize i = 0; i < num; i++) {
            ou8 v = *(ou8 *) MMat_at_idx_raw(self, i);
            value = o_min(value, v);
        }
        *(ou8*)out_min = value;
    }
    if (format == M_FORMAT_4_U8) {
        bvec4 value = bvec4_(ou8_MAX);
        for (osize i = 0; i < num; i++) {
            bvec4 *restrict v = MMat_at_idx_raw(self, i);
            value = bvec4_min_v(value, *v);
        }
        *(bvec4*)out_min = value;
    }
    if (format == M_FORMAT_1_F32) {
        of32 value = of32_MAX;
        for (osize i = 0; i < num; i++) {
            of32 v = *(of32 *) MMat_at_idx_raw(self, i);
            value = o_min(value, v);
        }
        *(of32*)out_min = value;
    }
    if (format == M_FORMAT_4_F32) {
        vec4 value = vec4_(of32_MAX);
        for (osize i = 0; i < num; i++) {
            vec4 *restrict v = MMat_at_idx_raw(self, i);
            value = vec4_min_v(value, *v);
        }
        *(vec4*)out_min = value;
    }

    // generic
    {
        int channels = m_format_channels(format);
        assert(channels <= M_FORMAT_MAX_SIZE);
        double value[M_FORMAT_MAX_SIZE];
        double ddata[M_FORMAT_MAX_SIZE];
        dvecn_set(value, of64_MAX, channels);

        for (osize i = 0; i < num; i++) {
            void *restrict data = MMat_at_idx_raw(self, i);
            m_format_cast_to_double(ddata, data, format);
            dvecn_min_v(value, value, ddata, channels);
        }
        m_format_cast_from_double(out_min, format, value);
    }
}

vec4 MMat_min_vec4(oobj obj)
{
    obyte x_min[M_FORMAT_MAX_SIZE];
    MMat_min_x(obj, x_min);
    return m_format_cast_to_vec4(x_min, MMat_format(obj));
}

void MMat_max_x(oobj obj, void *restrict out_max)
{
    MMat_assert(obj);
    MMat *self = obj;
    ou32 format = self->format;
    osize num = MMat_num(self);


    // fast path
    if (format == M_FORMAT_1_U8 || format == M_FORMAT_1_UR8) {
        ou8 value = ou8_MIN;
        for (osize i = 0; i < num; i++) {
            ou8 v = *(ou8 *) MMat_at_idx_raw(self, i);
            value = o_max(value, v);
        }
        *(ou8*)out_max = value;
    }
    if (format == M_FORMAT_4_U8) {
        bvec4 value = bvec4_(ou8_MIN);
        for (osize i = 0; i < num; i++) {
            bvec4 *restrict v = MMat_at_idx_raw(self, i);
            value = bvec4_max_v(value, *v);
        }
        *(bvec4*)out_max = value;
    }
    if (format == M_FORMAT_1_F32) {
        of32 value = of32_MIN;
        for (osize i = 0; i < num; i++) {
            of32 v = *(of32 *) MMat_at_idx_raw(self, i);
            value = o_max(value, v);
        }
        *(of32*)out_max = value;
    }
    if (format == M_FORMAT_4_F32) {
        vec4 value = vec4_(of32_MIN);
        for (osize i = 0; i < num; i++) {
            vec4 *restrict v = MMat_at_idx_raw(self, i);
            value = vec4_max_v(value, *v);
        }
        *(vec4*)out_max = value;
    }

    // generic
    {
        int channels = m_format_channels(format);
        assert(channels <= M_FORMAT_MAX_SIZE);
        double value[M_FORMAT_MAX_SIZE];
        double ddata[M_FORMAT_MAX_SIZE];
        dvecn_set(value, of64_MIN, channels);

        for (osize i = 0; i < num; i++) {
            void *restrict data = MMat_at_idx_raw(self, i);
            m_format_cast_to_double(ddata, data, format);
            dvecn_max_v(value, value, ddata, channels);
        }
        m_format_cast_from_double(out_max, format, value);
    }
}

vec4 MMat_max_vec4(oobj obj)
{
    obyte x_max[M_FORMAT_MAX_SIZE];
    MMat_max_x(obj, x_max);
    return m_format_cast_to_vec4(x_max, MMat_format(obj));
}

void MMat_minmax_x(oobj obj, void *restrict out_min, void *restrict out_max)
{
    MMat_assert(obj);
    MMat *self = obj;
    ou32 format = self->format;
    osize num = MMat_num(self);


    // fast path
    if (format == M_FORMAT_1_U8 || format == M_FORMAT_1_UR8) {
        ou8 min_value = ou8_MAX;
        ou8 max_value = ou8_MIN;
        for (osize i = 0; i < num; i++) {
            ou8 v = *(ou8 *) MMat_at_idx_raw(self, i);
            min_value = o_min(min_value, v);
            max_value = o_max(max_value, v);
        }
        *(ou8*)out_min = min_value;
        *(ou8*)out_max = max_value;
    }
    if (format == M_FORMAT_4_U8) {
        bvec4 min_value = bvec4_(ou8_MAX);
        bvec4 max_value = bvec4_(ou8_MIN);
        for (osize i = 0; i < num; i++) {
            bvec4 *restrict v = MMat_at_idx_raw(self, i);
            min_value = bvec4_min_v(min_value, *v);
            max_value = bvec4_max_v(max_value, *v);
        }
        *(bvec4*)out_min = min_value;
        *(bvec4*)out_max = max_value;
    }
    if (format == M_FORMAT_1_F32) {
        of32 min_value = of32_MAX;
        of32 max_value = of32_MIN;
        for (osize i = 0; i < num; i++) {
            of32 v = *(of32 *) MMat_at_idx_raw(self, i);
            min_value = o_min(min_value, v);
            max_value = o_max(max_value, v);
        }
        *(of32*)out_min = min_value;
        *(of32*)out_max = max_value;
    }
    if (format == M_FORMAT_4_F32) {
        vec4 min_value = vec4_(of32_MAX);
        vec4 max_value = vec4_(of32_MIN);
        for (osize i = 0; i < num; i++) {
            vec4 *restrict v = MMat_at_idx_raw(self, i);
            min_value = vec4_min_v(min_value, *v);
            max_value = vec4_max_v(max_value, *v);
        }
        *(vec4*)out_min = min_value;
        *(vec4*)out_max = max_value;
    }

    // generic
    {
        int channels = m_format_channels(format);
        assert(channels <= M_FORMAT_MAX_SIZE);
        double min_value[M_FORMAT_MAX_SIZE];
        double max_value[M_FORMAT_MAX_SIZE];
        double ddata[M_FORMAT_MAX_SIZE];
        dvecn_set(min_value, of64_MAX, channels);
        dvecn_set(max_value, of64_MIN, channels);

        for (osize i = 0; i < num; i++) {
            void *restrict data = MMat_at_idx_raw(self, i);
            m_format_cast_to_double(ddata, data, format);
            dvecn_min_v(min_value, min_value, ddata, channels);
            dvecn_max_v(max_value, max_value, ddata, channels);
        }
        m_format_cast_from_double(out_min, format, min_value);
        m_format_cast_from_double(out_max, format, max_value);
    }
}
void MMat_minmax_vec4(oobj obj, vec4 *out_min, vec4 *out_max)
{
    obyte x_min[M_FORMAT_MAX_SIZE];
    obyte x_max[M_FORMAT_MAX_SIZE];
    MMat_minmax_x(obj, x_min, x_max);
    *out_min = m_format_cast_to_vec4(x_min, MMat_format(obj));
    *out_max = m_format_cast_to_vec4(x_max, MMat_format(obj));
}

void MMat_add_dbl(oobj obj, const double *restrict add)
{
    MMat_assert(obj);
    MMat *self = obj;
    ou32 format = self->format;
    osize num = MMat_num(self);


    // fast path
    if (format == M_FORMAT_1_U8 || format == M_FORMAT_1_UR8) {
        double value = add[0];
        if(format == M_FORMAT_1_U8) {
            value *= 255.0;
        }
        for (osize i = 0; i < num; i++) {
            ou8 *restrict v = MMat_at_idx_raw(self, i);
            double tmp = (double) *v + value;
            tmp = md_round(tmp);
            *v = o_clamp(tmp, 0, 255);
        }
        return;
    }
    if (format == M_FORMAT_4_U8) {
        dvec4 value = dvec4_scale(*(dvec4*) add, 255.0);
        for (osize i = 0; i < num; i++) {
            bvec4 *restrict v = MMat_at_idx_raw(self, i);
            dvec4 tmp = dvec4_add_v(dvec4_cast_byte(v->v), value);
            tmp = dvec4_clamp(tmp, 0, 255);
            tmp = dvec4_round(tmp);
            *v = bvec4_cast_double(tmp.v);
        }
        return;
    }
    if (format == M_FORMAT_1_F32) {
        of32 value = add[0] * 255.0;
        for (osize i = 0; i < num; i++) {
            of32 *restrict v = MMat_at_idx_raw(self, i);
            *v = o_clamp(*v + value, 0.0f, 1.0f);
        }
        return;
    }
    if (format == M_FORMAT_4_F32) {
        vec4 value = vec4_scale(vec4_cast_double(add), 255);
        for (osize i = 0; i < num; i++) {
            vec4 *restrict v = MMat_at_idx_raw(self, i);
            *v = vec4_clamp(vec4_add_v(*v, value), 0, 1);
        }
        return;
    }

    // generic
    {
        int channels = m_format_channels(format);
        assert(channels <= M_FORMAT_MAX_SIZE);
        double ddata[M_FORMAT_MAX_SIZE];
        double value[M_FORMAT_MAX_SIZE];
        dvecn_scale(value, add, 255.0, channels);
        for (osize i = 0; i < num; i++) {
            void *restrict data = MMat_at_idx_raw(self, i);
            m_format_cast_to_double(ddata, data, format);
            dvecn_add_v(ddata, ddata, value, channels);
            m_format_cast_from_double(data, format, ddata);
        }
        return;
    }
}

void MMat_add_vec4(oobj obj, vec4 add)
{
    double d_add[M_FORMAT_MAX_SIZE] = {0};
    *(dvec4*) d_add = dvec4_cast_float(add.v);
    MMat_add_dbl(obj, d_add);
}


void MMat_scale_dbl(oobj obj, const double *restrict scale)
{
    MMat_assert(obj);
    MMat *self = obj;
    ou32 format = self->format;
    osize num = MMat_num(self);

    // fast path
    if (format == M_FORMAT_1_U8 || format == M_FORMAT_1_UR8) {
        double value = scale[0];
        for (osize i = 0; i < num; i++) {
            ou8 *restrict v = MMat_at_idx_raw(self, i);
            double tmp = (double) *v * value;
            tmp = md_round(tmp);
            *v = o_clamp(tmp, 0, 255);
        }
        return;
    }
    if (format == M_FORMAT_4_U8) {
        dvec4 value = *(dvec4*) scale;
        for (osize i = 0; i < num; i++) {
            bvec4 *restrict v = MMat_at_idx_raw(self, i);
            dvec4 tmp = dvec4_scale_v(dvec4_cast_byte(v->v), value);
            tmp = dvec4_clamp(tmp, 0, 255);
            tmp = dvec4_round(tmp);
            *v = bvec4_cast_double(tmp.v);
        }
        return;
    }
    if (format == M_FORMAT_1_F32) {
        of32 value = scale[0];
        for (osize i = 0; i < num; i++) {
            of32 *restrict v = MMat_at_idx_raw(self, i);
            *v = o_clamp(*v * value, 0.0f, 1.0f);
        }
        return;
    }
    if (format == M_FORMAT_4_F32) {
        vec4 value = vec4_cast_double(scale);
        for (osize i = 0; i < num; i++) {
            vec4 *restrict v = MMat_at_idx_raw(self, i);
            *v = vec4_clamp(vec4_scale_v(*v, value), 0, 1);
        }
        return;
    }

    // generic
    {
        int channels = m_format_channels(format);
        assert(channels <= M_FORMAT_MAX_SIZE);
        double ddata[M_FORMAT_MAX_SIZE];
        for (osize i = 0; i < num; i++) {
            void *restrict data = MMat_at_idx_raw(self, i);
            m_format_cast_to_double(ddata, data, format);
            dvecn_scale_v(ddata, ddata, scale, channels);
            m_format_cast_from_double(data, format, ddata);
        }
        return;
    }
}

void MMat_scale_vec4(oobj obj, vec4 scale)
{
    double d_scale[M_FORMAT_MAX_SIZE] = {0};
    *(dvec4*) d_scale = dvec4_cast_float(scale.v);
    MMat_scale_dbl(obj, d_scale);
}

void MMat_add_and_scale_dbl(oobj obj, const double *restrict add, const double *restrict scale)
{
    MMat_assert(obj);
    MMat *self = obj;
    ou32 format = self->format;
    osize num = MMat_num(self);


    // fast path
    if (format == M_FORMAT_1_U8 || format == M_FORMAT_1_UR8) {
        double add_value = add[0];
        if(format == M_FORMAT_1_U8) {
            add_value *= 255.0;
        }
        double scale_value = scale[0];
        for (osize i = 0; i < num; i++) {
            ou8 *restrict v = MMat_at_idx_raw(self, i);
            double tmp = ((double) *v + add_value) * scale_value;
            tmp = md_round(tmp);
            *v = o_clamp(tmp, 0, 255);
        }
        return;
    }
    if (format == M_FORMAT_4_U8) {
        dvec4 add_value = dvec4_scale(*(dvec4*) add, 255.0);
        dvec4 scale_value = *(dvec4*) scale;
        for (osize i = 0; i < num; i++) {
            bvec4 *restrict v = MMat_at_idx_raw(self, i);
            dvec4 tmp = dvec4_add_v(dvec4_cast_byte(v->v), add_value);
            tmp = dvec4_scale_v(tmp, scale_value);
            tmp = dvec4_clamp(tmp, 0, 255);
            tmp = dvec4_round(tmp);
            *v = bvec4_cast_double(tmp.v);
        }
        return;
    }
    if (format == M_FORMAT_1_F32) {
        of32 add_value = add[0] * 255.0;
        of32 scale_value = scale[0];
        for (osize i = 0; i < num; i++) {
            of32 *restrict v = MMat_at_idx_raw(self, i);
            of32 tmp = (*v + add_value) * scale_value;
            *v = o_clamp(tmp, 0.0f, 1.0f);
        }
        return;
    }
    if (format == M_FORMAT_4_F32) {
        vec4 add_value = vec4_scale(vec4_cast_double(add), 255);
        vec4 scale_value = vec4_cast_double(scale);
        for (osize i = 0; i < num; i++) {
            vec4 *restrict v = MMat_at_idx_raw(self, i);
            vec4 tmp = vec4_scale_v(vec4_add_v(*v, add_value), scale_value);
            *v = vec4_clamp(tmp, 0.0f, 1.0f);
        }
        return;
    }

    // generic
    {
        int channels = m_format_channels(format);
        assert(channels <= M_FORMAT_MAX_SIZE);
        double ddata[M_FORMAT_MAX_SIZE];
        double add_value[M_FORMAT_MAX_SIZE];
        dvecn_scale(add_value, add, 255.0, channels);
        for (osize i = 0; i < num; i++) {
            void *restrict data = MMat_at_idx_raw(self, i);
            m_format_cast_to_double(ddata, data, format);
            dvecn_add_v(ddata, ddata, add_value, channels);
            dvecn_scale_v(ddata, ddata, scale, channels);
            m_format_cast_from_double(data, format, ddata);
        }
        return;
    }
}

void MMat_add_and_scale_vec4(oobj obj, vec4 add, vec4 scale)
{
    double d_add[M_FORMAT_MAX_SIZE] = {0};
    *(dvec4*) d_add = dvec4_cast_float(add.v);
    double d_scale[M_FORMAT_MAX_SIZE] = {0};
    *(dvec4*) d_scale = dvec4_cast_float(scale.v);
    MMat_add_and_scale_dbl(obj, d_add, d_scale);
}

void MMat_normalize(oobj obj)
{
    ou32 format = MMat_format(obj);
    int channels = m_format_channels(format);
    obyte x_min[M_FORMAT_MAX_SIZE];
    obyte x_max[M_FORMAT_MAX_SIZE];
    double d_min[M_FORMAT_MAX_SIZE];
    double d_max[M_FORMAT_MAX_SIZE];
    double d_1[M_FORMAT_MAX_SIZE];

    // fine to share to reduce stack usage
    double *d_range = d_max;
    double *d_add = d_min;
    double *d_scale = d_max;

    MMat_minmax_x(obj, x_min, x_max);
    m_format_cast_to_double(d_min, x_min, format);
    m_format_cast_to_double(d_max, x_max, format);
    dvecn_sub_v(d_range, d_max, d_min, channels);
    dvecn_neg(d_add, d_min, channels);
    dvecn_set(d_1, 1, channels);
    dvecn_div_v(d_scale, d_1, d_range, channels);
    MMat_add_and_scale_dbl(obj, d_add, d_scale);
}
