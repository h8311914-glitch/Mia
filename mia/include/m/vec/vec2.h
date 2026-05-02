#ifndef M_VEC_VEC2_H
#define M_VEC_VEC2_H

/**
 * @file vec/vec2.h
 *
 * functions working with vec2
 */


#include "m/sca/flt.h"
#include "m/m_types/flt.h"

/** dst = v0, v1, ... */
o_inline m_vec2 vec2_new(float v0, float v1)
{
    return (m_vec2){{v0, v1}};
}

/** dst = (m_vec2) v */
o_inline m_vec2 vec2_cast_double(const double *cast)
{
    return (m_vec2){{cast[0], cast[1]}};
}

/** dst = (m_vec2) v */
o_inline m_vec2 vec2_cast_int(const int *cast)
{
    return (m_vec2){{cast[0], cast[1]}};
}

/** dst = (m_vec2) v */
o_inline m_vec2 vec2_cast_byte(const obyte *cast)
{
    return (m_vec2){{cast[0], cast[1]}};
}

/** dst = v / 255 */
o_inline m_vec2 vec2_cast_byte_1(const obyte *cast)
{
    return (m_vec2){{cast[0] / 255.0f, cast[1] / 255.0f}};
}


/** a == b */
o_inline bool vec2_equals(m_vec2 a, float b)
{
    return a.v0 == b
           && a.v1 == b;
}

/** a == b */
o_inline bool vec2_equals_v(m_vec2 a, m_vec2 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1;
}

/** a ~= b */
o_inline bool vec2_equals_eps(m_vec2 a, float b, float eps)
{
    return m_equals_eps(a.v0, b, eps)
           && m_equals_eps(a.v1, b, eps);
}

/** a ~= b */
o_inline bool vec2_equals_eps_v(m_vec2 a, m_vec2 b, float eps)
{
    return m_equals_eps(a.v0, b.v0, eps)
           && m_equals_eps(a.v1, b.v1, eps);
}


/** dst = s */
o_inline m_vec2 vec2_set(float s)
{
    return (m_vec2){{s, s}};
}

/** dst = unit_x */
o_inline m_vec2 vec2_unit_x(void)
{
    return (m_vec2){{1, 0}};
}


/** dst = unit_y */
o_inline m_vec2 vec2_unit_y(void)
{
    return (m_vec2){{0, 1}};
}

/** dst = -v */
o_inline m_vec2 vec2_neg(m_vec2 v)
{
    return (m_vec2){{-v.v0, -v.v1}};
}


/** dst = a + b */
o_inline m_vec2 vec2_add_v(m_vec2 a, m_vec2 b)
{
    return (m_vec2){
        {
            a.v0 + b.v0,
            a.v1 + b.v1
        }
    };
}


/** dst = a + b */
o_inline m_vec2 vec2_add(m_vec2 a, float b)
{
    return (m_vec2){
        {
            a.v0 + b,
            a.v1 + b
        }
    };
}


/** dst = a - b */
o_inline m_vec2 vec2_sub_v(m_vec2 a, m_vec2 b)
{
    return (m_vec2){
        {
            a.v0 - b.v0,
            a.v1 - b.v1
        }
    };
}


/** dst = a - b */
o_inline m_vec2 vec2_sub(m_vec2 a, float b)
{
    return (m_vec2){
        {
            a.v0 - b,
            a.v1 - b
        }
    };
}


/** dst = a * b */
o_inline m_vec2 vec2_scale_v(m_vec2 a, m_vec2 b)
{
    return (m_vec2){
        {
            a.v0 * b.v0,
            a.v1 * b.v1
        }
    };
}


/** dst = a * b */
o_inline m_vec2 vec2_scale(m_vec2 a, float b)
{
    return (m_vec2){
        {
            a.v0 * b,
            a.v1 * b
        }
    };
}


/** dst = a / b */
o_inline m_vec2 vec2_div_v(m_vec2 a, m_vec2 b)
{
    return (m_vec2){
        {
            a.v0 / b.v0,
            a.v1 / b.v1
        }
    };
}


