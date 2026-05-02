#ifndef M_VEC_VEC4_H
#define M_VEC_VEC4_H

/**
 * @file vec/vec4.h
 *
 * functions working with vec4
 */


#include "m/sca/flt.h"
#include "m/m_types/flt.h"

/** dst = v0, v1, ... */
o_inline m_vec4 vec4_new(float v0, float v1, float v2, float v3)
{
    return (m_vec4){{v0, v1, v2, v3}};
}

/** dst = (m_vec4) v */
o_inline m_vec4 vec4_cast_double(const double *cast)
{
    return (m_vec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = (m_vec4) v */
o_inline m_vec4 vec4_cast_int(const int *cast)
{
    return (m_vec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = (m_vec4) v */
o_inline m_vec4 vec4_cast_byte(const obyte *cast)
{
    return (m_vec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = v / 255 */
o_inline m_vec4 vec4_cast_byte_1(const obyte *cast)
{
    return (m_vec4){{cast[0] / 255.0f, cast[1] / 255.0f, cast[2] / 255.0f, cast[3] / 255.0f}};
}


/** a == b */
o_inline bool vec4_equals(m_vec4 a, float b)
{
    return a.v0 == b
           && a.v1 == b
           && a.v2 == b
           && a.v3 == b;
}

/** a == b */
o_inline bool vec4_equals_v(m_vec4 a, m_vec4 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1
           && a.v2 == b.v2
           && a.v3 == b.v3;
}

/** a ~= b */
o_inline bool vec4_equals_eps(m_vec4 a, float b, float eps)
{
    return m_equals_eps(a.v0, b, eps)
           && m_equals_eps(a.v1, b, eps)
           && m_equals_eps(a.v2, b, eps)
           && m_equals_eps(a.v3, b, eps);
}

/** a ~= b */
o_inline bool vec4_equals_eps_v(m_vec4 a, m_vec4 b, float eps)
{
    return m_equals_eps(a.v0, b.v0, eps)
           && m_equals_eps(a.v1, b.v1, eps)
           && m_equals_eps(a.v2, b.v2, eps)
           && m_equals_eps(a.v3, b.v3, eps);
}


/** dst = s */
o_inline m_vec4 vec4_set(float s)
{
    return (m_vec4){{s, s, s, s}};
}

/** dst = unit_x */
o_inline m_vec4 vec4_unit_x(void)
{
    return (m_vec4){{1, 0, 0, 0}};
}


/** dst = unit_y */
o_inline m_vec4 vec4_unit_y(void)
{
    return (m_vec4){{0, 1, 0, 0}};
}


/** dst = unit_z */
o_inline m_vec4 vec4_unit_z(void)
{
    return (m_vec4){{0, 0, 1, 0}};
}


/** dst = unit_w */
o_inline m_vec4 vec4_unit_w(void)
{
    return (m_vec4){{0, 0, 0, 1}};
}


/** dst = -v */
o_inline m_vec4 vec4_neg(m_vec4 v)
{
    return (m_vec4){{-v.v0, -v.v1, -v.v2, -v.v3}};
}


/** dst = a + b */
o_inline m_vec4 vec4_add_v(m_vec4 a, m_vec4 b)
{
    return (m_vec4){
        {
            a.v0 + b.v0,
            a.v1 + b.v1,
            a.v2 + b.v2,
            a.v3 + b.v3
        }
    };
}


/** dst = a + b */
o_inline m_vec4 vec4_add(m_vec4 a, float b)
{
    return (m_vec4){
        {
            a.v0 + b,
            a.v1 + b,
            a.v2 + b,
            a.v3 + b
        }
    };
}


/** dst = a - b */
o_inline m_vec4 vec4_sub_v(m_vec4 a, m_vec4 b)
{
    return (m_vec4){
        {
            a.v0 - b.v0,
            a.v1 - b.v1,
            a.v2 - b.v2,
            a.v3 - b.v3
        }
    };
}


/** dst = a - b */
o_inline m_vec4 vec4_sub(m_vec4 a, float b)
{
    return (m_vec4){
        {
            a.v0 - b,
            a.v1 - b,
            a.v2 - b,
            a.v3 - b
        }
    };
}


/** dst = a * b */
o_inline m_vec4 vec4_scale_v(m_vec4 a, m_vec4 b)
{
    return (m_vec4){
        {
            a.v0 * b.v0,
            a.v1 * b.v1,
            a.v2 * b.v2,
            a.v3 * b.v3
        }
    };
}


/** dst = a * b */
o_inline m_vec4 vec4_scale(m_vec4 a, float b)
{
    return (m_vec4){
        {
            a.v0 * b,
            a.v1 * b,
            a.v2 * b,
            a.v3 * b
        }
    };
}


/** dst = a / b */
o_inline m_vec4 vec4_div_v(m_vec4 a, m_vec4 b)
{
    return (m_vec4){
        {
            a.v0 / b.v0,
            a.v1 / b.v1,
            a.v2 / b.v2,
            a.v3 / b.v3
        }
    };
}


/** dst = a / b */
o_inline m_vec4 vec4_div(m_vec4 a, float b)
{
    return (m_vec4){
        {
            a.v0 / b,
            a.v1 / b,
            a.v2 / b,
            a.v3 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_vec4 vec4_add_scaled_v(m_vec4 a, m_vec4 b, m_vec4 c)
{
    return (m_vec4){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1,
            a.v2 + b.v2 * c.v2,
            a.v3 + b.v3 * c.v3
        }
    };
}

/** dst = a + b * c */
o_inline m_vec4 vec4_add_scaled(m_vec4 a, m_vec4 b, float c)
{
    return (m_vec4){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c,
            a.v2 + b.v2 * c,
            a.v3 + b.v3 * c
        }
    };
}


/** dst = a * M_PI / 180 */
o_inline m_vec4 vec4_radians(m_vec4 deg)
{
    return (m_vec4){
        {
            m_radians(deg.v0),
            m_radians(deg.v1),
            m_radians(deg.v2),
            m_radians(deg.v3)
        }
    };
}


/** dst = a * 180 / M_PI */
o_inline m_vec4 vec4_degrees(m_vec4 rad)
{
    return (m_vec4){
        {
            m_degrees(rad.v0),
            m_degrees(rad.v1),
            m_degrees(rad.v2),
            m_degrees(rad.v3)
        }
    };
}


/** dst = abs(x) */
o_inline m_vec4 vec4_abs(m_vec4 x)
{
    return (m_vec4){
        {
            m_abs(x.v0),
            m_abs(x.v1),
            m_abs(x.v2),
            m_abs(x.v3)
        }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_vec4 vec4_sign(m_vec4 x)
{
    return (m_vec4){
        {
            m_sign(x.v0),
            m_sign(x.v1),
            m_sign(x.v2),
            m_sign(x.v3)
        }
    };
}


/** dst = round(x) */
o_inline m_vec4 vec4_round(m_vec4 x)
{
    return (m_vec4){
        {
            m_round(x.v0),
            m_round(x.v1),
            m_round(x.v2),
            m_round(x.v3)
        }
    };
}


/** dst = floor(x) */
o_inline m_vec4 vec4_floor(m_vec4 x)
{
    return (m_vec4){
        {
            m_floor(x.v0),
            m_floor(x.v1),
            m_floor(x.v2),
            m_floor(x.v3)
        }
    };
}


/** dst = ceil(x) */
o_inline m_vec4 vec4_ceil(m_vec4 x)
{
    return (m_vec4){
        {
            m_ceil(x.v0),
            m_ceil(x.v1),
            m_ceil(x.v2),
            m_ceil(x.v3)
        }
    };
}


/** dst = x - floor(x) */
o_inline m_vec4 vec4_fract(m_vec4 x)
{
    return (m_vec4){
        {
            m_fract(x.v0),
            m_fract(x.v1),
            m_fract(x.v2),
            m_fract(x.v3)
        }
    };
}


/** dst = x - y * floor(x/y) */
o_inline m_vec4 vec4_mod(m_vec4 x, float y)
{
    return (m_vec4){
        {
            m_mod(x.v0, y),
            m_mod(x.v1, y),
            m_mod(x.v2, y),
            m_mod(x.v3, y)
        }
    };
}

/** dst = x - y * floor(x/y) */
o_inline m_vec4 vec4_mod_v(m_vec4 x, m_vec4 y)
{
    return (m_vec4){
        {
            m_mod(x.v0, y.v0),
            m_mod(x.v1, y.v1),
            m_mod(x.v2, y.v2),
            m_mod(x.v3, y.v3)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_vec4 vec4_min(m_vec4 x, float y)
{
    return (m_vec4){
        {
            m_min(x.v0, y),
            m_min(x.v1, y),
            m_min(x.v2, y),
            m_min(x.v3, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_vec4 vec4_min_v(m_vec4 x, m_vec4 y)
{
    return (m_vec4){
        {
            m_min(x.v0, y.v0),
            m_min(x.v1, y.v1),
            m_min(x.v2, y.v2),
            m_min(x.v3, y.v3)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_vec4 vec4_max(m_vec4 x, float y)
{
    return (m_vec4){
        {
            m_max(x.v0, y),
            m_max(x.v1, y),
            m_max(x.v2, y),
            m_max(x.v3, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_vec4 vec4_max_v(m_vec4 x, m_vec4 y)
{
    return (m_vec4){
        {
            m_max(x.v0, y.v0),
            m_max(x.v1, y.v1),
            m_max(x.v2, y.v2),
            m_max(x.v3, y.v3)
        }
    };
}


/** returns the index of the min v value */
o_inline int vec4_min_index(m_vec4 vec)
{
    int idx = vec.v0 <= vec.v1 ? 0 : 1;
    idx = vec.v[idx] <= vec.v2 ? idx : 2;
    idx = vec.v[idx] <= vec.v3 ? idx : 3;
    return idx;
}


/** returns the index of the max v value */
o_inline int vec4_max_index(m_vec4 vec)
{
    int idx = vec.v0 >= vec.v1 ? 0 : 1;
    idx = vec.v[idx] >= vec.v2 ? idx : 2;
    idx = vec.v[idx] >= vec.v3 ? idx : 3;
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_vec4 vec4_clamp(m_vec4 x, float min, float max)
{
    return (m_vec4){
        {
            m_clamp(x.v0, min, max),
            m_clamp(x.v1, min, max),
            m_clamp(x.v2, min, max),
            m_clamp(x.v3, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_vec4 vec4_clamp_v(m_vec4 x, m_vec4 min, m_vec4 max)
{
    return (m_vec4){
        {
            m_clamp(x.v0, min.v0, max.v0),
            m_clamp(x.v1, min.v1, max.v1),
            m_clamp(x.v2, min.v2, max.v2),
            m_clamp(x.v3, min.v3, max.v3)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_vec4 vec4_mix(m_vec4 a, m_vec4 b, float t)
{
    return (m_vec4){
        {
            m_mix(a.v0, b.v0, t),
            m_mix(a.v1, b.v1, t),
            m_mix(a.v2, b.v2, t),
            m_mix(a.v3, b.v3, t)
        }
    };
}

/** dst = a * (1-t) + b * t */
o_inline m_vec4 vec4_mix_v(m_vec4 a, m_vec4 b, m_vec4 t)
{
    return (m_vec4){
        {
            m_mix(a.v0, b.v0, t.v0),
            m_mix(a.v1, b.v1, t.v1),
            m_mix(a.v2, b.v2, t.v2),
            m_mix(a.v3, b.v3, t.v3)
        }
    };
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline float vec4_sum(m_vec4 v)
{
    return v.v0 + v.v1 + v.v2 + v.v3;
}


/** returns a dot b */
o_inline float vec4_dot(m_vec4 a, m_vec4 b)
{
    return a.v0 * b.v0
           + a.v1 * b.v1
           + a.v2 * b.v2
           + a.v3 * b.v3;
}


/** dst = a x b , dst.w... = 0 */
o_inline m_vec4 vec4_cross(m_vec4 a, m_vec4 b)
{
    return (m_vec4){
        {
            a.v1 * b.v2 - a.v2 * b.v1,
            a.v2 * b.v0 - a.v0 * b.v2,
            a.v0 * b.v1 - a.v1 * b.v0,
            0
        }
    };
}


/** returns ||v||_4 */
o_inline float vec4_norm(m_vec4 v)
{
    float dot = v.v0 * v.v0
                + v.v1 * v.v1
                + v.v2 * v.v2
                + v.v3 * v.v3;
    return m_sqrt(dot);
}


/** returns ||v||_p */
o_inline float vec4_norm_p(m_vec4 v, float p)
{
    float vpow = m_pow(v.v0, p)
                 + m_pow(v.v1, p)
                 + m_pow(v.v2, p)
                 + m_pow(v.v3, p);
    return m_pow(vpow, 1.0f / p);
}


/** returns ||v||_1 */
o_inline float vec4_norm_1(m_vec4 v)
{
    return m_abs(v.v0)
           + m_abs(v.v1)
           + m_abs(v.v2)
           + m_abs(v.v3);
}


/** returns ||v||_inf */
o_inline float vec4_norm_inf(m_vec4 v)
{
    float inf = m_max(m_abs(v.v0), m_abs(v.v1));
    inf = m_max(inf, m_abs(v.v2));
    inf = m_max(inf, m_abs(v.v3));
    return inf;
}


/** dst = v / norm(v) */
o_inline m_vec4 vec4_normalize_unsafe(m_vec4 v)
{
    float dot = v.v0 * v.v0
                + v.v1 * v.v1
                + v.v2 * v.v2
                + v.v3 * v.v3;
    float norm = m_sqrt(dot);
    return (m_vec4){
        {
            v.v0 / norm,
            v.v1 / norm,
            v.v2 / norm,
            v.v3 / norm
        }
    };
}


/** dst = v / (norm(v) > 0 ? norm(v) : 1) */
o_inline m_vec4 vec4_normalize(m_vec4 v)
{
    float dot = v.v0 * v.v0
                + v.v1 * v.v1
                + v.v2 * v.v2
                + v.v3 * v.v3;
    float norm = m_sqrt(dot);
    norm = norm > 0 ? norm : 1;
    return (m_vec4){
        {
            v.v0 / norm,
            v.v1 / norm,
            v.v2 / norm,
            v.v3 / norm
        }
    };
}

/** dst = normalize(cross(a, b)) */
o_inline m_vec4 vec4_cross_normalized(m_vec4 a, m_vec4 b)
{
    m_vec4 v = (m_vec4){
        {
            a.v1 * b.v2 - a.v2 * b.v1,
            a.v2 * b.v0 - a.v0 * b.v2,
            a.v0 * b.v1 - a.v1 * b.v0,
            0
        }
    };
    float dot = v.v0 * v.v0
                + v.v1 * v.v1
                + v.v2 * v.v2
                + v.v3 * v.v3;
    float norm = m_sqrt(dot);
    norm = norm > 0 ? norm : 1;
    return (m_vec4){
        {
            v.v0 / norm,
            v.v1 / norm,
            v.v2 / norm,
            v.v3 / norm
        }
    };
}


/** returns length of a vector, see vecn_norm. Just here to match glsl */
o_inline float vec4_length(m_vec4 v)
{
    float dot = v.v0 * v.v0
                + v.v1 * v.v1
                + v.v2 * v.v2
                + v.v3 * v.v3;
    return m_sqrt(dot);
}


/** returns norm(b-a) */
o_inline float vec4_distance(m_vec4 a, m_vec4 b)
{
    m_vec4 v = (m_vec4){
        {
            b.v0 - a.v0,
            b.v1 - a.v1,
            b.v2 - a.v2,
            b.v3 - a.v3
        }
    };
    float dot = v.v0 * v.v0
                + v.v1 * v.v1
                + v.v2 * v.v2
                + v.v3 * v.v3;
    return m_sqrt(dot);
}


/** returns dot(b-a) */
o_inline float vec4_sqr_distance(m_vec4 a, m_vec4 b)
{
    m_vec4 v = (m_vec4){
        {
            b.v0 - a.v0,
            b.v1 - a.v1,
            b.v2 - a.v2,
            b.v3 - a.v3
        }
    };
    return v.v0 * v.v0
           + v.v1 * v.v1
           + v.v2 * v.v2
           + v.v3 * v.v3;
}


#endif //M_VEC_VEC4_H
