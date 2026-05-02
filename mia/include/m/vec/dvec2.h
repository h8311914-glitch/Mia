#ifndef M_VEC_DVEC2_H
#define M_VEC_DVEC2_H

/**
 * @file vec/dvec2.h
 *
 * functions working with dvec2
 */


#include "m/sca/dbl.h"
#include "m/m_types/dbl.h"

/** dst = v0, v1, ... */
o_inline m_dvec2 dvec2_new(double v0, double v1)
{
    return (m_dvec2){{v0, v1}};
}

/** dst = (m_dvec2) v */
o_inline m_dvec2 dvec2_cast_float(const float *cast)
{
    return (m_dvec2){{cast[0], cast[1]}};
}

/** dst = (m_dvec2) v */
o_inline m_dvec2 dvec2_cast_int(const int *cast)
{
    return (m_dvec2){{cast[0], cast[1]}};
}

/** dst = (m_dvec2) v */
o_inline m_dvec2 dvec2_cast_byte(const obyte *cast)
{
    return (m_dvec2){{cast[0], cast[1]}};
}

/** dst = v / 255 */
o_inline m_dvec2 dvec2_cast_byte_1(const obyte *cast)
{
    return (m_dvec2){{cast[0] / 255.0, cast[1] / 255.0}};
}


/** a == b */
o_inline bool dvec2_equals(m_dvec2 a, double b)
{
    return a.v0 == b
           && a.v1 == b;
}

/** a == b */
o_inline bool dvec2_equals_v(m_dvec2 a, m_dvec2 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1;
}

/** a ~= b */
o_inline bool dvec2_equals_eps(m_dvec2 a, double b, double eps)
{
    return md_equals_eps(a.v0, b, eps)
           && md_equals_eps(a.v1, b, eps);
}

/** a ~= b */
o_inline bool dvec2_equals_eps_v(m_dvec2 a, m_dvec2 b, double eps)
{
    return md_equals_eps(a.v0, b.v0, eps)
           && md_equals_eps(a.v1, b.v1, eps);
}


/** dst = s */
o_inline m_dvec2 dvec2_set(double s)
{
    return (m_dvec2){{s, s}};
}

/** dst = unit_x */
o_inline m_dvec2 dvec2_unit_x(void)
{
    return (m_dvec2){{1, 0}};
}


/** dst = unit_y */
o_inline m_dvec2 dvec2_unit_y(void)
{
    return (m_dvec2){{0, 1}};
}

/** dst = -v */
o_inline m_dvec2 dvec2_neg(m_dvec2 v)
{
    return (m_dvec2){{-v.v0, -v.v1}};
}


/** dst = a + b */
o_inline m_dvec2 dvec2_add_v(m_dvec2 a, m_dvec2 b)
{
    return (m_dvec2){
        {
            a.v0 + b.v0,
            a.v1 + b.v1
        }
    };
}


/** dst = a + b */
o_inline m_dvec2 dvec2_add(m_dvec2 a, double b)
{
    return (m_dvec2){
        {
            a.v0 + b,
            a.v1 + b
        }
    };
}


/** dst = a - b */
o_inline m_dvec2 dvec2_sub_v(m_dvec2 a, m_dvec2 b)
{
    return (m_dvec2){
        {
            a.v0 - b.v0,
            a.v1 - b.v1
        }
    };
}


/** dst = a - b */
o_inline m_dvec2 dvec2_sub(m_dvec2 a, double b)
{
    return (m_dvec2){
        {
            a.v0 - b,
            a.v1 - b
        }
    };
}


/** dst = a * b */
o_inline m_dvec2 dvec2_scale_v(m_dvec2 a, m_dvec2 b)
{
    return (m_dvec2){
        {
            a.v0 * b.v0,
            a.v1 * b.v1
        }
    };
}


/** dst = a * b */
o_inline m_dvec2 dvec2_scale(m_dvec2 a, double b)
{
    return (m_dvec2){
        {
            a.v0 * b,
            a.v1 * b
        }
    };
}


