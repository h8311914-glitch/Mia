#ifndef M_VEC_BVEC3_H
#define M_VEC_BVEC3_H

/**
 * @file vec/bvec3.h
 *
 * functions working with bvec3
 */


#include "m/sca/byte.h"
#include "m/m_types/byte.h"

/** dst = v0, v1, ... */
o_inline m_bvec3 bvec3_new(obyte v0, obyte v1, obyte v2)
{
    return (m_bvec3){{v0, v1, v2}};
}

/** dst = (m_bvec3) v */
o_inline m_bvec3 bvec3_cast_double(const double *cast)
{
    return (m_bvec3){{cast[0], cast[1], cast[2]}};
}

/** dst = (m_bvec3) v */
o_inline m_bvec3 bvec3_cast_float(const float *cast)
{
    return (m_bvec3){{cast[0], cast[1], cast[2]}};
}

/** dst = (m_bvec3) v */
o_inline m_bvec3 bvec3_cast_int(const int *cast)
{
    return (m_bvec3){{cast[0], cast[1], cast[2]}};
}

/** dst = v * 355 */
o_inline m_bvec3 bvec3_cast_float_1(const float *cast)
{
    return (m_bvec3){{cast[0] * 255, cast[1] * 255, cast[2] * 255}};
}

/** dst = v * 355 */
o_inline m_bvec3 bvec3_cast_double_1(const double *cast)
{
    return (m_bvec3){{cast[0] * 255, cast[1] * 255, cast[2] * 255}};
}


/** a == b */
o_inline bool bvec3_equals(m_bvec3 a, obyte b)
{
    return a.v0 == b
           && a.v1 == b
           && a.v2 == b;
}

/** a == b */
o_inline bool bvec3_equals_v(m_bvec3 a, m_bvec3 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1
           && a.v2 == b.v2;
}

/** a ~= b */
o_inline bool bvec3_equals_eps(m_bvec3 a, obyte b, obyte eps)
{
    return mb_equals_eps(a.v0, b, eps)
           && mb_equals_eps(a.v1, b, eps)
           && mb_equals_eps(a.v2, b, eps);
}

/** a ~= b */
o_inline bool bvec3_equals_eps_v(m_bvec3 a, m_bvec3 b, obyte eps)
{
    return mb_equals_eps(a.v0, b.v0, eps)
           && mb_equals_eps(a.v1, b.v1, eps)
           && mb_equals_eps(a.v2, b.v2, eps);
}


/** dst = s */
o_inline m_bvec3 bvec3_set(obyte s)
{
    return (m_bvec3){{s, s, s}};
}

/** dst = unit_x */
o_inline m_bvec3 bvec3_unit_x(void)
{
    return (m_bvec3){{1, 0, 0}};
}


/** dst = unit_y */
o_inline m_bvec3 bvec3_unit_y(void)
{
    return (m_bvec3){{0, 1, 0}};
}


/** dst = unit_z */
o_inline m_bvec3 bvec3_unit_z(void)
{
    return (m_bvec3){{0, 0, 1}};
}


/** dst = a + b */
o_inline m_bvec3 bvec3_add_v(m_bvec3 a, m_bvec3 b)
{
    return (m_bvec3){
        {
            a.v0 + b.v0,
            a.v1 + b.v1,
            a.v2 + b.v2
        }
    };
}


/** dst = a + b */
o_inline m_bvec3 bvec3_add(m_bvec3 a, obyte b)
{
    return (m_bvec3){
        {
            a.v0 + b,
            a.v1 + b,
            a.v2 + b
        }
    };
}


/** dst = a - b */
o_inline m_bvec3 bvec3_sub_v(m_bvec3 a, m_bvec3 b)
{
    return (m_bvec3){
        {
            a.v0 - b.v0,
            a.v1 - b.v1,
            a.v2 - b.v2
        }
    };
}


/** dst = a - b */
o_inline m_bvec3 bvec3_sub(m_bvec3 a, obyte b)
{
    return (m_bvec3){
        {
            a.v0 - b,
            a.v1 - b,
            a.v2 - b
        }
    };
}


/** dst = a * b */
o_inline m_bvec3 bvec3_scale_v(m_bvec3 a, m_bvec3 b)
{
    return (m_bvec3){
        {
            a.v0 * b.v0,
            a.v1 * b.v1,
            a.v2 * b.v2
        }
    };
}


