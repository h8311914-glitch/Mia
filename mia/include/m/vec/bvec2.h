#ifndef M_VEC_BVEC2_H
#define M_VEC_BVEC2_H

/**
 * @file vec/bvec2.h
 *
 * functions working with bvec2
 */


#include "m/sca/byte.h"
#include "m/m_types/byte.h"


/** dst = v0, v1, ... */
o_inline m_bvec2 bvec2_new(obyte v0, obyte v1)
{
    return (m_bvec2){{v0, v1}};
}

/** dst = (m_bvec2) v */
o_inline m_bvec2 bvec2_cast_double(const double *cast)
{
    return (m_bvec2){{cast[0], cast[1]}};
}

/** dst = (m_bvec2) v */
o_inline m_bvec2 bvec2_cast_float(const float *cast)
{
    return (m_bvec2){{cast[0], cast[1]}};
}

/** dst = (m_bvec2) v */
o_inline m_bvec2 bvec2_cast_int(const int *cast)
{
    return (m_bvec2){{cast[0], cast[1]}};
}


/** dst = v * 255 */
o_inline m_bvec2 bvec2_cast_float_1(const float *cast)
{
    return (m_bvec2){{cast[0] * 255, cast[1] * 255}};
}

/** dst = v * 255 */
o_inline m_bvec2 bvec2_cast_double_1(const double *cast)
{
    return (m_bvec2){{cast[0] * 255, cast[1] * 255}};
}


/** a == b */
o_inline bool bvec2_equals(m_bvec2 a, obyte b)
{
    return a.v0 == b
           && a.v1 == b;
}

/** a == b */
o_inline bool bvec2_equals_v(m_bvec2 a, m_bvec2 b)
{
    return a.v0 == b.v0
           && a.v1 == b.v1;
}

/** a ~= b */
o_inline bool bvec2_equals_eps(m_bvec2 a, obyte b, obyte eps)
{
    return mb_equals_eps(a.v0, b, eps)
           && mb_equals_eps(a.v1, b, eps);
}

/** a ~= b */
o_inline bool bvec2_equals_eps_v(m_bvec2 a, m_bvec2 b, obyte eps)
{
    return mb_equals_eps(a.v0, b.v0, eps)
           && mb_equals_eps(a.v1, b.v1, eps);
}


/** dst = s */
o_inline m_bvec2 bvec2_set(obyte s)
{
    return (m_bvec2){{s, s}};
}

/** dst = unit_x */
o_inline m_bvec2 bvec2_unit_x(void)
{
    return (m_bvec2){{1, 0}};
}


/** dst = unit_y */
o_inline m_bvec2 bvec2_unit_y(void)
{
    return (m_bvec2){{0, 1}};
}


/** dst = a + b */
o_inline m_bvec2 bvec2_add_v(m_bvec2 a, m_bvec2 b)
{
    return (m_bvec2){
        {
            a.v0 + b.v0,
            a.v1 + b.v1
        }
    };
}


/** dst = a + b */
o_inline m_bvec2 bvec2_add(m_bvec2 a, obyte b)
{
    return (m_bvec2){
        {
            a.v0 + b,
            a.v1 + b
        }
    };
}


/** dst = a - b */
o_inline m_bvec2 bvec2_sub_v(m_bvec2 a, m_bvec2 b)
{
    return (m_bvec2){
        {
            a.v0 - b.v0,
            a.v1 - b.v1
        }
    };
}


/** dst = a - b */
o_inline m_bvec2 bvec2_sub(m_bvec2 a, obyte b)
{
    return (m_bvec2){
        {
            a.v0 - b,
            a.v1 - b
        }
    };
}


/** dst = a * b */
o_inline m_bvec2 bvec2_scale_v(m_bvec2 a, m_bvec2 b)
{
    return (m_bvec2){
        {
            a.v0 * b.v0,
            a.v1 * b.v1
        }
    };
}


/** dst = a * b */
o_inline m_bvec2 bvec2_scale(m_bvec2 a, obyte b)
{
    return (m_bvec2){
        {
            a.v0 * b,
            a.v1 * b
        }
    };
}


