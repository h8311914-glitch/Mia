#ifndef M_VEC_VEC3_H
#define M_VEC_VEC3_H

/**
 * @file vec/vec3.h
 *
 * functions working with vec3
 */



#include "m/sca/flt.h"
#include "m/m_types/flt.h"

/** dst = v0, v1, ... */
o_inline m_vec3 vec3_new(float v0, float v1, float v2)
{
    return (m_vec3){{v0, v1, v2}};
}

/** dst = (m_vec3) v */
o_inline m_vec3 vec3_cast_double(const double *cast)
{
    return (m_vec3){{cast[0], cast[1], cast[2]}};
}

/** dst = (m_vec3) v */
o_inline m_vec3 vec3_cast_int(const int *cast)
{
    return (m_vec3){{cast[0], cast[1], cast[2]}};
}

/** dst = (m_vec3) v */
o_inline m_vec3 vec3_cast_byte(const obyte *cast)
{
    return (m_vec3){{cast[0], cast[1], cast[2]}};
}

/** dst = v / 255 */
o_inline m_vec3 vec3_cast_byte_1(const obyte *cast)
{
    return (m_vec3){{cast[0] / 255.0f, cast[1] / 255.0f, cast[2] / 255.0f}};
}



/** a == b */
o_inline bool vec3_equals(m_vec3 a, float b)
{
    return a.v0 == b
           && a.v1 == b
           && a.v2 == b;
}

/** a == b */
o_inline bool vec3_equals_v(m_vec3 a, m_vec3 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1
           && a.v2 == b.v2;
}

/** a ~= b */
o_inline bool vec3_equals_eps(m_vec3 a, float b, float eps)
{
    return m_equals_eps(a.v0, b, eps)
           && m_equals_eps(a.v1, b, eps)
           && m_equals_eps(a.v2, b, eps);
}

/** a ~= b */
o_inline bool vec3_equals_eps_v(m_vec3 a, m_vec3 b, float eps)
{
    return m_equals_eps(a.v0, b.v0, eps)
           && m_equals_eps(a.v1, b.v1, eps)
           && m_equals_eps(a.v2, b.v2, eps);
}


/** dst = s */
o_inline m_vec3 vec3_set(float s)
{
    return (m_vec3){{s, s, s}};
}

/** dst = unit_x */
o_inline m_vec3 vec3_unit_x(void)
{
    return (m_vec3){{1, 0, 0}};
}


/** dst = unit_y */
o_inline m_vec3 vec3_unit_y(void)
{
    return (m_vec3){{0, 1, 0}};
}


/** dst = unit_z */
o_inline m_vec3 vec3_unit_z(void)
{
    return (m_vec3){{0, 0, 1}};
}


/** dst = -v */
o_inline m_vec3 vec3_neg(m_vec3 v)
{
    return (m_vec3){{-v.v0, -v.v1, -v.v2}};
}


/** dst = a + b */
o_inline m_vec3 vec3_add_v(m_vec3 a, m_vec3 b)
{
    return (m_vec3){
            {
                a.v0 + b.v0,
                a.v1 + b.v1,
                a.v2 + b.v2
            }
    };
}


/** dst = a + b */
o_inline m_vec3 vec3_add(m_vec3 a, float b)
{
    return (m_vec3){
            {
                a.v0 + b,
                a.v1 + b,
                a.v2 + b
            }
    };
}


/** dst = a - b */
o_inline m_vec3 vec3_sub_v(m_vec3 a, m_vec3 b)
{
    return (m_vec3){
            {
                a.v0 - b.v0,
                a.v1 - b.v1,
                a.v2 - b.v2
            }
    };
}


/** dst = a - b */
o_inline m_vec3 vec3_sub(m_vec3 a, float b)
{
    return (m_vec3){
            {
                a.v0 - b,
                a.v1 - b,
                a.v2 - b
            }
    };
}


