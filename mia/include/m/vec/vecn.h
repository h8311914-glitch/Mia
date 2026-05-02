#ifndef M_VEC_VECN_H
#define M_VEC_VECN_H

/**
 * @file vec/vecn.h
 *
 * functions working on float vectors with a given size
 */



#include "m/sca/flt.h"

/** macro to cast a vector into a float vector */
#define vecn_cast_into(dst, from, n) \
do {                                                                           \
    for(int vecn_cast_into_i_=0; vecn_cast_into_i_<(n); vecn_cast_into_i_++) { \
        (dst)[vecn_cast_into_i_] = (float) (from)[vecn_cast_into_i_];          \
    }                                                                          \
} while(0)


/** dst = v / 255 */
o_inline void vecn_cast_byte_1(float *dst_v, const obyte *vec, int n)
{
    for (int i = 0; i < n; i++) {
        dst_v[i] = (float) vec[i] / (float) 255;
    }
}


/** a == b */
o_inline bool vecn_equals(const float *a, float b, int n)
{
    for (int i = 0; i < n; i++) {
        if (a[i] != b) {
            return false;
        }
    }
    return true;
}

/** a == b */
o_inline bool vecn_equals_v(const float *a, const float *b, int n)
{
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

/** a ~= b */
o_inline bool vecn_equals_eps(const float *a, float b, float eps, int n)
{
    for (int i = 0; i < n; i++) {
        if (m_abs(a[i] - b) > eps) {
            return false;
        }
    }
    return true;
}

/** a ~= b */
o_inline bool vecn_equals_eps_v(const float *a, const float *b, float eps, int n)
{
    for (int i = 0; i < n; i++) {
        if (m_abs(a[i] - b[i]) > eps) {
            return false;
        }
    }
    return true;
}

/** dst = v */
o_inline void vecn_clone(float *dst, const float *v, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = v[i];
    }
}

/** dst = s */
o_inline void vecn_set(float *dst, float s, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = s;
    }
}

/** dst = unit_x */
o_inline void vecn_unit_x(float *dst, int n)
{
    vecn_set(dst, 0, n);
    dst[0] = 1;
}

/** dst = unit_y */
o_inline void vecn_unit_y(float *dst, int n)
{
    assert(n >= 2 && "m vec*_unit_z");
    vecn_set(dst, 0, n);
    dst[1] = 1;
}

/** assert(n>=3); dst = unit_z */
o_inline void vecn_unit_z(float *dst, int n)
{
    assert(n >= 3 && "m vec*_unit_z");
    vecn_set(dst, 0, n);
    dst[2] = 1;
}

/** assert(n>=4); dst = unit_w */
o_inline void vecn_unit_w(float *dst, int n)
{
    assert(n >= 4 && "m vec*_unit_w");
    vecn_set(dst, 0, n);
    dst[3] = 1;
}

/** dst = -v */
o_inline void vecn_neg(float *dst, const float *v, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = -v[i];
    }
}

/** dst = a + b */
o_inline void vecn_add_v(float *dst, const float *a, const float *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b[i];
    }
}

/** dst = a + b */
o_inline void vecn_add(float *dst, const float *a, float b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b;
    }
}

/** dst = a - b */
o_inline void vecn_sub_v(float *dst, const float *a, const float *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] - b[i];
    }
}

/** dst = a - b */
o_inline void vecn_sub(float *dst, const float *a, float b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] - b;
    }
}

/** dst = a * b */
o_inline void vecn_scale_v(float *dst, const float *a, const float *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] * b[i];
    }
}

/** dst = a * b */
o_inline void vecn_scale(float *dst, const float *a, float b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] * b;
    }
}

/** dst = a / b */
o_inline void vecn_div_v(float *dst, const float *a, const float *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] / b[i];
    }
}

/** dst = a / b */
o_inline void vecn_div(float *dst, const float *a, float b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] / b;
    }
}

/** dst = a + b * c */
o_inline void vecn_add_scaled_v(float *dst, const float *a, const float *b, const float *c, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b[i] * c[i];
    }
}

/** dst = a + b * c */
o_inline void vecn_add_scaled(float *dst, const float *a, const float *b, float c, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = a[i] + b[i] * c;
    }
}

/** dst = a * M_PI / 180 */
o_inline void vecn_radians(float *dst, const float *deg, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_radians(deg[i]);
    }
}

/** dst = a * 180 / M_PI */
o_inline void vecn_degrees(float *dst, const float *rad, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_degrees(rad[i]);
    }
}

/** dst = abs(x) */
o_inline void vecn_abs(float *dst, const float *x, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_abs(x[i]);
    }
}

/** dst = x > 0 ? 1 : (x < 0 ? -1 : 0) */
o_inline void vecn_sign(float *dst, const float *x, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_sign(x[i]);
    }
}

/** dst = round(x) */
o_inline void vecn_round(float *dst, const float *x, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_round(x[i]);
    }
}

/** dst = floor(x) */
o_inline void vecn_floor(float *dst, const float *x, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_floor(x[i]);
    }
}

/** dst = ceil(x) */
o_inline void vecn_ceil(float *dst, const float *x, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_ceil(x[i]);
    }
}

/** dst = x - floor(x) */
o_inline void vecn_fract(float *dst, const float *x, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_fract(x[i]);
    }
}