/** dst = a / b */
o_inline m_bvec2 bvec2_div_v(m_bvec2 a, m_bvec2 b)
{
    return (m_bvec2){
        {
            a.v0 / b.v0,
            a.v1 / b.v1
        }
    };
}


/** dst = a / b */
o_inline m_bvec2 bvec2_div(m_bvec2 a, obyte b)
{
    return (m_bvec2){
        {
            a.v0 / b,
            a.v1 / b
        }
    };
}

/** dst = a + b * c */
o_inline m_bvec2 bvec2_add_scaled_v(m_bvec2 a, m_bvec2 b, m_bvec2 c)
{
    return (m_bvec2){
        {
            a.v0 + b.v0 * c.v0,
            a.v1 + b.v1 * c.v1
        }
    };
}

/** dst = a + b * c */
o_inline m_bvec2 bvec2_add_scaled(m_bvec2 a, m_bvec2 b, obyte c)
{
    return (m_bvec2){
        {
            a.v0 + b.v0 * c,
            a.v1 + b.v1 * c
        }
    };
}


/** dst = x % y as positive version for obyteegers */
o_inline m_bvec2 bvec2_mod(m_bvec2 x, obyte y)
{
    return (m_bvec2){
        {
            mb_mod(x.v0, y),
            mb_mod(x.v1, y)
        }
    };
}

/** dst = x % y as positive version for obyteegers */
o_inline m_bvec2 bvec2_mod_v(m_bvec2 x, m_bvec2 y)
{
    return (m_bvec2){
        {
            mb_mod(x.v0, y.v0),
            mb_mod(x.v1, y.v1)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_bvec2 bvec2_min(m_bvec2 x, obyte y)
{
    return (m_bvec2){
        {
            mb_min(x.v0, y),
            mb_min(x.v1, y)
        }
    };
}


/** dst = a < b ? a : b */
o_inline m_bvec2 bvec2_min_v(m_bvec2 x, m_bvec2 y)
{
    return (m_bvec2){
        {
            mb_min(x.v0, y.v0),
            mb_min(x.v1, y.v1)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_bvec2 bvec2_max(m_bvec2 x, obyte y)
{
    return (m_bvec2){
        {
            mb_max(x.v0, y),
            mb_max(x.v1, y)
        }
    };
}


/** dst = a > b ? a : b */
o_inline m_bvec2 bvec2_max_v(m_bvec2 x, m_bvec2 y)
{
    return (m_bvec2){
        {
            mb_max(x.v0, y.v0),
            mb_max(x.v1, y.v1)
        }
    };
}


/** returns the index of the min v value */
o_inline obyte bvec2_min_index(m_bvec2 vec)
{
    return vec.v0 <= vec.v1 ? 0 : 1;
}


/** returns the index of the max v value */
o_inline obyte bvec2_max_index(m_bvec2 vec)
{
    return vec.v0 >= vec.v1 ? 0 : 1;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_bvec2 bvec2_clamp(m_bvec2 x, obyte min, obyte max)
{
    return (m_bvec2){
        {
            mb_clamp(x.v0, min, max),
            mb_clamp(x.v1, min, max)
        }
    };
}


/** dst = x < min ? min : (x > max ? max : x) */
o_inline m_bvec2 bvec2_clamp_v(m_bvec2 x, m_bvec2 min, m_bvec2 max)
{
    return (m_bvec2){
        {
            mb_clamp(x.v0, min.v0, max.v0),
            mb_clamp(x.v1, min.v1, max.v1)
        }
    };
}


/** dst = a * (1-t) + b * t */
o_inline m_bvec2 bvec2_mix(m_bvec2 a, m_bvec2 b, float t)
{
    return (m_bvec2){
        {
            mb_mix(a.v0, b.v0, t),
            mb_mix(a.v1, b.v1, t)
        }
    };
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline obyte bvec2_sum(m_bvec2 v)
{
    return v.v0 + v.v1;
}

#endif //M_VEC_BVEC2_H