/** dst = a * b */
o_inline m_vec3 vec3_scale_v(m_vec3 a, m_vec3 b)
{
    return (m_vec3){
            {
                a.v0 * b.v0,
                a.v1 * b.v1,
                a.v2 * b.v2
            }
    };
}


/** dst = a * b */
o_inline m_vec3 vec3_scale(m_vec3 a, float b)
{
    return (m_vec3){
            {
                a.v0 * b,
                a.v1 * b,
                a.v2 * b
            }
    };
}


/** dst = a / b */
o_inline m_vec3 vec3_div_v(m_vec3 a, m_vec3 b)
{
    return (m_vec3){
            {
                a.v0 / b.v0,
                a.v1 / b.v1,
                a.v2 / b.v2
            }
    };
}


/** dst = a / b */
o_inline m_vec3 vec3_div(m_vec3 a, float b)
{
    return (m_vec3){
            {
                a.v0 / b,
                a.v1 / b,
                a.v2 / b
            }
    };
}

/** dst = a + b * c */
o_inline m_vec3 vec3_add_scaled_v(m_vec3 a, m_vec3 b, m_vec3 c)
{
    return (m_vec3){
            {
                a.v0 + b.v0 * c.v0,
                a.v1 + b.v1 * c.v1,
                a.v2 + b.v2 * c.v2
            }
    };
}

/** dst = a + b * c */
o_inline m_vec3 vec3_add_scaled(m_vec3 a, m_vec3 b, float c)
{
    return (m_vec3){
            {
                a.v0 + b.v0 * c,
                a.v1 + b.v1 * c,
                a.v2 + b.v2 * c
            }
    };
}


/** dst = a * M_PI / 180 */
o_inline m_vec3 vec3_radians(m_vec3 deg)
{
    return (m_vec3){
                {
                    m_radians(deg.v0),
                    m_radians(deg.v1),
                    m_radians(deg.v2)
                }
    };
}


/** dst = a * 180 / M_PI */
o_inline m_vec3 vec3_degrees(m_vec3 rad)
{
    return (m_vec3){
                {
                    m_degrees(rad.v0),
                    m_degrees(rad.v1),
                    m_degrees(rad.v2)
                }
    };
}


