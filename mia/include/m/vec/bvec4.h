#ifndef M_VEC_BVEC4_H
#define M_VEC_BVEC4_H

/**
 * @file vec/bvec4.h
 *
 * functions working with bvec4
 */


#include "m/sca/byte.h"
#include "m/m_types/byte.h"

/** dst = v0, v1, ... */
o_inline m_bvec4 bvec4_new(obyte v0, obyte v1, obyte v2, obyte v3)
{
    return (m_bvec4){{v0, v1, v2, v3}};
}

/** dst = (m_bvec4) v */
o_inline m_bvec4 bvec4_cast_double(const double *cast)
{
    return (m_bvec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = (m_bvec4) v */
o_inline m_bvec4 bvec4_cast_float(const float *cast)
{
    return (m_bvec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = (m_bvec4) v */
o_inline m_bvec4 bvec4_cast_int(const int *cast)
{
    return (m_bvec4){{cast[0], cast[1], cast[2], cast[3]}};
}

/** dst = v * 455 */
o_inline m_bvec4 bvec4_cast_float_1(const float *cast)
{
    return (m_bvec4){{cast[0] * 255, cast[1] * 255, cast[2] * 255, cast[3] * 255}};
}

/** dst = v * 455 */
o_inline m_bvec4 bvec4_cast_double_1(const double *cast)
{
    return (m_bvec4){{cast[0] * 255, cast[1] * 255, cast[2] * 255, cast[3] * 255}};
}


/** a == b */
o_inline bool bvec4_equals(m_bvec4 a, obyte b)
{
    return a.v0 == b
           && a.v1 == b
           && a.v2 == b
           && a.v3 == b;
}

/** a == b */
o_inline bool bvec4_equals_v(m_bvec4 a, m_bvec4 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1
           && a.v2 == b.v2
           && a.v3 == b.v3;
}

/** a ~= b */
o_inline bool bvec4_equals_eps(m_bvec4 a, obyte b, obyte eps)
{
    return mb_equals_eps(a.v0, b, eps)
           && mb_equals_eps(a.v1, b, eps)
           && mb_equals_eps(a.v2, b, eps)
           && mb_equals_eps(a.v3, b, eps);
}

/** a ~= b */
o_inline bool bvec4_equals_eps_v(m_bvec4 a, m_bvec4 b, obyte eps)
{
    return mb_equals_eps(a.v0, b.v0, eps)
           && mb_equals_eps(a.v1, b.v1, eps)
           && mb_equals_eps(a.v2, b.v2, eps)
           && mb_equals_eps(a.v3, b.v3, eps);
}


/** dst = s */
o_inline m_bvec4 bvec4_set(obyte s)
{
    return (m_bvec4){{s, s, s, s}};
}

/** dst = unit_x */
o_inline m_bvec4 bvec4_unit_x(void)
{
    return (m_bvec4){{1, 0, 0, 0}};
}


/** dst = unit_y */
o_inline m_bvec4 bvec4_unit_y(void)
{
    return (m_bvec4){{0, 1, 0, 0}};
}


/** dst = unit_z */
o_inline m_bvec4 bvec4_unit_z(void)
{
    return (m_bvec4){{0, 0, 1, 0}};
}


/** dst = unit_w */
o_inline m_bvec4 bvec4_unit_w(void)
{
    return (m_bvec4){{0, 0, 0, 1}};
}


/** dst = a + b */
o_inline m_bvec4 bvec4_add_v(m_bvec4 a, m_bvec4 b)
{
    return (m_bvec4){
        {
            a.v0 + b.v0,
            a.v1 + b.v1,
            a.v2 + b.v2,
            a.v3 + b.v3
        }
    };
}


/** dst = a + b */
o_inline m_bvec4 bvec4_add(m_bvec4 a, obyte b)
{
    return (m_bvec4){
        {
            a.v0 + b,
            a.v1 + b,
            a.v2 + b,
            a.v3 + b
        }
    };
}


/** dst = a - b */
o_inline m_bvec4 bvec4_sub_v(m_bvec4 a, m_bvec4 b)
{
    return (m_bvec4){
        {
            a.v0 - b.v0,
            a.v1 - b.v1,
            a.v2 - b.v2,
            a.v3 - b.v3
        }
    };
}


/** dst = a - b */
o_inline m_bvec4 bvec4_sub(m_bvec4 a, obyte b)
{
    return (m_bvec4){
        {
            a.v0 - b,
            a.v1 - b,
            a.v2 - b,
            a.v3 - b
        }
    };
}


/** dst = a * b */
o_inline m_bvec4 bvec4_scale_v(m_bvec4 a, m_bvec4 b)
{
    return (m_bvec4){
        {
            a.v0 * b.v0,
            a.v1 * b.v1,
            a.v2 * b.v2,
            a.v3 * b.v3
        }
    };
}


/** dst = a * b */
o_inline m_bvec4 bvec4_scale(m_bvec4 a, obyte b)
{
    return (m_bvec4){
        {
            a.v0 * b,
            a.v1 * b,
            a.v2 * b,
            a.v3 * b
        }
    };
}


/** dst = a / b */
o_inline m_bvec4 bvec4_div_v(m_bvec4 a, m_bvec4 b)
{
    return (m_bvec4){
        {
            a.v0 / b.v0,
            a.v1 / b.v1,
            a.v2 / b.v2,
            a.v3 / b.v3
        }
    };
}


/** dst = a / b */
o_inline m_bvec4 bvec4_div(m_bvec4 a, obyte b)
{
    return (m_bvec4){
        {
            a.v0 / b,
            a.v1 / b,
            a.v2 / b,
            a.v3 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_bvec4 bvec4_add_scaled_v(m_bvec4 a, m_bvec4 b, m_bvec4 c)
{
    return (m_bvec4){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1,
            a.v2 + b.v2 * c.v2,
            a.v3 + b.v3 * c.v3
        }
    };
}

/** dst = a + b * c */
o_inline m_bvec4 bvec4_add_scaled(m_bvec4 a, m_bvec4 b, obyte c)
{
    return (m_bvec4){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c,
            a.v2 + b.v2 * c,
            a.v3 + b.v3 * c
        }
    };
}


/** dst = x % y as positive version for obyteegers */
o_inline m_bvec4 bvec4_mod(m_bvec4 x, obyte y)
{
    return (m_bvec4){
        {
            mb_mod(x.v0, y),
            mb_mod(x.v1, y),
            mb_mod(x.v2, y),
            mb_mod(x.v3, y)
        }
    };
}

/** dst = x % y as positive version for obyteegers */
o_inline m_bvec4 bvec4_mod_v(m_bvec4 x, m_bvec4 y)
{
    return (m_bvec4){
        {
            mb_mod(x.v0, y.v0),
            mb_mod(x.v1, y.v1),
            mb_mod(x.v2, y.v2),
            mb_mod(x.v3, y.v3)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_bvec4 bvec4_min(m_bvec4 x, obyte y)
{
    return (m_bvec4){
        {
            mb_min(x.v0, y),
            mb_min(x.v1, y),
            mb_min(x.v2, y),
            mb_min(x.v3, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_bvec4 bvec4_min_v(m_bvec4 x, m_bvec4 y)
{
    return (m_bvec4){
        {
            mb_min(x.v0, y.v0),
            mb_min(x.v1, y.v1),
            mb_min(x.v2, y.v2),
            mb_min(x.v3, y.v3)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_bvec4 bvec4_max(m_bvec4 x, obyte y)
{
    return (m_bvec4){
        {
            mb_max(x.v0, y),
            mb_max(x.v1, y),
            mb_max(x.v2, y),
            mb_max(x.v3, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_bvec4 bvec4_max_v(m_bvec4 x, m_bvec4 y)
{
    return (m_bvec4){
        {
            mb_max(x.v0, y.v0),
            mb_max(x.v1, y.v1),
            mb_max(x.v2, y.v2),
            mb_max(x.v3, y.v3)
        }
    };
}


/** returns the index of the min v value */
o_inline obyte bvec4_min_index(m_bvec4 vec)
{
    obyte idx = vec.v0 <= vec.v1 ? 0 : 1;
    idx = vec.v[idx] <= vec.v2 ? idx : 2;
    idx = vec.v[idx] <= vec.v3 ? idx : 3;
    return idx;
}


/** returns the index of the max v value */
o_inline obyte bvec4_max_index(m_bvec4 vec)
{
    obyte idx = vec.v0 >= vec.v1 ? 0 : 1;
    idx = vec.v[idx] >= vec.v2 ? idx : 2;
    idx = vec.v[idx] >= vec.v3 ? idx : 3;
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_bvec4 bvec4_clamp(m_bvec4 x, obyte min, obyte max)
{
    return (m_bvec4){
        {
            mb_clamp(x.v0, min, max),
            mb_clamp(x.v1, min, max),
            mb_clamp(x.v2, min, max),
            mb_clamp(x.v3, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_bvec4 bvec4_clamp_v(m_bvec4 x, m_bvec4 min, m_bvec4 max)
{
    return (m_bvec4){
        {
            mb_clamp(x.v0, min.v0, max.v0),
            mb_clamp(x.v1, min.v1, max.v1),
            mb_clamp(x.v2, min.v2, max.v2),
            mb_clamp(x.v3, min.v3, max.v3)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_bvec4 bvec4_mix(m_bvec4 a, m_bvec4 b, float t)
{
    return (m_bvec4){
        {
            mb_mix(a.v0, b.v0, t),
            mb_mix(a.v1, b.v1, t),
            mb_mix(a.v2, b.v2, t),
            mb_mix(a.v3, b.v3, t)
        }
    };
}


/** returns v[0] + v[1] + ... + v[n-1] */
o_inline obyte bvec4_sum(m_bvec4 v)
{
    return v.v0 + v.v1 + v.v2 + v.v3;
}

#endif //M_VEC_BVEC4_H