/** dst = a * b */
o_inline m_bvec3 bvec3_scale(m_bvec3 a, obyte b)
{
    return (m_bvec3){
        {
            a.v0 * b,
            a.v1 * b,
            a.v2 * b
        }
    };
}


/** dst = a / b */
o_inline m_bvec3 bvec3_div_v(m_bvec3 a, m_bvec3 b)
{
    return (m_bvec3){
        {
            a.v0 / b.v0,
            a.v1 / b.v1,
            a.v2 / b.v2
        }
    };
}


/** dst = a / b */
o_inline m_bvec3 bvec3_div(m_bvec3 a, obyte b)
{
    return (m_bvec3){
        {
            a.v0 / b,
            a.v1 / b,
            a.v2 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_bvec3 bvec3_add_scaled_v(m_bvec3 a, m_bvec3 b, m_bvec3 c)
{
    return (m_bvec3){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1,
            a.v2 + b.v2 * c.v2
        }
    };
}

/** dst = a + b * c */
o_inline m_bvec3 bvec3_add_scaled(m_bvec3 a, m_bvec3 b, obyte c)
{
    return (m_bvec3){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c,
            a.v2 + b.v2 * c
        }
    };
}


/** dst = x % y as positive version for obyteegers */
o_inline m_bvec3 bvec3_mod(m_bvec3 x, obyte y)
{
    return (m_bvec3){
        {
            mb_mod(x.v0, y),
            mb_mod(x.v1, y),
            mb_mod(x.v2, y)
        }
    };
}

/** dst = x % y as positive version for obyteegers */
o_inline m_bvec3 bvec3_mod_v(m_bvec3 x, m_bvec3 y)
{
    return (m_bvec3){
        {
            mb_mod(x.v0, y.v0),
            mb_mod(x.v1, y.v1),
            mb_mod(x.v2, y.v2)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_bvec3 bvec3_min(m_bvec3 x, obyte y)
{
    return (m_bvec3){
        {
            mb_min(x.v0, y),
            mb_min(x.v1, y),
            mb_min(x.v2, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_bvec3 bvec3_min_v(m_bvec3 x, m_bvec3 y)
{
    return (m_bvec3){
        {
            mb_min(x.v0, y.v0),
            mb_min(x.v1, y.v1),
            mb_min(x.v2, y.v2)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_bvec3 bvec3_max(m_bvec3 x, obyte y)
{
    return (m_bvec3){
        {
            mb_max(x.v0, y),
            mb_max(x.v1, y),
            mb_max(x.v2, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_bvec3 bvec3_max_v(m_bvec3 x, m_bvec3 y)
{
    return (m_bvec3){
        {
            mb_max(x.v0, y.v0),
            mb_max(x.v1, y.v1),
            mb_max(x.v2, y.v2)
        }
    };
}


/** returns the index of the min v value */
o_inline obyte bvec3_min_index(m_bvec3 vec)
{
    obyte idx = vec.v0 <= vec.v1 ? 0 : 1;
    idx = vec.v[idx] <= vec.v2 ? idx : 2;
    return idx;
}


/** returns the index of the max v value */
o_inline obyte bvec3_max_index(m_bvec3 vec)
{
    obyte idx = vec.v0 >= vec.v1 ? 0 : 1;
    idx = vec.v[idx] >= vec.v2 ? idx : 2;
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_bvec3 bvec3_clamp(m_bvec3 x, obyte min, obyte max)
{
    return (m_bvec3){
        {
            mb_clamp(x.v0, min, max),
            mb_clamp(x.v1, min, max),
            mb_clamp(x.v2, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_bvec3 bvec3_clamp_v(m_bvec3 x, m_bvec3 min, m_bvec3 max)
{
    return (m_bvec3){
        {
            mb_clamp(x.v0, min.v0, max.v0),
            mb_clamp(x.v1, min.v1, max.v1),
            mb_clamp(x.v2, min.v2, max.v2)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_bvec3 bvec3_mix(m_bvec3 a, m_bvec3 b, float t)
{
    return (m_bvec3){
        {
            mb_mix(a.v0, b.v0, t),
            mb_mix(a.v1, b.v1, t),
            mb_mix(a.v2, b.v2, t)
        }
    };
}


/** returns v[0] + v[1] + ... + v[n-1] */
o_inline obyte bvec3_sum(m_bvec3 v)
{
    return v.v0 + v.v1 + v.v2;
}

#endif //M_VEC_BVEC3_H