/** dst = abs(x) */
o_inline m_vec3 vec3_abs(m_vec3 x)
{
    return (m_vec3){
                {
                    m_abs(x.v0),
                    m_abs(x.v1),
                    m_abs(x.v2)
                }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_vec3 vec3_sign(m_vec3 x)
{
    return (m_vec3){
                {
                    m_sign(x.v0),
                    m_sign(x.v1),
                    m_sign(x.v2)
                }
    };
}


/** dst = round(x) */
o_inline m_vec3 vec3_round(m_vec3 x)
{
    return (m_vec3){
                {
                    m_round(x.v0),
                    m_round(x.v1),
                    m_round(x.v2)
                }
    };
}


/** dst = floor(x) */
o_inline m_vec3 vec3_floor(m_vec3 x)
{
    return (m_vec3){
                {
                    m_floor(x.v0),
                    m_floor(x.v1),
                    m_floor(x.v2)
                }
    };
}


/** dst = ceil(x) */
o_inline m_vec3 vec3_ceil(m_vec3 x)
{
    return (m_vec3){
                {
                    m_ceil(x.v0),
                    m_ceil(x.v1),
                    m_ceil(x.v2)
                }
    };
}


/** dst = x - floor(x) */
o_inline m_vec3 vec3_fract(m_vec3 x)
{
    return (m_vec3){
                {
                    m_fract(x.v0),
                    m_fract(x.v1),
                    m_fract(x.v2)
                }
    };
}


/** dst = x - y * floor(x/y) */
o_inline m_vec3 vec3_mod(m_vec3 x, float y)
{
    return (m_vec3){
                {
                    m_mod(x.v0, y),
                    m_mod(x.v1, y),
                    m_mod(x.v2, y)
                }
    };
}

/** dst = x - y * floor(x/y) */
o_inline m_vec3 vec3_mod_v(m_vec3 x, m_vec3 y)
{
    return (m_vec3){
                {
                    m_mod(x.v0, y.v0),
                    m_mod(x.v1, y.v1),
                    m_mod(x.v2, y.v2)
                }
    };
}


/** dst = a < b ? a : b */
o_inline m_vec3 vec3_min(m_vec3 x, float y)
{
    return (m_vec3){
                {
                    m_min(x.v0, y),
                    m_min(x.v1, y),
                    m_min(x.v2, y)
                }
    };
}


/** dst = a < b ? a : b */
o_inline m_vec3 vec3_min_v(m_vec3 x, m_vec3 y)
{
    return (m_vec3){
                {
                    m_min(x.v0, y.v0),
                    m_min(x.v1, y.v1),
                    m_min(x.v2, y.v2)
                }
    };
}


/** dst = a > b ? a : b */
o_inline m_vec3 vec3_max(m_vec3 x, float y)
{
    return (m_vec3){
                {
                    m_max(x.v0, y),
                    m_max(x.v1, y),
                    m_max(x.v2, y)
                }
    };
}


/** dst = a > b ? a : b */
o_inline m_vec3 vec3_max_v(m_vec3 x, m_vec3 y)
{
    return (m_vec3){
                {
                    m_max(x.v0, y.v0),
                    m_max(x.v1, y.v1),
                    m_max(x.v2, y.v2)
                }
    };
}


/** returns the index of the min v value */
o_inline int vec3_min_index(m_vec3 vec)
{
    int idx = vec.v0 <= vec.v1? 0 : 1;
    idx = vec.v[idx] <= vec.v2? idx : 2;
    return idx;
}


/** returns the index of the max v value */
o_inline int vec3_max_index(m_vec3 vec)
{
    int idx = vec.v0 >= vec.v1? 0 : 1;
    idx = vec.v[idx] >= vec.v2? idx : 2;
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_vec3 vec3_clamp(m_vec3 x, float min, float max)
{
    return (m_vec3){
                {
                    m_clamp(x.v0, min, max),
                    m_clamp(x.v1, min, max),
                    m_clamp(x.v2, min, max)
                }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_vec3 vec3_clamp_v(m_vec3 x, m_vec3 min, m_vec3 max)
{
    return (m_vec3){
                {
                    m_clamp(x.v0, min.v0, max.v0),
                    m_clamp(x.v1, min.v1, max.v1),
                    m_clamp(x.v2, min.v2, max.v2)
                }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_vec3 vec3_mix(m_vec3 a, m_vec3 b, float t)
{
    return (m_vec3){
                {
                    m_mix(a.v0, b.v0, t),
                    m_mix(a.v1, b.v1, t),
                    m_mix(a.v2, b.v2, t)
                }
    };
}

/** dst = a * (1-t) + b * t */
o_inline m_vec3 vec3_mix_v(m_vec3 a, m_vec3 b, m_vec3 t)
{
    return (m_vec3){
                {
                    m_mix(a.v0, b.v0, t.v0),
                    m_mix(a.v1, b.v1, t.v1),
                    m_mix(a.v2, b.v2, t.v2)
                }
    };
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline float vec3_sum(m_vec3 v)
{
    return v.v0 + v.v1 + v.v2;
}


/** returns a dot b */
o_inline float vec3_dot(m_vec3 a, m_vec3 b)
{
    return a.v0 * b.v0
            + a.v1 * b.v1
            + a.v2 * b.v2;
}


/** dst = a x b , dst.w... = 0 */
o_inline m_vec3 vec3_cross(m_vec3 a, m_vec3 b)
{
    return (m_vec3) {
            {
                a.v1 * b.v2 - a.v2 * b.v1,
                a.v2 * b.v0 - a.v0 * b.v2,
                a.v0 * b.v1 - a.v1 * b.v0
            }
    };
}


/** returns ||v||_3 */
o_inline float vec3_norm(m_vec3 v)
{
    float dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    return m_sqrt(dot);
}


/** returns ||v||_p */
o_inline float vec3_norm_p(m_vec3 v, float p)
{
    float vpow = m_pow(v.v0, p)
            + m_pow(v.v1, p)
            + m_pow(v.v2, p);
    return m_pow(vpow, 1.0f / p);
}


/** returns ||v||_1 */
o_inline float vec3_norm_1(m_vec3 v)
{
    return m_abs(v.v0)
            + m_abs(v.v1)
            + m_abs(v.v2);
}


/** returns ||v||_inf */
o_inline float vec3_norm_inf(m_vec3 v)
{
    float inf = m_max(m_abs(v.v0), m_abs(v.v1));
    inf = m_max(inf, m_abs(v.v2));
    return inf;
}


/** dst = v / norm(v) */
o_inline m_vec3 vec3_normalize_unsafe(m_vec3 v)
{
    float dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    float norm = m_sqrt(dot);
    return (m_vec3) {
            {
                v.v0 / norm,
                v.v1 / norm,
                v.v2 / norm
            }
    };
}


/** dst = v / (norm(v) > 0 ? norm(v) : 1) */
o_inline m_vec3 vec3_normalize(m_vec3 v)
{
    float dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    float norm = m_sqrt(dot);
    norm = norm>0? norm : 1;
    return (m_vec3) {
            {
                v.v0 / norm,
                v.v1 / norm,
                v.v2 / norm
            }
    };
}

/** dst = normalize(cross(a, b)) */
o_inline m_vec3 vec3_cross_normalized(m_vec3 a, m_vec3 b)
{
    m_vec3 v = (m_vec3) {
            {
                a.v1 * b.v2 - a.v2 * b.v1,
                a.v2 * b.v0 - a.v0 * b.v2,
                a.v0 * b.v1 - a.v1 * b.v0
            }
    };
    float dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    float norm = m_sqrt(dot);
    norm = norm>0? norm : 1;
    return (m_vec3) {
            {
                v.v0 / norm,
                v.v1 / norm,
                v.v2 / norm
            }
    };
}


/** returns length of a vector, see vecn_norm. Just here to match glsl */
o_inline float vec3_length(m_vec3 v)
{
    float dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    return m_sqrt(dot);
}


/** returns norm(b-a) */
o_inline float vec3_distance(m_vec3 a, m_vec3 b)
{
    m_vec3 v = (m_vec3) {
            {
                b.v0 - a.v0,
                b.v1 - a.v1,
                b.v2 - a.v2
            }};
    float dot = v.v0 * v.v0
            + v.v1 * v.v1
            + v.v2 * v.v2;
    return m_sqrt(dot);
}


/** returns dot(b-a) */
o_inline float vec3_sqr_distance(m_vec3 a, m_vec3 b)
{
    m_vec3 v = (m_vec3) {
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
o_inline m_vec3 vec3_faceforward(m_vec3 n, m_vec3 i, m_vec3 nref)
{
    if (vec3_dot(i, nref) < 0) {
        return n;
    }
    return vec3_neg(n);
    }


/** dst = i - 2.0 * n * dot(n,i) */
o_inline m_vec3 vec3_reflect(m_vec3 i, m_vec3 n)
{
    m_vec3 res = vec3_scale(n, 2.0f * vec3_dot(n, i));
    return vec3_sub_v(i, res);
}


o_inline m_vec3 vec3_refract(m_vec3 i, m_vec3 n, float eta)
{
    m_vec3 res = vec3_neg(i);
    float cosi = vec3_dot(res, n);
    float cost2 = (float) 1 - eta * eta * ((float) 1 - cosi * cosi);
    res = vec3_scale(n, eta * cosi - m_sqrt(m_abs(cost2)));
    m_vec3 t = vec3_scale(i, eta);
    t = vec3_add_v(t, res);
    return vec3_scale(t, m_step(0, cost2));
}

#endif //M_VEC_VEC3_H
