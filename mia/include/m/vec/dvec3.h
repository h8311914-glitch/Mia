#ifndef M_VEC_DVEC3_H
#define M_VEC_DVEC3_H

/**
 * @file vec/dvec3.h
 *
 * functions working with dvec3
 */



#include "m/sca/dbl.h"
#include "m/m_types/dbl.h"

/** dst = v0, v1, ... */
o_inline m_dvec3 dvec3_new(double v0, double v1, double v2)
{
    return (m_dvec3){{v0, v1, v2}};
}

/** dst = (m_dvec3) v */
o_inline m_dvec3 dvec3_cast_float(const float *cast)
{
    return (m_dvec3){{cast[0], cast[1], cast[2]}};
}

/** dst = (m_dvec3) v */
o_inline m_dvec3 dvec3_cast_int(const int *cast)
{
    return (m_dvec3){{cast[0], cast[1], cast[2]}};
}

/** dst = (m_dvec3) v */
o_inline m_dvec3 dvec3_cast_byte(const obyte *cast)
{
    return (m_dvec3){{cast[0], cast[1], cast[2]}};
}

/** dst = v / 255 */
o_inline m_dvec3 dvec3_cast_byte_1(const obyte *cast)
{
    return (m_dvec3){{cast[0] / 255.0, cast[1] / 255.0, cast[2] / 255.0}};
}



/** a == b */
o_inline bool dvec3_equals(m_dvec3 a, double b)
{
    return a.v0 == b
           && a.v1 == b
           && a.v2 == b;
}

/** a == b */
o_inline bool dvec3_equals_v(m_dvec3 a, m_dvec3 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1
           && a.v2 == b.v2;
}

/** a ~= b */
o_inline bool dvec3_equals_eps(m_dvec3 a, double b, double eps)
{
    return md_equals_eps(a.v0, b, eps)
           && md_equals_eps(a.v1, b, eps)
           && md_equals_eps(a.v2, b, eps);
}

/** a ~= b */
o_inline bool dvec3_equals_eps_v(m_dvec3 a, m_dvec3 b, double eps)
{
    return md_equals_eps(a.v0, b.v0, eps)
           && md_equals_eps(a.v1, b.v1, eps)
           && md_equals_eps(a.v2, b.v2, eps);
}


/** dst = s */
o_inline m_dvec3 dvec3_set(double s)
{
    return (m_dvec3){{s, s, s}};
}

/** dst = unit_x */
o_inline m_dvec3 dvec3_unit_x(void)
{
    return (m_dvec3){{1, 0, 0}};
}


/** dst = unit_y */
o_inline m_dvec3 dvec3_unit_y(void)
{
    return (m_dvec3){{0, 1, 0}};
}


/** dst = unit_z */
o_inline m_dvec3 dvec3_unit_z(void)
{
    return (m_dvec3){{0, 0, 1}};
}


/** dst = -v */
o_inline m_dvec3 dvec3_neg(m_dvec3 v)
{
    return (m_dvec3){{-v.v0, -v.v1, -v.v2}};
}


/** dst = a + b */
o_inline m_dvec3 dvec3_add_v(m_dvec3 a, m_dvec3 b)
{
    return (m_dvec3){
            {
                a.v0 + b.v0,
                a.v1 + b.v1,
                a.v2 + b.v2
            }
    };
}


/** dst = a + b */
o_inline m_dvec3 dvec3_add(m_dvec3 a, double b)
{
    return (m_dvec3){
            {
                a.v0 + b,
                a.v1 + b,
                a.v2 + b
            }
    };
}


/** dst = a - b */
o_inline m_dvec3 dvec3_sub_v(m_dvec3 a, m_dvec3 b)
{
    return (m_dvec3){
            {
                a.v0 - b.v0,
                a.v1 - b.v1,
                a.v2 - b.v2
            }
    };
}


/** dst = a - b */
o_inline m_dvec3 dvec3_sub(m_dvec3 a, double b)
{
    return (m_dvec3){
            {
                a.v0 - b,
                a.v1 - b,
                a.v2 - b
            }
    };
}