/** dst = x - y * floor(x/y) */
o_inline void vecn_mod(float *dst, const float *x, float y, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_mod(x[i], y);
    }
}

/** dst = x - y * floor(x/y) */
o_inline void vecn_mod_v(float *dst, const float *x, const float *y, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_mod(x[i], y[i]);
    }
}

/** dst = a < b ? a : b */
o_inline void vecn_min(float *dst, const float *a, float b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_min(a[i], b);
    }
}

/** dst = a < b ? a : b */
o_inline void vecn_min_v(float *dst, const float *a, const float *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_min(a[i], b[i]);
    }
}

/** dst = a > b ? a : b */
o_inline void vecn_max(float *dst, const float *a, float b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_max(a[i], b);
    }
}

/** dst = a > b ? a : b */
o_inline void vecn_max_v(float *dst, const float *a, const float *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_max(a[i], b[i]);
    }
}

/** returns the index of the min v value */
o_inline int vecn_min_index(const float *v, int n)
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
o_inline int vecn_max_index(const float *v, int n)
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
o_inline void vecn_clamp(float *dst, const float *x, float min, float max, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_clamp(x[i], min, max);
    }
}

/** dst = x < min ? min : (x > max ? max : x) */
o_inline void vecn_clamp_v(float *dst, const float *x, const float *min, const float *max, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_clamp(x[i], min[i], max[i]);
    }
}

/** dst = a * (1-t) + b * t */
o_inline void vecn_mix(float *dst, const float *a, const float *b, float t, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_mix(a[i], b[i], t);
    }
}

/** dst = a * (1-t) + b * t */
o_inline void vecn_mix_v(float *dst, const float *a, const float *b, const float *t, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_mix(a[i], b[i], t[i]);
    }
}

/** returns (double) v[0] + v[1] + ... + v[n-1] */
o_inline double vecn_sum_d(const float *v, int n)
{
    double sum = 0;
    for (int i = 0; i < n; i++) {
        sum += v[i];
    }
    return sum;
}

/** returns v[0] + v[1] + ... + v[n-1] */
o_inline float vecn_sum(const float *v, int n)
{
    return (float) vecn_sum_d(v, n);
}


/** returns a dot b */
o_inline float vecn_dot(const float *a, const float *b, int n)
{
    float dot = 0;
    for (int i = 0; i < n; i++) {
        dot += a[i] * b[i];
    }
    return dot;
}

/** assert(n>=3) ; dst = a x b , dst.w... = 0 */
o_inline void vecn_cross(float *dst, const float *a, const float *b, int n)
{
    assert(n >= 3 && "m vec*_cross only in 3D");
    dst[0] = a[1] * b[2] - a[2] * b[1];
    dst[1] = a[2] * b[0] - a[0] * b[2];
    dst[2] = a[0] * b[1] - a[1] * b[0];
    for (int i = 3; i < n; i++)
        dst[i] = 0;
}

/** returns ||v||_2 */
o_inline float vecn_norm(const float *v, int n)
{
    return m_sqrt(vecn_dot(v, v, n));
}

/** returns ||v||_p */
o_inline float vecn_norm_p(const float *v, float p, int n)
{
    float sum = 0;
    for (int i = 0; i < n; i++) {
        sum += m_pow(m_abs(v[i]), p);
    }
    return m_pow(sum, (float) 1 / p);
}

/** returns ||v||_1 */
o_inline float vecn_norm_1(const float *v, int n)
{
    float sum = 0;
    for (int i = 0; i < n; i++) {
        sum += m_abs(v[i]);
    }
    return sum;
}

/** returns ||v||_inf */
o_inline float vecn_norm_inf(const float *v, int n)
{
    float max = 0;
    for (int i = 0; i < n; i++) {
        if (max < m_abs(v[i])) {
            max = m_abs(v[i]);
        }
    }
    return max;
}

/** dst = v / norm(v) */
o_inline void vecn_normalize_unsafe(float *dst, const float *v, int n)
{
    vecn_scale(dst, v, (float) 1 / vecn_norm(v, n), n);
}

/** dst = v / (norm(v) > 0 ? norm(v) : 1) */
o_inline void vecn_normalize(float *dst, const float *v, int n)
{
    float norm = vecn_norm(v, n);
    vecn_scale(dst, v, (float) 1 / (norm > (float) 0 ? norm : (float) 1), n);
}

/** dst = normalize(cross(a, b)) */
o_inline void vecn_cross_normalized(float *dst, const float *a, const float *b, int n)
{
    vecn_cross(dst, a, b, n);
    vecn_normalize(dst, dst, n);
}

/** returns length of a vector, see vecn_norm. Just here to match glsl */
o_inline float vecn_length(const float *v, int n)
{
    return vecn_norm(v, n);
}

/** returns norm(b-a) */
o_inline float vecn_distance(const float *a, const float *b, int n)
{
    assert(n <= M_MAX_SIZE);
    float tmp[M_MAX_SIZE];
    vecn_sub_v(tmp, b, a, n);
    return vecn_norm(tmp, n);
}

/** returns dot(b-a) */
o_inline float vecn_sqr_distance(const float *a, const float *b, int n)
{
    assert(n <= M_MAX_SIZE);
    float tmp[M_MAX_SIZE];
    vecn_sub_v(tmp, b, a, n);
    return vecn_dot(tmp, tmp, n);
}

#endif //M_VEC_VECN_H
