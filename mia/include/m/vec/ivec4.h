#ifndef M_VEC_IVEC4_H
#define M_VEC_IVEC4_H

/**
 * @file vec/ivec4.h
 *
 * functions working with ivec4
 */


#include "m/sca/int.h"
#include "m/m_types/int.h"

/** dst = v0, v1, ... */
o_inline m_ivec4 ivec4_new(int v0, int v1, int v2, int v3)
{
    return (m_ivec4){{v0, v1, v2, v3}};
}

/** dst = (m_ivec4) v */
o_inline m_ivec4 ivec4_cast_double(const double *cast)
{
    return (m_ivec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = (m_ivec4) v */
o_inline m_ivec4 ivec4_cast_float(const float *cast)
{
    return (m_ivec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = (m_ivec4) v */
o_inline m_ivec4 ivec4_cast_byte(const obyte *cast)
{
    return (m_ivec4){{cast[0], cast[1], cast[2], cast[3]}};
}


/** a == b */
o_inline bool ivec4_equals(m_ivec4 a, int b)
{
    return a.v0 == b
           && a.v1 == b
           && a.v2 == b
           && a.v3 == b;
}

/** a == b */
o_inline bool ivec4_equals_v(m_ivec4 a, m_ivec4 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1
           && a.v2 == b.v2
           && a.v3 == b.v3;
}

/** a ~= b */
o_inline bool ivec4_equals_eps(m_ivec4 a, int b, int eps)
{
    return mi_equals_eps(a.v0, b, eps)
           && mi_equals_eps(a.v1, b, eps)
           && mi_equals_eps(a.v2, b, eps)
           && mi_equals_eps(a.v3, b, eps);
}

/** a ~= b */
o_inline bool ivec4_equals_eps_v(m_ivec4 a, m_ivec4 b, int eps)
{
    return mi_equals_eps(a.v0, b.v0, eps)
           && mi_equals_eps(a.v1, b.v1, eps)
           && mi_equals_eps(a.v2, b.v2, eps)
           && mi_equals_eps(a.v3, b.v3, eps);
}


/** dst = s */
o_inline m_ivec4 ivec4_set(int s)
{
    return (m_ivec4){{s, s, s, s}};
}

/** dst = unit_x */
o_inline m_ivec4 ivec4_unit_x(void)
{
    return (m_ivec4){{1, 0, 0, 0}};
}


/** dst = unit_y */
o_inline m_ivec4 ivec4_unit_y(void)
{
    return (m_ivec4){{0, 1, 0, 0}};
}


/** dst = unit_z */
o_inline m_ivec4 ivec4_unit_z(void)
{
    return (m_ivec4){{0, 0, 1, 0}};
}


/** dst = unit_w */
o_inline m_ivec4 ivec4_unit_w(void)
{
    return (m_ivec4){{0, 0, 0, 1}};
}


/** dst = -v */
o_inline m_ivec4 ivec4_neg(m_ivec4 v)
{
    return (m_ivec4){{-v.v0, -v.v1, -v.v2, -v.v3}};
}


/** dst = a + b */
o_inline m_ivec4 ivec4_add_v(m_ivec4 a, m_ivec4 b)
{
    return (m_ivec4){
        {
            a.v0 + b.v0,
            a.v1 + b.v1,
            a.v2 + b.v2,
            a.v3 + b.v3
        }
    };
}


/** dst = a + b */
o_inline m_ivec4 ivec4_add(m_ivec4 a, int b)
{
    return (m_ivec4){
        {
            a.v0 + b,
            a.v1 + b,
            a.v2 + b,
            a.v3 + b
        }
    };
}


/** dst = a - b */
o_inline m_ivec4 ivec4_sub_v(m_ivec4 a, m_ivec4 b)
{
    return (m_ivec4){
        {
            a.v0 - b.v0,
            a.v1 - b.v1,
            a.v2 - b.v2,
            a.v3 - b.v3
        }
    };
}


/** dst = a - b */
o_inline m_ivec4 ivec4_sub(m_ivec4 a, int b)
{
    return (m_ivec4){
        {
            a.v0 - b,
            a.v1 - b,
            a.v2 - b,
            a.v3 - b
        }
    };
}


/** dst = a * b */
o_inline m_ivec4 ivec4_scale_v(m_ivec4 a, m_ivec4 b)
{
    return (m_ivec4){
        {
            a.v0 * b.v0,
            a.v1 * b.v1,
            a.v2 * b.v2,
            a.v3 * b.v3
        }
    };
}


/** dst = a * b */
o_inline m_ivec4 ivec4_scale(m_ivec4 a, int b)
{
    return (m_ivec4){
        {
            a.v0 * b,
            a.v1 * b,
            a.v2 * b,
            a.v3 * b
        }
    };
}


/** dst = a / b */
o_inline m_ivec4 ivec4_div_v(m_ivec4 a, m_ivec4 b)
{
    return (m_ivec4){
        {
            a.v0 / b.v0,
            a.v1 / b.v1,
            a.v2 / b.v2,
            a.v3 / b.v3
        }
    };
}


/** dst = a / b */
o_inline m_ivec4 ivec4_div(m_ivec4 a, int b)
{
    return (m_ivec4){
        {
            a.v0 / b,
            a.v1 / b,
            a.v2 / b,
            a.v3 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_ivec4 ivec4_add_scaled_v(m_ivec4 a, m_ivec4 b, m_ivec4 c)
{
    return (m_ivec4){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1,
            a.v2 + b.v2 * c.v2,
            a.v3 + b.v3 * c.v3
        }
    };
}

/** dst = a + b * c */
o_inline m_ivec4 ivec4_add_scaled(m_ivec4 a, m_ivec4 b, int c)
{
    return (m_ivec4){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c,
            a.v2 + b.v2 * c,
            a.v3 + b.v3 * c
        }
    };
}

/** dst = abs(x) */
o_inline m_ivec4 ivec4_abs(m_ivec4 x)
{
    return (m_ivec4){
        {
            mi_abs(x.v0),
            mi_abs(x.v1),
            mi_abs(x.v2),
            mi_abs(x.v3)
        }
    };
}


/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline m_ivec4 ivec4_sign(m_ivec4 x)
{
    return (m_ivec4){
        {
            mi_sign(x.v0),
            mi_sign(x.v1),
            mi_sign(x.v2),
            mi_sign(x.v3)
        }
    };
}

/** dst = x % y as positive version for integers */
o_inline m_ivec4 ivec4_mod(m_ivec4 x, int y)
{
    return (m_ivec4){
        {
            mi_mod(x.v0, y),
            mi_mod(x.v1, y),
            mi_mod(x.v2, y),
            mi_mod(x.v3, y)
        }
    };
}

/** dst = x % y as positive version for integers */
o_inline m_ivec4 ivec4_mod_v(m_ivec4 x, m_ivec4 y)
{
    return (m_ivec4){
        {
            mi_mod(x.v0, y.v0),
            mi_mod(x.v1, y.v1),
            mi_mod(x.v2, y.v2),
            mi_mod(x.v3, y.v3)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_ivec4 ivec4_min(m_ivec4 x, int y)
{
    return (m_ivec4){
        {
            mi_min(x.v0, y),
            mi_min(x.v1, y),
            mi_min(x.v2, y),
            mi_min(x.v3, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_ivec4 ivec4_min_v(m_ivec4 x, m_ivec4 y)
{
    return (m_ivec4){
        {
            mi_min(x.v0, y.v0),
            mi_min(x.v1, y.v1),
            mi_min(x.v2, y.v2),
            mi_min(x.v3, y.v3)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_ivec4 ivec4_max(m_ivec4 x, int y)
{
    return (m_ivec4){
        {
            mi_max(x.v0, y),
            mi_max(x.v1, y),
            mi_max(x.v2, y),
            mi_max(x.v3, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_ivec4 ivec4_max_v(m_ivec4 x, m_ivec4 y)
{
    return (m_ivec4){
        {
            mi_max(x.v0, y.v0),
            mi_max(x.v1, y.v1),
            mi_max(x.v2, y.v2),
            mi_max(x.v3, y.v3)
        }
    };
}


/** returns the index of the min v value */
o_inline int ivec4_min_index(m_ivec4 vec)
{
    int idx = vec.v0 <= vec.v1 ? 0 : 1;
    idx = vec.v[idx] <= vec.v2 ? idx : 2;
    idx = vec.v[idx] <= vec.v3 ? idx : 3;
    return idx;
}


/** returns the index of the max v value */
o_inline int ivec4_max_index(m_ivec4 vec)
{
    int idx = vec.v0 >= vec.v1 ? 0 : 1;
    idx = vec.v[idx] >= vec.v2 ? idx : 2;
    idx = vec.v[idx] >= vec.v3 ? idx : 3;
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_ivec4 ivec4_clamp(m_ivec4 x, int min, int max)
{
    return (m_ivec4){
        {
            mi_clamp(x.v0, min, max),
            mi_clamp(x.v1, min, max),
            mi_clamp(x.v2, min, max),
            mi_clamp(x.v3, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_ivec4 ivec4_clamp_v(m_ivec4 x, m_ivec4 min, m_ivec4 max)
{
    return (m_ivec4){
        {
            mi_clamp(x.v0, min.v0, max.v0),
            mi_clamp(x.v1, min.v1, max.v1),
            mi_clamp(x.v2, min.v2, max.v2),
            mi_clamp(x.v3, min.v3, max.v3)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_ivec4 ivec4_mix(m_ivec4 a, m_ivec4 b, float t)
{
    return (m_ivec4){
        {
            mi_mix(a.v0, b.v0, t),
            mi_mix(a.v1, b.v1, t),
            mi_mix(a.v2, b.v2, t),
            mi_mix(a.v3, b.v3, t)
        }
    };
}


/** returns v[0] + v[1] + ... + v[n-1] */
o_inline int ivec4_sum(m_ivec4 v)
{
    return v.v0 + v.v1 + v.v2 + v.v3;
}


#endif //M_VEC_IVEC4_H
