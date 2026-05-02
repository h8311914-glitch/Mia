#ifndef M_VEC_IVEC2_H
#define M_VEC_IVEC2_H

/**
 * @file vec/ivec2.h
 *
 * functions working with ivec2
 */


#include "ivecn.h"
#include "m/m_types/int.h"

/** dst = v0, v1, ... */
o_inline m_ivec2 ivec2_new(int v0, int v1)
{
    return (m_ivec2){{v0, v1}};
}

/** dst = (m_ivec2) v */
o_inline m_ivec2 ivec2_cast_double(const double *cast)
{
    return (m_ivec2){{cast[0], cast[1]}};
}

/** dst = (m_ivec2) v */
o_inline m_ivec2 ivec2_cast_float(const float *cast)
{
    return (m_ivec2){{cast[0], cast[1]}};
}

/** dst = (m_ivec2) v */
o_inline m_ivec2 ivec2_cast_byte(const obyte *cast)
{
    return (m_ivec2){{cast[0], cast[1]}};
}


/** a == b */
o_inline bool ivec2_equals(m_ivec2 a, int b)
{
    return a.v0 == b
           && a.v1 == b;
}

/** a == b */
o_inline bool ivec2_equals_v(m_ivec2 a, m_ivec2 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1;
}

/** a ~= b */
o_inline bool ivec2_equals_eps(m_ivec2 a, int b, int eps)
{
    return mi_equals_eps(a.v0, b, eps)
           && mi_equals_eps(a.v1, b, eps);
}

/** a ~= b */
o_inline bool ivec2_equals_eps_v(m_ivec2 a, m_ivec2 b, int eps)
{
    return mi_equals_eps(a.v0, b.v0, eps)
           && mi_equals_eps(a.v1, b.v1, eps);
}


/** dst = s */
o_inline m_ivec2 ivec2_set(int s)
{
    return (m_ivec2){{s, s}};
}

/** dst = unit_x */
o_inline m_ivec2 ivec2_unit_x(void)
{
    return (m_ivec2){{1, 0}};
}


/** dst = unit_y */
o_inline m_ivec2 ivec2_unit_y(void)
{
    return (m_ivec2){{0, 1}};
}


/** dst = -v */
o_inline m_ivec2 ivec2_neg(m_ivec2 v)
{
    return (m_ivec2){{-v.v0, -v.v1}};
}


/** dst = a + b */
o_inline m_ivec2 ivec2_add_v(m_ivec2 a, m_ivec2 b)
{
    return (m_ivec2){
        {
            a.v0 + b.v0,
            a.v1 + b.v1
        }
    };
}


/** dst = a + b */
o_inline m_ivec2 ivec2_add(m_ivec2 a, int b)
{
    return (m_ivec2){
        {
            a.v0 + b,
            a.v1 + b
        }
    };
}


/** dst = a - b */
o_inline m_ivec2 ivec2_sub_v(m_ivec2 a, m_ivec2 b)
{
    return (m_ivec2){
        {
            a.v0 - b.v0,
            a.v1 - b.v1
        }
    };
}


/** dst = a - b */
o_inline m_ivec2 ivec2_sub(m_ivec2 a, int b)
{
    return (m_ivec2){
        {
            a.v0 - b,
            a.v1 - b
        }
    };
}


/** dst = a * b */
o_inline m_ivec2 ivec2_scale_v(m_ivec2 a, m_ivec2 b)
{
    return (m_ivec2){
        {
            a.v0 * b.v0,
            a.v1 * b.v1
        }
    };
}


/** dst = a * b */
o_inline m_ivec2 ivec2_scale(m_ivec2 a, int b)
{
    return (m_ivec2){
        {
            a.v0 * b,
            a.v1 * b
        }
    };
}


/** dst = a / b */
o_inline m_ivec2 ivec2_div_v(m_ivec2 a, m_ivec2 b)
{
    return (m_ivec2){
        {
            a.v0 / b.v0,
            a.v1 / b.v1
        }
    };
}


/** dst = a / b */
o_inline m_ivec2 ivec2_div(m_ivec2 a, int b)
{
    return (m_ivec2){
        {
            a.v0 / b,
            a.v1 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_ivec2 ivec2_add_scaled_v(m_ivec2 a, m_ivec2 b, m_ivec2 c)
{
    return (m_ivec2){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1
        }
    };
}

/** dst = a + b * c */
o_inline m_ivec2 ivec2_add_scaled(m_ivec2 a, m_ivec2 b, int c)
{
    return (m_ivec2){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c
        }
    };
}

/** dst = abs(x) */
o_inline m_ivec2 ivec2_abs(m_ivec2 x)
{
    return (m_ivec2){
        {
            mi_abs(x.v0),
            mi_abs(x.v1)
        }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_ivec2 ivec2_sign(m_ivec2 x)
{
    return (m_ivec2){
        {
            mi_sign(x.v0),
            mi_sign(x.v1)
        }
    };
}


/** dst = x % y as positive version for integers */
o_inline m_ivec2 ivec2_mod(m_ivec2 x, int y)
{
    return (m_ivec2){
        {
            mi_mod(x.v0, y),
            mi_mod(x.v1, y)
        }
    };
}

/** dst = x % y as positive version for integers */
o_inline m_ivec2 ivec2_mod_v(m_ivec2 x, m_ivec2 y)
{
    return (m_ivec2){
        {
            mi_mod(x.v0, y.v0),
            mi_mod(x.v1, y.v1)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_ivec2 ivec2_min(m_ivec2 x, int y)
{
    return (m_ivec2){
        {
            mi_min(x.v0, y),
            mi_min(x.v1, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_ivec2 ivec2_min_v(m_ivec2 x, m_ivec2 y)
{
    return (m_ivec2){
        {
            mi_min(x.v0, y.v0),
            mi_min(x.v1, y.v1)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_ivec2 ivec2_max(m_ivec2 x, int y)
{
    return (m_ivec2){
        {
            mi_max(x.v0, y),
            mi_max(x.v1, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_ivec2 ivec2_max_v(m_ivec2 x, m_ivec2 y)
{
    return (m_ivec2){
        {
            mi_max(x.v0, y.v0),
            mi_max(x.v1, y.v1)
        }
    };
}


/** returns the index of the min v value */
o_inline int ivec2_min_index(m_ivec2 vec)
{
    return vec.v0 <= vec.v1 ? 0 : 1;
}


/** returns the index of the max v value */
o_inline int ivec2_max_index(m_ivec2 vec)
{
    return vec.v0 >= vec.v1 ? 0 : 1;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_ivec2 ivec2_clamp(m_ivec2 x, int min, int max)
{
    return (m_ivec2){
        {
            mi_clamp(x.v0, min, max),
            mi_clamp(x.v1, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_ivec2 ivec2_clamp_v(m_ivec2 x, m_ivec2 min, m_ivec2 max)
{
    return (m_ivec2){
        {
            mi_clamp(x.v0, min.v0, max.v0),
            mi_clamp(x.v1, min.v1, max.v1)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_ivec2 ivec2_mix(m_ivec2 a, m_ivec2 b, float t)
{
    return (m_ivec2){
        {
            mi_mix(a.v0, b.v0, t),
            mi_mix(a.v1, b.v1, t)
        }
    };
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline int ivec2_sum(m_ivec2 v)
{
    return v.v0 + v.v1;
}


#endif //M_VEC_IVEC2_H