/** dst = a * b */
o_inline m_dvec3 dvec3_scale_v(m_dvec3 a, m_dvec3 b)
{
    return (m_dvec3){
            {
                a.v0 * b.v0,
                a.v1 * b.v1,
                a.v2 * b.v2
            }
    };
}


/** dst = a * b */
o_inline m_dvec3 dvec3_scale(m_dvec3 a, double b)
{
    return (m_dvec3){
            {
                a.v0 * b,
                a.v1 * b,
                a.v2 * b
            }
    };
}


/** dst = a / b */
o_inline m_dvec3 dvec3_div_v(m_dvec3 a, m_dvec3 b)
{
    return (m_dvec3){
            {
                a.v0 / b.v0,
                a.v1 / b.v1,
                a.v2 / b.v2
            }
    };
}


/** dst = a / b */
o_inline m_dvec3 dvec3_div(m_dvec3 a, double b)
{
    return (m_dvec3){
            {
                a.v0 / b,
                a.v1 / b,
                a.v2 / b
            }
    };
}

/** dst = a + b * c */
o_inline m_dvec3 dvec3_add_scaled_v(m_dvec3 a, m_dvec3 b, m_dvec3 c)
{
    return (m_dvec3){
            {
                a.v0 + b.v0 * c.v0,
                a.v1 + b.v1 * c.v1,
                a.v2 + b.v2 * c.v2
            }
    };
}

/** dst = a + b * c */
o_inline m_dvec3 dvec3_add_scaled(m_dvec3 a, m_dvec3 b, double c)
{
    return (m_dvec3){
            {
                a.v0 + b.v0 * c,
                a.v1 + b.v1 * c,
                a.v2 + b.v2 * c
            }
    };
}


/** dst = a * M_PI / 180 */
o_inline m_dvec3 dvec3_radians(m_dvec3 deg)
{
    return (m_dvec3){
                {
                    md_radians(deg.v0),
                    md_radians(deg.v1),
                    md_radians(deg.v2)
                }
    };
}


/** dst = a * 180 / M_PI */
o_inline m_dvec3 dvec3_degrees(m_dvec3 rad)
{
    return (m_dvec3){
                {
                    md_degrees(rad.v0),
                    md_degrees(rad.v1),
                    md_degrees(rad.v2)
                }
    };
}