/** dst = a / b */
o_inline m_dvec2 dvec2_div_v(m_dvec2 a, m_dvec2 b)
{
    return (m_dvec2){
        {
            a.v0 / b.v0,
            a.v1 / b.v1
        }
    };
}


/** dst = a / b */
o_inline m_dvec2 dvec2_div(m_dvec2 a, double b)
{
    return (m_dvec2){
        {
            a.v0 / b,
            a.v1 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_dvec2 dvec2_add_scaled_v(m_dvec2 a, m_dvec2 b, m_dvec2 c)
{
    return (m_dvec2){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1
        }
    };
}

/** dst = a + b * c */
o_inline m_dvec2 dvec2_add_scaled(m_dvec2 a, m_dvec2 b, double c)
{
    return (m_dvec2){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c
        }
    };
}


/** dst = a * M_PI / 180 */
o_inline m_dvec2 dvec2_radians(m_dvec2 deg)
{
    return (m_dvec2){
        {
            md_radians(deg.v0),
            md_radians(deg.v1)
        }
    };
}


/** dst = a * 180 / M_PI */
o_inline m_dvec2 dvec2_degrees(m_dvec2 rad)
{
    return (m_dvec2){
        {
            md_degrees(rad.v0),
            md_degrees(rad.v1)
        }
    };
}


/** dst = abs(x) */
o_inline m_dvec2 dvec2_abs(m_dvec2 x)
{
    return (m_dvec2){
        {
            md_abs(x.v0),
            md_abs(x.v1)
        }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_dvec2 dvec2_sign(m_dvec2 x)
{
    return (m_dvec2){
        {
            md_sign(x.v0),
            md_sign(x.v1)
        }
    };
}


/** dst = round(x) */
o_inline m_dvec2 dvec2_round(m_dvec2 x)
{
    return (m_dvec2){
        {
            md_round(x.v0),
            md_round(x.v1)
        }
    };
}


/** dst = floor(x) */
o_inline m_dvec2 dvec2_floor(m_dvec2 x)
{
    return (m_dvec2){
        {
            md_floor(x.v0),
            md_floor(x.v1)
        }
    };
}


/** dst = ceil(x) */
o_inline m_dvec2 dvec2_ceil(m_dvec2 x)
{
    return (m_dvec2){
        {
            md_ceil(x.v0),
            md_ceil(x.v1)
        }
    };
}


/** dst = x - floor(x) */
o_inline m_dvec2 dvec2_fract(m_dvec2 x)
{
    return (m_dvec2){
        {
            md_fract(x.v0),
            md_fract(x.v1)
        }
    };
}


/** dst = x - y * floor(x/y) */
o_inline m_dvec2 dvec2_mod(m_dvec2 x, double y)
{
    return (m_dvec2){
        {
            md_mod(x.v0, y),
            md_mod(x.v1, y)
        }
    };
}

/** dst = x - y * floor(x/y) */
o_inline m_dvec2 dvec2_mod_v(m_dvec2 x, m_dvec2 y)
{
    return (m_dvec2){
        {
            md_mod(x.v0, y.v0),
            md_mod(x.v1, y.v1)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_dvec2 dvec2_min(m_dvec2 x, double y)
{
    return (m_dvec2){
        {
            md_min(x.v0, y),
            md_min(x.v1, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_dvec2 dvec2_min_v(m_dvec2 x, m_dvec2 y)
{
    return (m_dvec2){
        {
            md_min(x.v0, y.v0),
            md_min(x.v1, y.v1)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_dvec2 dvec2_max(m_dvec2 x, double y)
{
    return (m_dvec2){
        {
            md_max(x.v0, y),
            md_max(x.v1, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_dvec2 dvec2_max_v(m_dvec2 x, m_dvec2 y)
{
    return (m_dvec2){
        {
            md_max(x.v0, y.v0),
            md_max(x.v1, y.v1)
        }
    };
}


/** returns the index of the min v value */
o_inline int dvec2_min_index(m_dvec2 vec)
{
    return vec.v0 <= vec.v1 ? 0 : 1;
}


/** returns the index of the max v value */
o_inline int dvec2_max_index(m_dvec2 vec)
{
    return vec.v0 >= vec.v1 ? 0 : 1;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_dvec2 dvec2_clamp(m_dvec2 x, double min, double max)
{
    return (m_dvec2){
        {
            md_clamp(x.v0, min, max),
            md_clamp(x.v1, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_dvec2 dvec2_clamp_v(m_dvec2 x, m_dvec2 min, m_dvec2 max)
{
    return (m_dvec2){
        {
            md_clamp(x.v0, min.v0, max.v0),
            md_clamp(x.v1, min.v1, max.v1)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_dvec2 dvec2_mix(m_dvec2 a, m_dvec2 b, double t)
{
    return (m_dvec2){
        {
            md_mix(a.v0, b.v0, t),
            md_mix(a.v1, b.v1, t)
        }
    };
}

/** dst = a * (1-t) + b * t */
o_inline m_dvec2 dvec2_mix_v(m_dvec2 a, m_dvec2 b, m_dvec2 t)
{
    return (m_dvec2){
        {
            md_mix(a.v0, b.v0, t.v0),
            md_mix(a.v1, b.v1, t.v1)
        }
    };
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline double dvec2_sum(m_dvec2 v)
{
    return v.v0 + v.v1;
}


/** returns a dot b */
o_inline double dvec2_dot(m_dvec2 a, m_dvec2 b)
{
    return a.v0 * b.v0 + a.v1 * b.v1;
}


/** returns ||v||_2 */
o_inline double dvec2_norm(m_dvec2 v)
{
    double dot = v.v0 * v.v0 + v.v1 * v.v1;
    return md_sqrt(dot);
}


/** returns ||v||_p */
o_inline double dvec2_norm_p(m_dvec2 v, double p)
{
    double vpow = md_pow(v.v0, p) + md_pow(v.v1, p);
    return md_pow(vpow, 1.0 / p);
}


/** returns ||v||_1 */
o_inline double dvec2_norm_1(m_dvec2 v)
{
    return md_abs(v.v0) + md_abs(v.v1);
}


/** returns ||v||_inf */
o_inline double dvec2_norm_inf(m_dvec2 v)
{
    return md_max(md_abs(v.v0), md_abs(v.v1));
}


/** dst = v / norm(v) */
o_inline m_dvec2 dvec2_normalize_unsafe(m_dvec2 v)
{
    double dot = v.v0 * v.v0 + v.v1 * v.v1;
    double norm = md_sqrt(dot);
    return (m_dvec2){
        {
            v.v0 / norm,
            v.v1 / norm
        }
    };
}


/** dst = v / (norm(v) > 0 ? norm(v) : 1) */
o_inline m_dvec2 dvec2_normalize(m_dvec2 v)
{
    double dot = v.v0 * v.v0 + v.v1 * v.v1;
    double norm = md_sqrt(dot);
    norm = norm > 0 ? norm : 1.0;
    return (m_dvec2){
        {
            v.v0 / norm,
            v.v1 / norm
        }
    };
}


/** returns length of a vector, see vecn_norm. Just here to match glsl */
o_inline double dvec2_length(m_dvec2 v)
{
    double dot = v.v0 * v.v0 + v.v1 * v.v1;
    return md_sqrt(dot);
}


/** returns norm(b-a) */
o_inline double dvec2_distance(m_dvec2 a, m_dvec2 b)
{
    m_dvec2 v = (m_dvec2){
        {
            b.v0 - a.v0,
            b.v1 - a.v1
        }
    };
    double dot = v.v0 * v.v0 + v.v1 * v.v1;
    return md_sqrt(dot);
}


/** returns dot(b-a) */
o_inline double dvec2_sqr_distance(m_dvec2 a, m_dvec2 b)
{
    m_dvec2 v = (m_dvec2){
        {
            b.v0 - a.v0,
            b.v1 - a.v1
        }
    };
    return v.v0 * v.v0 + v.v1 * v.v1;
}


#endif //M_VEC_DVEC2_H
