#ifndef M_VEC_IVEC3_H
#define M_VEC_IVEC3_H

/**
 * @file vec/ivec3.h
 *
 * functions working with ivec3
 */


#include "m/sca/int.h"
#include "m/m_types/int.h"

/** dst = v0, v1, ... */
o_inline m_ivec3 ivec3_new(int v0, int v1, int v2)
{
    return (m_ivec3){{v0, v1, v2}};
}

/** dst = (m_ivec3) v */
o_inline m_ivec3 ivec3_cast_double(const double *cast)
{
    return (m_ivec3){{cast[0], cast[1], cast[2]}};
}

/** dst = (m_ivec3) v */
o_inline m_ivec3 ivec3_cast_float(const float *cast)
{
    return (m_ivec3){{cast[0], cast[1], cast[2]}};
}

/** dst = (m_ivec3) v */
o_inline m_ivec3 ivec3_cast_byte(const obyte *cast)
{
    return (m_ivec3){{cast[0], cast[1], cast[2]}};
}


/** a == b */
o_inline bool ivec3_equals(m_ivec3 a, int b)
{
    return a.v0 == b
           && a.v1 == b
           && a.v2 == b;
}

/** a == b */
o_inline bool ivec3_equals_v(m_ivec3 a, m_ivec3 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1
           && a.v2 == b.v2;
}

/** a ~= b */
o_inline bool ivec3_equals_eps(m_ivec3 a, int b, int eps)
{
    return mi_equals_eps(a.v0, b, eps)
           && mi_equals_eps(a.v1, b, eps)
           && mi_equals_eps(a.v2, b, eps);
}

/** a ~= b */
o_inline bool ivec3_equals_eps_v(m_ivec3 a, m_ivec3 b, int eps)
{
    return mi_equals_eps(a.v0, b.v0, eps)
           && mi_equals_eps(a.v1, b.v1, eps)
           && mi_equals_eps(a.v2, b.v2, eps);
}


/** dst = s */
o_inline m_ivec3 ivec3_set(int s)
{
    return (m_ivec3){{s, s, s}};
}

/** dst = unit_x */
o_inline m_ivec3 ivec3_unit_x(void)
{
    return (m_ivec3){{1, 0, 0}};
}


/** dst = unit_y */
o_inline m_ivec3 ivec3_unit_y(void)
{
    return (m_ivec3){{0, 1, 0}};
}


/** dst = unit_z */
o_inline m_ivec3 ivec3_unit_z(void)
{
    return (m_ivec3){{0, 0, 1}};
}


/** dst = -v */
o_inline m_ivec3 ivec3_neg(m_ivec3 v)
{
    return (m_ivec3){{-v.v0, -v.v1, -v.v2}};
}


/** dst = a + b */
o_inline m_ivec3 ivec3_add_v(m_ivec3 a, m_ivec3 b)
{
    return (m_ivec3){
        {
            a.v0 + b.v0,
            a.v1 + b.v1,
            a.v2 + b.v2
        }
    };
}


/** dst = a + b */
o_inline m_ivec3 ivec3_add(m_ivec3 a, int b)
{
    return (m_ivec3){
        {
            a.v0 + b,
            a.v1 + b,
            a.v2 + b
        }
    };
}


/** dst = a - b */
o_inline m_ivec3 ivec3_sub_v(m_ivec3 a, m_ivec3 b)
{
    return (m_ivec3){
        {
            a.v0 - b.v0,
            a.v1 - b.v1,
            a.v2 - b.v2
        }
    };
}


/** dst = a - b */
o_inline m_ivec3 ivec3_sub(m_ivec3 a, int b)
{
    return (m_ivec3){
        {
            a.v0 - b,
            a.v1 - b,
            a.v2 - b
        }
    };
}


/** dst = a * b */
o_inline m_ivec3 ivec3_scale_v(m_ivec3 a, m_ivec3 b)
{
    return (m_ivec3){
        {
            a.v0 * b.v0,
            a.v1 * b.v1,
            a.v2 * b.v2
        }
    };
}


/** dst = a * b */
o_inline m_ivec3 ivec3_scale(m_ivec3 a, int b)
{
    return (m_ivec3){
        {
            a.v0 * b,
            a.v1 * b,
            a.v2 * b
        }
    };
}


/** dst = a / b */
o_inline m_ivec3 ivec3_div_v(m_ivec3 a, m_ivec3 b)
{
    return (m_ivec3){
        {
            a.v0 / b.v0,
            a.v1 / b.v1,
            a.v2 / b.v2
        }
    };
}


