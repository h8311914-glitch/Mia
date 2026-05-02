#ifndef M_MAT_FLT_MATN_H
#define M_MAT_FLT_MATN_H

/**
 * @file mat/matn.h
 *
 * functions working on float matrices with a given size
 */



#include "m/sca/flt.h"

/** macro to cast a mattor into a float mattor */
#define matn_cast_into(dst, from, n) \
do {                                                                                 \
    for(int matn_cast_into_i_=0; matn_cast_into_i_<((n)*(n)); matn_cast_into_i_++) { \
        (dst)[matn_cast_into_i_] = (float) (from)[matn_cast_into_i_];                \
    }                                                                                \
} while(0)


/** dst = r==c ? 1 : 0 (identity) */
o_inline void matn_eye(float *dst, int n)
{
    for (int r = 0; r < n; r++) {
        for (int c = 0; c < n; c++) {
            dst[r * n + c] = r == c ? 1 : 0;
        }
    }
}

/** dst = m[row][:] */
o_inline void matn_get_row(float *dst_v, const float *m, int row, int n)
{
    assert(row >= 0 && row < n);
    for (int c = 0; c < n; c++) {
        dst_v[c] = m[c * n + row];
    }
}

/** dst = m[:][col] */
o_inline void matn_get_col(float *dst_v, const float *m, int col, int n)
{
    assert(col >= 0 && col < n);
    for (int r = 0; r < n; r++) {
        dst_v[r] = m[col * n + r];
    }
}

/** dst[row][:] = v */
o_inline void matn_set_row(float *dst, const float *v, int row, int n)
{
    assert(row >= 0 && row < n);
    for (int c = 0; c < n; c++) {
        dst[c * n + row] = v[c];
    }
}

/** dst[:][col] = v */
o_inline void matn_set_col(float *dst, const float *v, int col, int n)
{
    assert(col >= 0 && col < n);
    for (int r = 0; r < n; r++) {
        dst[col * n + r] = v[r];
    }
}

/** dst[row][:] = s */
o_inline void matn_set_row_s(float *dst, float s, int row, int n)
{
    assert(row >= 0 && row < n);
    for (int c = 0; c < n; c++) {
        dst[c * n + row] = s;
    }
}

/** dst[:][col] = s */
o_inline void matn_set_col_s(float *dst, float s, int col, int n)
{
    assert(col >= 0 && col < n);
    for (int r = 0; r < n; r++) {
        dst[col * n + r] = s;
    }
}

/** returns sum of diagonal from upper left to lower right */
o_inline float matn_trace(const float *mat, int n)
{
    float sum = 0;
    for (int i = 0; i < n; i++) {
        sum += mat[i * n + i];
    }
    return sum;
}

/** dst = m^t  (restrict data) */
o_inline void matn_transpose_no_alias(float *restrict dst, const float *restrict m, int n)
{
    for (int c = 0; c < n; c++) {
        for (int r = 0; r < n; r++) {
            dst[c * n + r] = m[r * n + c];
        }
    }
}

/** dst = m^t */
o_inline void matn_transpose(float *dst, const float *m, int n)
{
    assert(n <= M_MAX_SIZE);
    float tmp[M_MAX_SIZE * M_MAX_SIZE];
    matn_transpose_no_alias(tmp, m, n);
    for (int i = 0; i < n * n; i++) {
        dst[i] = tmp[i];
    }
}

/** dst = a @ b  (restrict data) */
o_inline void matn_mul_mat_no_alias(float *restrict dst, const float *restrict a,
                           const float *restrict b, int n)
{
    for (int c = 0; c < n; c++) {
        for (int r = 0; r < n; r++) {
            dst[c * n + r] = 0;
            for (int k = 0; k < n; k++) {
                dst[c * n + r] += a[k * n + r] * b[c * n + k];
            }
        }
    }
}

/** dst = a @ b */
o_inline void matn_mul_mat(float *dst, const float *a, const float *b, int n)
{
    assert(n <= M_MAX_SIZE);
    float tmp[M_MAX_SIZE * M_MAX_SIZE];
    matn_mul_mat_no_alias(tmp, a, b, n);
    for (int i = 0; i < n * n; i++) {
        dst[i] = tmp[i];
    }
}

/** dst = a @ b  (restrict data) */
o_inline void matn_mul_vec_no_alias(float *restrict dst_v, const float *restrict a,
                           const float *restrict b, int n)
{
    for (int r = 0; r < n; r++) {
        dst_v[r] = 0;
        for (int c = 0; c < n; c++) {
            dst_v[r] += a[c * n + r] * b[c];
        }
    }
}

/** dst = a @ b */
o_inline void matn_mul_vec(float *dst_v, const float *a, const float *b, int n)
{
    assert(n <= M_MAX_SIZE);
    float tmp[M_MAX_SIZE];
    matn_mul_vec_no_alias(tmp, a, b, n);
    for (int i = 0; i < n; i++) {
        dst_v[i] = tmp[i];
    }
}

/** dst = a @ b  (restrict data) */
o_inline void vecn_mul_mat_no_alias(float *restrict dst_v, const float *restrict a,
                           const float *restrict b, int n)
{
    for (int c = 0; c < n; c++) {
        dst_v[c] = 0;
        for (int r = 0; r < n; r++) {
            dst_v[c] += b[c * n + r] * a[r];
        }
    }
}

/** dst = a @ b */
o_inline void vecn_mul_mat(float *dst_v, const float *a, const float *b, int n)
{
    assert(n <= M_MAX_SIZE);
    float tmp[M_MAX_SIZE];
    vecn_mul_mat_no_alias(tmp, a, b, n);
    for (int i = 0; i < n; i++) {
        dst_v[i] = tmp[i];
    }
}


/** block<block_n*block_n> = m<n*n>[col:col+block_n, row:row+block_n] */
o_inline void matn_get_block(float *restrict dst_block, const float *restrict m, int row, int col, int block_n, int n)
{
    assert(row >= 0 && row + block_n <= n);
    assert(col >= 0 && col + block_n <= n);
    for (int c = 0; c < block_n; c++) {
        for (int r = 0; r < block_n; r++) {
            dst_block[c * block_n + r] = m[c * n + r];
        }
    }
}

/** dst<n*n>[col:col+block_n, row:row+block_n] = block<block_n*block_n> */
o_inline void matn_set_block(float *restrict dst, const float *restrict block, int row, int col, int block_n, int n)
{
    assert(row >= 0 && row + block_n <= n);
    assert(col >= 0 && col + block_n <= n);
    for (int c = 0; c < block_n; c++) {
        for (int r = 0; r < block_n; r++) {
            dst[c * n + r] = block[c * block_n + r];
        }
    }
}

#endif //M_MAT_FLT_MATN_H
