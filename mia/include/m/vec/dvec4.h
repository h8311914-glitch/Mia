#ifndef M_VEC_DVEC4_H
#define M_VEC_DVEC4_H

/**
 * @file dvec/dvec4.h
 *
 * functions working with dvec4
 */


#include "m/sca/dbl.h"
#include "m/m_types/dbl.h"

/** dst = v0, v1, ... */
o_inline m_dvec4 dvec4_new(double v0, double v1, double v2, double v3)
{
    return (m_dvec4){{v0, v1, v2, v3}};
}

/** dst = (m_dvec4) v */
o_inline m_dvec4 dvec4_cast_float(const float *cast)
{
    return (m_dvec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = (m_dvec4) v */
o_inline m_dvec4 dvec4_cast_int(const int *cast)
{
    return (m_dvec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = (m_dvec4) v */
o_inline m_dvec4 dvec4_cast_byte(const obyte *cast)
{
    return (m_dvec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = v / 255 */
o_inline m_dvec4 dvec4_cast_byte_1(const obyte *cast)
{
    return (m_dvec4){{cast[0] / 255.0, cast[1] / 255.0, cast[2] / 255.0, cast[3] / 255.0}};
}


/** a == b */
o_inline bool dvec4_equals(m_dvec4 a, double b)
{
    return a.v0 == b
           && a.v1 == b
           && a.v2 == b
           && a.v3 == b;
}

/** a == b */
o_inline bool dvec4_equals_v(m_dvec4 a, m_dvec4 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1
           && a.v2 == b.v2
           && a.v3 == b.v3;
}

/** a ~= b */
o_inline bool dvec4_equals_eps(m_dvec4 a, double b, double eps)
{
    return md_equals_eps(a.v0, b, eps)
           && md_equals_eps(a.v1, b, eps)
           && md_equals_eps(a.v2, b, eps)
           && md_equals_eps(a.v3, b, eps);
}

/** a ~= b */
o_inline bool dvec4_equals_eps_v(m_dvec4 a, m_dvec4 b, double eps)
{
    return md_equals_eps(a.v0, b.v0, eps)
           && md_equals_eps(a.v1, b.v1, eps)
           && md_equals_eps(a.v2, b.v2, eps)
           && md_equals_eps(a.v3, b.v3, eps);
}


/** dst = s */
o_inline m_dvec4 dvec4_set(double s)
{
    return (m_dvec4){{s, s, s, s}};
}

/** dst = unit_x */
o_inline m_dvec4 dvec4_unit_x(void)
{
    return (m_dvec4){{1, 0, 0, 0}};
}


/** dst = unit_y */
o_inline m_dvec4 dvec4_unit_y(void)
{
    return (m_dvec4){{0, 1, 0, 0}};
}


/** dst = unit_z */
o_inline m_dvec4 dvec4_unit_z(void)
{
    return (m_dvec4){{0, 0, 1, 0}};
}


/** dst = unit_w */
o_inline m_dvec4 dvec4_unit_w(void)
{
    return (m_dvec4){{0, 0, 0, 1}};
}


/** dst = -v */
o_inline m_dvec4 dvec4_neg(m_dvec4 v)
{
    return (m_dvec4){{-v.v0, -v.v1, -v.v2, -v.v3}};
}


/** dst = a + b */
o_inline m_dvec4 dvec4_add_v(m_dvec4 a, m_dvec4 b)
{
    return (m_dvec4){
        {
            a.v0 + b.v0,
            a.v1 + b.v1,
            a.v2 + b.v2,
            a.v3 + b.v3
        }
    };
}


/** dst = a + b */
o_inline m_dvec4 dvec4_add(m_dvec4 a, double b)
{
    return (m_dvec4){
        {
            a.v0 + b,
            a.v1 + b,
            a.v2 + b,
            a.v3 + b
        }
    };
}


/** dst = a - b */
o_inline m_dvec4 dvec4_sub_v(m_dvec4 a, m_dvec4 b)
{
    return (m_dvec4){
        {
            a.v0 - b.v0,
            a.v1 - b.v1,
            a.v2 - b.v2,
            a.v3 - b.v3
        }
    };
}


/** dst = a - b */
o_inline m_dvec4 dvec4_sub(m_dvec4 a, double b)
{
    return (m_dvec4){
        {
            a.v0 - b,
            a.v1 - b,
            a.v2 - b,
            a.v3 - b
        }
    };
}


/** dst = a * b */
o_inline m_dvec4 dvec4_scale_v(m_dvec4 a, m_dvec4 b)
{
    return (m_dvec4){
        {
            a.v0 * b.v0,
            a.v1 * b.v1,
            a.v2 * b.v2,
            a.v3 * b.v3
        }
    };
}


/** dst = a * b */
o_inline m_dvec4 dvec4_scale(m_dvec4 a, double b)
{
    return (m_dvec4){
        {
            a.v0 * b,
            a.v1 * b,
            a.v2 * b,
            a.v3 * b
        }
    };
}


/** dst = a / b */
o_inline m_dvec4 dvec4_div_v(m_dvec4 a, m_dvec4 b)
{
    return (m_dvec4){
        {
            a.v0 / b.v0,
            a.v1 / b.v1,
            a.v2 / b.v2,
            a.v3 / b.v3
        }
    };
}


/** dst = a / b */
o_inline m_dvec4 dvec4_div(m_dvec4 a, double b)
{
    return (m_dvec4){
        {
            a.v0 / b,
            a.v1 / b,
            a.v2 / b,
            a.v3 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_dvec4 dvec4_add_scaled_v(m_dvec4 a, m_dvec4 b, m_dvec4 c)
{
    return (m_dvec4){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1,
            a.v2 + b.v2 * c.v2,
            a.v3 + b.v3 * c.v3
        }
    };
}

/** dst = a + b * c */
o_inline m_dvec4 dvec4_add_scaled(m_dvec4 a, m_dvec4 b, double c)
{
    return (m_dvec4){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c,
            a.v2 + b.v2 * c,
            a.v3 + b.v3 * c
        }
    };
}


/** dst = a * M_PI / 180 */
o_inline m_dvec4 dvec4_radians(m_dvec4 deg)
{
    return (m_dvec4){
            {
                md_radians(deg.v0),
                md_radians(deg.v1),
                md_radians(deg.v2),
                md_radians(deg.v3)
            }
    };
}


/** dst = a * 180 / M_PI */
o_inline m_dvec4 dvec4_degrees(m_dvec4 rad)
{
    return (m_dvec4){
            {
                md_degrees(rad.v0),
                md_degrees(rad.v1),
                md_degrees(rad.v2),
                md_degrees(rad.v3)
            }
    };
}


/** dst = abs(x) */
o_inline m_dvec4 dvec4_abs(m_dvec4 x)
{
    return (m_dvec4){
            {
                md_abs(x.v0),
                md_abs(x.v1),
                md_abs(x.v2),
                md_abs(x.v3)
            }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_dvec4 dvec4_sign(m_dvec4 x)
{
    return (m_dvec4){
            {
                md_sign(x.v0),
                md_sign(x.v1),
                md_sign(x.v2),
                md_sign(x.v3)
            }
    };
}


/** dst = round(x) */
o_inline m_dvec4 dvec4_round(m_dvec4 x)
{
    return (m_dvec4){
            {
                md_round(x.v0),
                md_round(x.v1),
                md_round(x.v2),
                md_round(x.v3)
            }
    };
}


/** dst = floor(x) */
o_inline m_dvec4 dvec4_floor(m_dvec4 x)
{
    return (m_dvec4){
            {
                md_floor(x.v0),
                md_floor(x.v1),
                md_floor(x.v2),
                md_floor(x.v3)
            }
    };
}


/** dst = ceil(x) */
o_inline m_dvec4 dvec4_ceil(m_dvec4 x)
{
    return (m_dvec4){
            {
                md_ceil(x.v0),
                md_ceil(x.v1),
                md_ceil(x.v2),
                md_ceil(x.v3)
            }
    };
}


/** dst = x - floor(x) */
o_inline m_dvec4 dvec4_fract(m_dvec4 x)
{
    return (m_dvec4){
            {
                md_fract(x.v0),
                md_fract(x.v1),
                md_fract(x.v2),
                md_fract(x.v3)
            }
    };
}


/** dst = x - y * floor(x/y) */
o_inline m_dvec4 dvec4_mod(m_dvec4 x, double y)
{
    return (m_dvec4){
            {
                md_mod(x.v0, y),
                md_mod(x.v1, y),
                md_mod(x.v2, y),
                md_mod(x.v3, y)
            }
    };
}

/** dst = x - y * floor(x/y) */
o_inline m_dvec4 dvec4_mod_v(m_dvec4 x, m_dvec4 y)
{
    return (m_dvec4){
            {
                md_mod(x.v0, y.v0),
                md_mod(x.v1, y.v1),
                md_mod(x.v2, y.v2),
                md_mod(x.v3, y.v3)
            }
    };
}


/** dst = a < b ? a : b */
o_inline m_dvec4 dvec4_min(m_dvec4 x, double y)
{
    return (m_dvec4){
            {
                md_min(x.v0, y),
                md_min(x.v1, y),
                md_min(x.v2, y),
                md_min(x.v3, y)
            }
    };
}


/** dst = a < b ? a : b */
o_inline m_dvec4 dvec4_min_v(m_dvec4 x, m_dvec4 y)
{
    return (m_dvec4){
            {
                md_min(x.v0, y.v0),
                md_min(x.v1, y.v1),
                md_min(x.v2, y.v2),
                md_min(x.v3, y.v3)
            }
    };
}


/** dst = a > b ? a : b */
o_inline m_dvec4 dvec4_max(m_dvec4 x, double y)
{
    return (m_dvec4){
            {
                md_max(x.v0, y),
                md_max(x.v1, y),
                md_max(x.v2, y),
                md_max(x.v3, y)
            }
    };
}


/** dst = a > b ? a : b */
o_inline m_dvec4 dvec4_max_v(m_dvec4 x, m_dvec4 y)
{
    return (m_dvec4){
            {
                md_max(x.v0, y.v0),
                md_max(x.v1, y.v1),
                md_max(x.v2, y.v2),
                md_max(x.v3, y.v3)
            }
    };
}


/** returns the index of the min v value */
o_inline int dvec4_min_index(m_dvec4 dvec)
{
    int idx = dvec.v0 <= dvec.v1? 0 : 1;
    idx = dvec.v[idx] <= dvec.v2? idx : 2;
    idx = dvec.v[idx] <= dvec.v3? idx : 3;
    return idx;
}


/** returns the index of the max v value */
o_inline int dvec4_max_index(m_dvec4 dvec)
{
    int idx = dvec.v0 >= dvec.v1? 0 : 1;
    idx = dvec.v[idx] >= dvec.v2? idx : 2;
    idx = dvec.v[idx] >= dvec.v3? idx : 3;
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_dvec4 dvec4_clamp(m_dvec4 x, double min, double max)
{
    return (m_dvec4){
            {
                md_clamp(x.v0, min, max),
                md_clamp(x.v1, min, max),
                md_clamp(x.v2, min, max),
                md_clamp(x.v3, min, max)
            }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_dvec4 dvec4_clamp_v(m_dvec4 x, m_dvec4 min, m_dvec4 max)
{
    return (m_dvec4){
            {
                md_clamp(x.v0, min.v0, max.v0),
                md_clamp(x.v1, min.v1, max.v1),
                md_clamp(x.v2, min.v2, max.v2),
                md_clamp(x.v3, min.v3, max.v3)
            }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_dvec4 dvec4_mix(m_dvec4 a, m_dvec4 b, double t)
{
    return (m_dvec4){
            {
                md_mix(a.v0, b.v0, t),
                md_mix(a.v1, b.v1, t),
                md_mix(a.v2, b.v2, t),
                md_mix(a.v3, b.v3, t)
            }
    };
}

/** dst = a * (1-t) + b * t */
o_inline m_dvec4 dvec4_mix_v(m_dvec4 a, m_dvec4 b, m_dvec4 t)
{
    return (m_dvec4){
            {
                md_mix(a.v0, b.v0, t.v0),
                md_mix(a.v1, b.v1, t.v1),
                md_mix(a.v2, b.v2, t.v2),
                md_mix(a.v3, b.v3, t.v3)
            }
    };
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline double dvec4_sum(m_dvec4 v)
{
    return v.v0 + v.v1 + v.v2 + v.v3;
}


/** returns a dot b */
o_inline double dvec4_dot(m_dvec4 a, m_dvec4 b)
{
    return a.v0 * b.v0
            + a.v1 * b.v1
            + a.v2 * b.v2
            + a.v3 * b.v3;
}


/** dst = a x b , dst.w... = 0 */
o_inline m_dvec4 dvec4_cross(m_dvec4 a, m_dvec4 b)
{
    return (m_dvec4) {
        {
            a.v1 * b.v2 - a.v2 * b.v1,
            a.v2 * b.v0 - a.v0 * b.v2,
            a.v0 * b.v1 - a.v1 * b.v0,
            0
        }
    };    
}


/** returns ||v||_4 */
o_inline double dvec4_norm(m_dvec4 v)
{
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2
            + v.v3 * v.v3;
    return m_sqrt(dot);
}


/** returns ||v||_p */
o_inline double dvec4_norm_p(m_dvec4 v, double p)
{
    double vpow = md_pow(v.v0, p)
            + md_pow(v.v1, p)
            + md_pow(v.v2, p)
            + md_pow(v.v3, p);
    return md_pow(vpow, 1.0 / p);
}


/** returns ||v||_1 */
o_inline double dvec4_norm_1(m_dvec4 v)
{
    return md_abs(v.v0)
            + md_abs(v.v1)
            + md_abs(v.v2)
            + md_abs(v.v3);
}


/** returns ||v||_inf */
o_inline double dvec4_norm_inf(m_dvec4 v)
{
    double inf = md_max(md_abs(v.v0), md_abs(v.v1));
    inf = md_max(inf, md_abs(v.v2));
    inf = md_max(inf, md_abs(v.v3));
    return inf;
}


/** dst = v / norm(v) */
o_inline m_dvec4 dvec4_normalize_unsafe(m_dvec4 v)
{
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2
            + v.v3 * v.v3;
    double norm = md_sqrt(dot);
    return (m_dvec4) {
        {
            v.v0 / norm,
            v.v1 / norm,
            v.v2 / norm,
            v.v3 / norm
        }
    };
}


/** dst = v / (norm(v) > 0 ? norm(v) : 1) */
o_inline m_dvec4 dvec4_normalize(m_dvec4 v)
{
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2
            + v.v3 * v.v3;
    double norm = md_sqrt(dot);
    norm = norm>0? norm : 1;
    return (m_dvec4) {
        {
            v.v0 / norm,
            v.v1 / norm,
            v.v2 / norm,
            v.v3 / norm
        }
    };
}

/** dst = normalize(cross(a, b)) */
o_inline m_dvec4 dvec4_cross_normalized(m_dvec4 a, m_dvec4 b)
{
    m_dvec4 v = (m_dvec4) {
        {
            a.v1 * b.v2 - a.v2 * b.v1,
            a.v2 * b.v0 - a.v0 * b.v2,
            a.v0 * b.v1 - a.v1 * b.v0,
            0
        }
    };
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2
            + v.v3 * v.v3;
    double norm = md_sqrt(dot);
    norm = norm>0? norm : 1;
    return (m_dvec4) {
        {
            v.v0 / norm,
            v.v1 / norm,
            v.v2 / norm,
            v.v3 / norm
        }
    };
}


/** returns length of a dvector, see dvecn_norm. Just here to match glsl */
o_inline double dvec4_length(m_dvec4 v)
{
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2
            + v.v3 * v.v3;
    return md_sqrt(dot);
}


/** returns norm(b-a) */
o_inline double dvec4_distance(m_dvec4 a, m_dvec4 b)
{
    m_dvec4 v = (m_dvec4) {
        {
            b.v0 - a.v0,
            b.v1 - a.v1,
            b.v2 - a.v2,
            b.v3 - a.v3
        }};
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2
            + v.v3 * v.v3;
    return md_sqrt(dot);
}


/** returns dot(b-a) */
o_inline double dvec4_sqr_distance(m_dvec4 a, m_dvec4 b)
{
    m_dvec4 v = (m_dvec4) {
        {
            b.v0 - a.v0,
            b.v1 - a.v1,
            b.v2 - a.v2,
            b.v3 - a.v3
        }};
    return v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2
            + v.v3 * v.v3;
}


#endif //M_VEC_DVEC4_H