/** dst = a / b */
o_inline m_vec2 vec2_div(m_vec2 a, float b)
{
    return (m_vec2){
        {
            a.v0 / b,
            a.v1 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_vec2 vec2_add_scaled_v(m_vec2 a, m_vec2 b, m_vec2 c)
{
    return (m_vec2){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1
        }
    };
}

/** dst = a + b * c */
o_inline m_vec2 vec2_add_scaled(m_vec2 a, m_vec2 b, float c)
{
    return (m_vec2){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c
        }
    };
}


/** dst = a * M_PI / 180 */
o_inline m_vec2 vec2_radians(m_vec2 deg)
{
    return (m_vec2){
        {
            m_radians(deg.v0),
            m_radians(deg.v1)
        }
    };
}


/** dst = a * 180 / M_PI */
o_inline m_vec2 vec2_degrees(m_vec2 rad)
{
    return (m_vec2){
        {
            m_degrees(rad.v0),
            m_degrees(rad.v1)
        }
    };
}


/** dst = abs(x) */
o_inline m_vec2 vec2_abs(m_vec2 x)
{
    return (m_vec2){
        {
            m_abs(x.v0),
            m_abs(x.v1)
        }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_vec2 vec2_sign(m_vec2 x)
{
    return (m_vec2){
        {
            m_sign(x.v0),
            m_sign(x.v1)
        }
    };
}


/** dst = round(x) */
o_inline m_vec2 vec2_round(m_vec2 x)
{
    return (m_vec2){
        {
            m_round(x.v0),
            m_round(x.v1)
        }
    };
}


/** dst = floor(x) */
o_inline m_vec2 vec2_floor(m_vec2 x)
{
    return (m_vec2){
        {
            m_floor(x.v0),
            m_floor(x.v1)
        }
    };
}


/** dst = ceil(x) */
o_inline m_vec2 vec2_ceil(m_vec2 x)
{
    return (m_vec2){
        {
            m_ceil(x.v0),
            m_ceil(x.v1)
        }
    };
}


/** dst = x - floor(x) */
o_inline m_vec2 vec2_fract(m_vec2 x)
{
    return (m_vec2){
        {
            m_fract(x.v0),
            m_fract(x.v1)
        }
    };
}


/** dst = x - y * floor(x/y) */
o_inline m_vec2 vec2_mod(m_vec2 x, float y)
{
    return (m_vec2){
        {
            m_mod(x.v0, y),
            m_mod(x.v1, y)
        }
    };
}

/** dst = x - y * floor(x/y) */
o_inline m_vec2 vec2_mod_v(m_vec2 x, m_vec2 y)
{
    return (m_vec2){
        {
            m_mod(x.v0, y.v0),
            m_mod(x.v1, y.v1)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_vec2 vec2_min(m_vec2 x, float y)
{
    return (m_vec2){
        {
            m_min(x.v0, y),
            m_min(x.v1, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_vec2 vec2_min_v(m_vec2 x, m_vec2 y)
{
    return (m_vec2){
        {
            m_min(x.v0, y.v0),
            m_min(x.v1, y.v1)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_vec2 vec2_max(m_vec2 x, float y)
{
    return (m_vec2){
        {
            m_max(x.v0, y),
            m_max(x.v1, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_vec2 vec2_max_v(m_vec2 x, m_vec2 y)
{
    return (m_vec2){
        {
            m_max(x.v0, y.v0),
            m_max(x.v1, y.v1)
        }
    };
}


/** returns the index of the min v value */
o_inline int vec2_min_index(m_vec2 vec)
{
    return vec.v0 <= vec.v1 ? 0 : 1;
}


/** returns the index of the max v value */
o_inline int vec2_max_index(m_vec2 vec)
{
    return vec.v0 >= vec.v1 ? 0 : 1;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_vec2 vec2_clamp(m_vec2 x, float min, float max)
{
    return (m_vec2){
        {
            m_clamp(x.v0, min, max),
            m_clamp(x.v1, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_vec2 vec2_clamp_v(m_vec2 x, m_vec2 min, m_vec2 max)
{
    return (m_vec2){
        {
            m_clamp(x.v0, min.v0, max.v0),
            m_clamp(x.v1, min.v1, max.v1)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_vec2 vec2_mix(m_vec2 a, m_vec2 b, float t)
{
    return (m_vec2){
        {
            m_mix(a.v0, b.v0, t),
            m_mix(a.v1, b.v1, t)
        }
    };
}

/** dst = a * (1-t) + b * t */
o_inline m_vec2 vec2_mix_v(m_vec2 a, m_vec2 b, m_vec2 t)
{
    return (m_vec2){
        {
            m_mix(a.v0, b.v0, t.v0),
            m_mix(a.v1, b.v1, t.v1)
        }
    };
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline float vec2_sum(m_vec2 v)
{
    return v.v0 + v.v1;
}


/** returns a dot b */
o_inline float vec2_dot(m_vec2 a, m_vec2 b)
{
    return a.v0 * b.v0 + a.v1 * b.v1;
}


/** returns ||v||_2 */
o_inline float vec2_norm(m_vec2 v)
{
    float dot = v.v0 * v.v0 + v.v1 * v.v1;
    return m_sqrt(dot);
}


/** returns ||v||_p */
o_inline float vec2_norm_p(m_vec2 v, float p)
{
    float vpow = m_pow(v.v0, p) + m_pow(v.v1, p);
    return m_pow(vpow, 1.0f / p);
}


/** returns ||v||_1 */
o_inline float vec2_norm_1(m_vec2 v)
{
    return m_abs(v.v0) + m_abs(v.v1);
}


/** returns ||v||_inf */
o_inline float vec2_norm_inf(m_vec2 v)
{
    return m_max(m_abs(v.v0), m_abs(v.v1));
}


/** dst = v / norm(v) */
o_inline m_vec2 vec2_normalize_unsafe(m_vec2 v)
{
    float dot = v.v0 * v.v0 + v.v1 * v.v1;
    float norm = m_sqrt(dot);
    return (m_vec2){
        {
            v.v0 / norm,
            v.v1 / norm
        }
    };
}


/** dst = v / (norm(v) > 0 ? norm(v) : 1) */
o_inline m_vec2 vec2_normalize(m_vec2 v)
{
    float dot = v.v0 * v.v0 + v.v1 * v.v1;
    float norm = m_sqrt(dot);
    norm = norm > 0 ? norm : 1.0f;
    return (m_vec2){
        {
            v.v0 / norm,
            v.v1 / norm
        }
    };
}


/** returns length of a vector, see vecn_norm. Just here to match glsl */
o_inline float vec2_length(m_vec2 v)
{
    float dot = v.v0 * v.v0 + v.v1 * v.v1;
    return m_sqrt(dot);
}


/** returns norm(b-a) */
o_inline float vec2_distance(m_vec2 a, m_vec2 b)
{
    m_vec2 v = (m_vec2){
        {
            b.v0 - a.v0,
            b.v1 - a.v1
        }
    };
    float dot = v.v0 * v.v0 + v.v1 * v.v1;
    return m_sqrt(dot);
}


/** returns dot(b-a) */
o_inline float vec2_sqr_distance(m_vec2 a, m_vec2 b)
{
    m_vec2 v = (m_vec2){
        {
            b.v0 - a.v0,
            b.v1 - a.v1
        }
    };
    return v.v0 * v.v0 + v.v1 * v.v1;
}


#endif //M_VEC_VEC2_H