/** dst = a / b */
o_inline m_ivec3 ivec3_div(m_ivec3 a, int b)
{
    return (m_ivec3){
        {
            a.v0 / b,
            a.v1 / b,
            a.v2 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_ivec3 ivec3_add_scaled_v(m_ivec3 a, m_ivec3 b, m_ivec3 c)
{
    return (m_ivec3){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1,
            a.v2 + b.v2 * c.v2
        }
    };
}

/** dst = a + b * c */
o_inline m_ivec3 ivec3_add_scaled(m_ivec3 a, m_ivec3 b, int c)
{
    return (m_ivec3){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c,
            a.v2 + b.v2 * c
        }
    };
}


/** dst = abs(x) */
o_inline m_ivec3 ivec3_abs(m_ivec3 x)
{
    return (m_ivec3){
        {
            mi_abs(x.v0),
            mi_abs(x.v1),
            mi_abs(x.v2)
        }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_ivec3 ivec3_sign(m_ivec3 x)
{
    return (m_ivec3){
        {
            mi_sign(x.v0),
            mi_sign(x.v1),
            mi_sign(x.v2)
        }
    };
}

/** dst = x % y as positive version for integers */
o_inline m_ivec3 ivec3_mod(m_ivec3 x, int y)
{
    return (m_ivec3){
        {
            mi_mod(x.v0, y),
            mi_mod(x.v1, y),
            mi_mod(x.v2, y)
        }
    };
}

/** dst = x % y as positive version for integers */
o_inline m_ivec3 ivec3_mod_v(m_ivec3 x, m_ivec3 y)
{
    return (m_ivec3){
        {
            mi_mod(x.v0, y.v0),
            mi_mod(x.v1, y.v1),
            mi_mod(x.v2, y.v2)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_ivec3 ivec3_min(m_ivec3 x, int y)
{
    return (m_ivec3){
        {
            mi_min(x.v0, y),
            mi_min(x.v1, y),
            mi_min(x.v2, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_ivec3 ivec3_min_v(m_ivec3 x, m_ivec3 y)
{
    return (m_ivec3){
        {
            mi_min(x.v0, y.v0),
            mi_min(x.v1, y.v1),
            mi_min(x.v2, y.v2)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_ivec3 ivec3_max(m_ivec3 x, int y)
{
    return (m_ivec3){
        {
            mi_max(x.v0, y),
            mi_max(x.v1, y),
            mi_max(x.v2, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_ivec3 ivec3_max_v(m_ivec3 x, m_ivec3 y)
{
    return (m_ivec3){
        {
            mi_max(x.v0, y.v0),
            mi_max(x.v1, y.v1),
            mi_max(x.v2, y.v2)
        }
    };
}


/** returns the index of the min v value */
o_inline int ivec3_min_index(m_ivec3 vec)
{
    int idx = vec.v0 <= vec.v1 ? 0 : 1;
    idx = vec.v[idx] <= vec.v2 ? idx : 2;
    return idx;
}


/** returns the index of the max v value */
o_inline int ivec3_max_index(m_ivec3 vec)
{
    int idx = vec.v0 >= vec.v1 ? 0 : 1;
    idx = vec.v[idx] >= vec.v2 ? idx : 2;
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_ivec3 ivec3_clamp(m_ivec3 x, int min, int max)
{
    return (m_ivec3){
        {
            mi_clamp(x.v0, min, max),
            mi_clamp(x.v1, min, max),
            mi_clamp(x.v2, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_ivec3 ivec3_clamp_v(m_ivec3 x, m_ivec3 min, m_ivec3 max)
{
    return (m_ivec3){
        {
            mi_clamp(x.v0, min.v0, max.v0),
            mi_clamp(x.v1, min.v1, max.v1),
            mi_clamp(x.v2, min.v2, max.v2)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_ivec3 ivec3_mix(m_ivec3 a, m_ivec3 b, float t)
{
    return (m_ivec3){
        {
            mi_mix(a.v0, b.v0, t),
            mi_mix(a.v1, b.v1, t),
            mi_mix(a.v2, b.v2, t)
        }
    };
}


/** returns v[0] + v[1] + ... + v[n-1] */
o_inline int ivec3_sum(m_ivec3 v)
{
    return v.v0 + v.v1 + v.v2;
}


#endif //M_VEC_IVEC3_H
