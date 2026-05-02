#ifndef M_VEC_BVECN_H
#define M_VEC_BVECN_H

/**
 * @file vec/bvecn.h
 *
 * functions working on obyte bvectors with a given size
 */

#include "m/sca/byte.h"

/** macro to cast a bvector into a obyte bvector */
#define bvecn_cast_into(dst, from, n) \
do {                                                                           \
    for(int bvecn_cast_into_i_=0; bvecn_cast_into_i_<(n); bvecn_cast_into_i_++) { \
        (dst)[bvecn_cast_into_i_] = (obyte) (from)[bvecn_cast_into_i_];          \
    }                                                                          \
} while(0)


/** dst = v * 255 */
o_inline void bvecn_cast_double_1(obyte *dst_v, const double *vec, int n)
{
    for (int i = 0; i < n; i++) {
        dst_v[i] = (obyte) (vec[i] * 255);
    }
}


/** dst = v * 255 */
o_inline void bvecn_cast_float_1(obyte *dst_v, const float *vec, int n)
{
    for (int i = 0; i < n; i++) {
        dst_v[i] = (obyte) (vec[i] * 255);
    }
}


/** a == b */
o_inline bool bvecn_equals(const obyte *a, obyte b, int n)
{
    for (int i = 0; i < n; i++) {
        if (a[i] != b) {
            return false;
        }
    }
    return true;
}

/** a == b */
o_inline bool bvecn_equals_v(const obyte *a, const obyte *b, int n)
{
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

/** a ~= b */
o_inline bool bvecn_equals_eps(const obyte *a, obyte b, obyte eps, int n)
{
    for (int i = 0; i < n; i++) {
        if (o_abs((int) a[i] - (int) b) > eps) {
            return false;
        }
    }
    return true;
}

/** a ~= b */
o_inline bool bvecn_equals_eps_v(const obyte *a, const obyte *b, obyte eps, int n)
{
    for (int i = 0; i < n; i++) {
        if (o_abs((int) a[i] - (int) b[i]) > eps) {
            return false;
        }
    }
    return true;
}

/** dst = v */
o_inline void bvecn_clone(obyte *dst, const obyte *v, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = v[i];
    }
}

/** dst = s */
o_inline void bvecn_set(obyte *dst, obyte s, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = s;
    }
}

/** dst = unit_x */
o_inline void bvecn_unit_x(obyte *dst, int n)
{
    bvecn_set(dst, 0, n);
    dst[0] = 1;
}

/** dst = unit_y */
o_inline void bvecn_unit_y(obyte *dst, int n)
{
    assert(n >= 2 && "m bvec*_unit_z");
    bvecn_set(dst, 0, n);
    dst[1] = 1;
}

/** assert(n>=3); dst = unit_z */
o_inline void bvecn_unit_z(obyte *dst, int n)
{
    assert(n >= 3 && "m bvec*_unit_z");
    bvecn_set(dst, 0, n);
    dst[2] = 1;
}

/** assert(n>=4); dst = unit_w */
o_inline void bvecn_unit_w(obyte *dst, int n)
{
    assert(n >= 4 && "m bvec*_unit_w");
    bvecn_set(dst, 0, n);
    dst[3] = 1;
}

/** dst = a + b */
o_inline void bvecn_add_v(obyte *dst, const obyte *a, const obyte *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b[i];
    }
}

/** dst = a + b */
o_inline void bvecn_add(obyte *dst, const obyte *a, obyte b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b;
    }
}

/** dst = a - b */
o_inline void bvecn_sub_v(obyte *dst, const obyte *a, const obyte *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] - b[i];
    }
}

/** dst = a - b */
o_inline void bvecn_sub(obyte *dst, const obyte *a, obyte b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] - b;
    }
}

/** dst = a * b */
o_inline void bvecn_scale_v(obyte *dst, const obyte *a, const obyte *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] * b[i];
    }
}

/** dst = a * b */
o_inline void bvecn_scale(obyte *dst, const obyte *a, obyte b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] * b;
    }
}

/** dst = a / b */
o_inline void bvecn_div_v(obyte *dst, const obyte *a, const obyte *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] / b[i];
    }
}

/** dst = a / b */
o_inline void bvecn_div(obyte *dst, const obyte *a, obyte b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] / b;
    }
}

/** dst = a + b * c */
o_inline void bvecn_add_scaled_v(obyte *dst, const obyte *a, const obyte *b, const obyte *c, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b[i] * c[i];
    }
}

/** dst = a + b * c */
o_inline void bvecn_add_scaled(obyte *dst, const obyte *a, const obyte *b, obyte c, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b[i] * c;
    }
}

/** dst = x % y as positive version for integers */
o_inline void bvecn_mod(obyte *dst, const obyte *x, obyte y, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_mod(x[i], y);
    }
}

/** dst = x % y as positive version for integers */
o_inline void bvecn_mod_v(obyte *dst, const obyte *x, const obyte *y, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_mod(x[i], y[i]);
    }
}

/** dst = a < b ? a : b */
o_inline void bvecn_min(obyte *dst, const obyte *a, obyte b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_min(a[i], b);
    }
}

/** dst = a < b ? a : b */
o_inline void bvecn_min_v(obyte *dst, const obyte *a, const obyte *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_min(a[i], b[i]);
    }
}

/** dst = a > b ? a : b */
o_inline void bvecn_max(obyte *dst, const obyte *a, obyte b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_max(a[i], b);
    }
}

/** dst = a > b ? a : b */
o_inline void bvecn_max_v(obyte *dst, const obyte *a, const obyte *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_max(a[i], b[i]);
    }
}

/** returns the index of the min v value */
o_inline int bvecn_min_index(const obyte *v, int n)
{
    int idx = 0;
    for (int i = 1; i < n; i++) {
        if (v[i] < v[idx]) {
            idx = i;
        }
    }
    return idx;
}

/** returns the index of the max v value */
o_inline int bvecn_max_index(const obyte *v, int n)
{
    int idx = 0;
    for (int i = 1; i < n; i++) {
        if (v[i] > v[idx]) {
            idx = i;
        }
    }
    return idx;
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline void bvecn_clamp(obyte *dst, const obyte *x, obyte min, obyte max, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_clamp(x[i], min, max);
    }
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline void bvecn_clamp_v(obyte *dst, const obyte *x, const obyte *min, const obyte *max, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_clamp(x[i], min[i], max[i]);
    }
}

/** dst = a * (1-t) + b * t */
o_inline void bvecn_mix(obyte *dst, const obyte *a, const obyte *b, float t, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_mix(a[i], b[i], t);
    }
}

/** dst = a * (1-t) + b * t */
o_inline void bvecn_mix_v(int *dst, const obyte *a, const obyte *b, const float *t, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = mb_mix(a[i], b[i], t[i]);
    }
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline int bvecn_sum(const obyte *v, int n)
{
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += v[i];
    }
    return sum;
}

#endif //M_VEC_BVECN_H