/** dst = abs(x) */
o_inline m_dvec3 dvec3_abs(m_dvec3 x)
{
    return (m_dvec3){
                {
                    md_abs(x.v0),
                    md_abs(x.v1),
                    md_abs(x.v2)
                }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_dvec3 dvec3_sign(m_dvec3 x)
{
    return (m_dvec3){
                {
                    md_sign(x.v0),
                    md_sign(x.v1),
                    md_sign(x.v2)
                }
    };
}


/** dst = round(x) */
o_inline m_dvec3 dvec3_round(m_dvec3 x)
{
    return (m_dvec3){
                {
                    md_round(x.v0),
                    md_round(x.v1),
                    md_round(x.v2)
                }
    };
}


/** dst = floor(x) */
o_inline m_dvec3 dvec3_floor(m_dvec3 x)
{
    return (m_dvec3){
                {
                    md_floor(x.v0),
                    md_floor(x.v1),
                    md_floor(x.v2)
                }
    };
}


/** dst = ceil(x) */
o_inline m_dvec3 dvec3_ceil(m_dvec3 x)
{
    return (m_dvec3){
                {
                    md_ceil(x.v0),
                    md_ceil(x.v1),
                    md_ceil(x.v2)
                }
    };
}


/** dst = x - floor(x) */
o_inline m_dvec3 dvec3_fract(m_dvec3 x)
{
    return (m_dvec3){
                {
                    md_fract(x.v0),
                    md_fract(x.v1),
                    md_fract(x.v2)
                }
    };
}


/** dst = x - y * floor(x/y) */
o_inline m_dvec3 dvec3_mod(m_dvec3 x, double y)
{
    return (m_dvec3){
                {
                    md_mod(x.v0, y),
                    md_mod(x.v1, y),
                    md_mod(x.v2, y)
                }
    };
}

/** dst = x - y * floor(x/y) */
o_inline m_dvec3 dvec3_mod_v(m_dvec3 x, m_dvec3 y)
{
    return (m_dvec3){
                {
                    md_mod(x.v0, y.v0),
                    md_mod(x.v1, y.v1),
                    md_mod(x.v2, y.v2)
                }
    };
}


/** dst = a < b ? a : b */
o_inline m_dvec3 dvec3_min(m_dvec3 x, double y)
{
    return (m_dvec3){
                {
                    md_min(x.v0, y),
                    md_min(x.v1, y),
                    md_min(x.v2, y)
                }
    };
}


/** dst = a < b ? a : b */
o_inline m_dvec3 dvec3_min_v(m_dvec3 x, m_dvec3 y)
{
    return (m_dvec3){
                {
                    md_min(x.v0, y.v0),
                    md_min(x.v1, y.v1),
                    md_min(x.v2, y.v2)
                }
    };
}


/** dst = a > b ? a : b */
o_inline m_dvec3 dvec3_max(m_dvec3 x, double y)
{
    return (m_dvec3){
                {
                    md_max(x.v0, y),
                    md_max(x.v1, y),
                    md_max(x.v2, y)
                }
    };
}


/** dst = a > b ? a : b */
o_inline m_dvec3 dvec3_max_v(m_dvec3 x, m_dvec3 y)
{
    return (m_dvec3){
                {
                    md_max(x.v0, y.v0),
                    md_max(x.v1, y.v1),
                    md_max(x.v2, y.v2)
                }
    };
}


/** returns the index of the min v value */
o_inline int dvec3_min_index(m_dvec3 vec)
{
    int idx = vec.v0 <= vec.v1? 0 : 1;
    idx = vec.v[idx] <= vec.v2? idx : 2;
    return idx;
}


/** returns the index of the max v value */
o_inline int dvec3_max_index(m_dvec3 vec)
{
    int idx = vec.v0 >= vec.v1? 0 : 1;
    idx = vec.v[idx] >= vec.v2? idx : 2;
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_dvec3 dvec3_clamp(m_dvec3 x, double min, double max)
{
    return (m_dvec3){
                {
                    md_clamp(x.v0, min, max),
                    md_clamp(x.v1, min, max),
                    md_clamp(x.v2, min, max)
                }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_dvec3 dvec3_clamp_v(m_dvec3 x, m_dvec3 min, m_dvec3 max)
{
    return (m_dvec3){
                {
                    md_clamp(x.v0, min.v0, max.v0),
                    md_clamp(x.v1, min.v1, max.v1),
                    md_clamp(x.v2, min.v2, max.v2)
                }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_dvec3 dvec3_mix(m_dvec3 a, m_dvec3 b, double t)
{
    return (m_dvec3){
                {
                    md_mix(a.v0, b.v0, t),
                    md_mix(a.v1, b.v1, t),
                    md_mix(a.v2, b.v2, t)
                }
    };
}

/** dst = a * (1-t) + b * t */
o_inline m_dvec3 dvec3_mix_v(m_dvec3 a, m_dvec3 b, m_dvec3 t)
{
    return (m_dvec3){
                {
                    md_mix(a.v0, b.v0, t.v0),
                    md_mix(a.v1, b.v1, t.v1),
                    md_mix(a.v2, b.v2, t.v2)
                }
    };
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline double dvec3_sum(m_dvec3 v)
{
    return v.v0 + v.v1 + v.v2;
}


/** returns a dot b */
o_inline double dvec3_dot(m_dvec3 a, m_dvec3 b)
{
    return a.v0 * b.v0
            + a.v1 * b.v1
            + a.v2 * b.v2;
}


/** dst = a x b , dst.w... = 0 */
o_inline m_dvec3 dvec3_cross(m_dvec3 a, m_dvec3 b)
{
    return (m_dvec3) {
            {
                a.v1 * b.v2 - a.v2 * b.v1,
                a.v2 * b.v0 - a.v0 * b.v2,
                a.v0 * b.v1 - a.v1 * b.v0
            }
    };
}


/** returns ||v||_3 */
o_inline double dvec3_norm(m_dvec3 v)
{
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    return md_sqrt(dot);
}


/** returns ||v||_p */
o_inline double dvec3_norm_p(m_dvec3 v, double p)
{
    double vpow = md_pow(v.v0, p)
            + md_pow(v.v1, p)
            + md_pow(v.v2, p);
    return md_pow(vpow, 1.0 / p);
}


/** returns ||v||_1 */
o_inline double dvec3_norm_1(m_dvec3 v)
{
    return md_abs(v.v0)
            + md_abs(v.v1)
            + md_abs(v.v2);
}


/** returns ||v||_inf */
o_inline double dvec3_norm_inf(m_dvec3 v)
{
    double inf = md_max(md_abs(v.v0), md_abs(v.v1));
    inf = md_max(inf, md_abs(v.v2));
    return inf;
}


/** dst = v / norm(v) */
o_inline m_dvec3 dvec3_normalize_unsafe(m_dvec3 v)
{
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    double norm = md_sqrt(dot);
    return (m_dvec3) {
            {
                v.v0 / norm,
                v.v1 / norm,
                v.v2 / norm
            }
    };
}


/** dst = v / (norm(v) > 0 ? norm(v) : 1) */
o_inline m_dvec3 dvec3_normalize(m_dvec3 v)
{
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    double norm = md_sqrt(dot);
    norm = norm>0? norm : 1;
    return (m_dvec3) {
            {
                v.v0 / norm,
                v.v1 / norm,
                v.v2 / norm
            }
    };
}

/** dst = normalize(cross(a, b)) */
o_inline m_dvec3 dvec3_cross_normalized(m_dvec3 a, m_dvec3 b)
{
    m_dvec3 v = (m_dvec3) {
            {
                a.v1 * b.v2 - a.v2 * b.v1,
                a.v2 * b.v0 - a.v0 * b.v2,
                a.v0 * b.v1 - a.v1 * b.v0
            }
    };
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    double norm = md_sqrt(dot);
    norm = norm>0? norm : 1;
    return (m_dvec3) {
            {
                v.v0 / norm,
                v.v1 / norm,
                v.v2 / norm
            }
    };
}


/** returns length of a vector, see vecn_norm. Just here to match glsl */
o_inline double dvec3_length(m_dvec3 v)
{
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    return md_sqrt(dot);
}


/** returns norm(b-a) */
o_inline double dvec3_distance(m_dvec3 a, m_dvec3 b)
{
    m_dvec3 v = (m_dvec3) {
            {
                b.v0 - a.v0,
                b.v1 - a.v1,
                b.v2 - a.v2
            }};
    double dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    return md_sqrt(dot);
}


/** returns dot(b-a) */
o_inline double dvec3_sqr_distance(m_dvec3 a, m_dvec3 b)
{
    m_dvec3 v = (m_dvec3) {
            {
                b.v0 - a.v0,
                b.v1 - a.v1,
                b.v2 - a.v2
            }};
    return v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
}


/** dst = dot(i, nref) < 0 ? N : -N */
o_inline m_dvec3 dvec3_faceforward(m_dvec3 n, m_dvec3 i, m_dvec3 nref)
{
    if (dvec3_dot(i, nref) < 0) {
        return n;
    }
    return dvec3_neg(n);
    }


/** dst = i - 2.0 * n * dot(n,i) */
o_inline m_dvec3 dvec3_reflect(m_dvec3 i, m_dvec3 n)
{
    dvec3 res = dvec3_scale(n, 2.0 * dvec3_dot(n, i));
    return dvec3_sub_v(i, res);
}


o_inline m_dvec3 dvec3_refract(m_dvec3 i, m_dvec3 n, double eta)
{
    dvec3 res = dvec3_neg(i);
    double cosi = dvec3_dot(res, n);
    double cost2 = (double) 1 - eta * eta * ((double) 1 - cosi * cosi);
    res = dvec3_scale(n, eta * cosi - md_sqrt(md_abs(cost2)));
    dvec3 t = dvec3_scale(i, eta);
    t = dvec3_add_v(t, res);
    return dvec3_scale(t, md_step(0, cost2));
}

#endif //M_VEC_DVEC3_H
