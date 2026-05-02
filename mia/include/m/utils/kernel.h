#ifndef M_UTILS_FLT_KERNEL_H
#define M_UTILS_FLT_KERNEL_H

/**
 * @file utils/kernel.h
 *
 * setup some basic convolution kernels
 */

#include "m/vec/vecn.h"
#include "m/vec/vec2.h"
#include "m/types/flt.h"

/** dst = normalize(src) to sum=normalize */
o_inline void m_kernel_normalize_to(float *dst, const float *src, float normalize, int n)
{
    float sum = vecn_sum(src, n);
    vecn_scale(dst, src, normalize / sum, n);
}

/** mute: dst = 0 */
o_inline void m_kernel_clear(float *dst, int n)
{
    vecn_set(dst, 0, n);
}

/** identity: dst = 0; dst[n-1] = 1 */
o_inline void m_kernel_eye(float *dst, int n)
{
    vecn_set(dst, 0, n - 1);
    dst[n - 1] = 1;
}

/**
 * blur: dst = 1;
 * allows for subwindows located at end with fractions (size);
 * normalized used if >0
 */
o_inline void m_kernel_blur(float *dst, float size, float normalize, int n)
{
    if (size >= n || size < 0) {
        vecn_set(dst, 1.0, n);
    } else {
        int full = m_floor(size);
        float fract = m_fract(size);
        assert(full < n);
        int clear = n - full - 1;
        if (clear > 0) {
            vecn_set(dst, 0, clear);
        }
        dst[clear] = fract;
        if (full > 0) {
            vecn_set(dst + clear + 1, 1, full);
        }
    }
    if (normalize > 0) {
        m_kernel_normalize_to(dst, dst, normalize, n);
    }
}

/**
 * blur high: dst[end]=1.0 dst=-1/n;
 * allows for subwindows located at end with fractions (size)
 * normalized used if >0
 */
o_inline void m_kernel_blur_high(float *dst, float size, float normalize, int n)
{
    m_kernel_blur(dst, size, normalize, n);
    vecn_neg(dst, dst, n);
    dst[n - 1] += 1.0f;
}

/**
 * gauss: dst = gaussian curve;
 * pass sigma<=0 to auto scale
 * normalized used if >0
 */
o_inline void m_kernel_gauss(float *dst, float sigma, float normalize, int n)
{
    sigma = sigma > 0 ? sigma : ((n - 1) * 0.5 - 1) * 0.3 + 0.8;

    float scale = -0.5 / (sigma * sigma);

    int center = n / 2;

    for (int k = 0; k < n; k++) {
        float x = k - center;
        float value = m_exp(scale * x * x);
        dst[k] = m_clamp(value, 0, 1);
    }

    if (normalize > 0) {
        m_kernel_normalize_to(dst, dst, normalize, n);
    }
}

/**
 * gauss high: dst[center]=1.0;
 * dst=-gaussian curve;
 * pass sigma<=0 to auto scale
 * normalized used if >0
 */
o_inline void m_kernel_gauss_high(float *dst, float sigma, float normalize, int n)
{
    m_kernel_gauss(dst, sigma, normalize, n);
    vecn_neg(dst, dst, n);
    dst[n / 2] += 1.0f;
}

/**
 * gauss band: dst[center]=1.0;
 * dst=gauss_narrow - gauss wide
 * pass sigma<=0 to auto scale
 * normalized used if >0
 * @note needs tmp_kernel_space of size n to calc (avoids m to allocate... :D)
 */
o_inline void m_kernel_gauss_band(float *dst, float *tmp_kernel_space, float simga_narrow, float sigma_wide, int n)
{
    m_kernel_gauss(tmp_kernel_space, simga_narrow, 1, n);
    m_kernel_gauss(dst, sigma_wide, 1, n);
    vecn_sub_v(dst, tmp_kernel_space, dst, n);
}

/**
 * Fills a 2d kernel thats filled with set;
 * Row Major packed ordering;
 * if plus, edges are cleared to 0
 * normalized used if >0
 */
o_inline void m_kernel_2d(float *dst, float set, bool plus, float normalize, int cols, int rows)
{
    vecn_set(dst, set, cols * rows);
    if (plus) {
        dst[0] = 0.0f;
        dst[cols - 1] = 0.0f;
        dst[(rows - 1) * cols] = 0.0f;
        dst[rows * cols - 1] = 0.0f;
    }

    if (normalize > 0.0f) {
        m_kernel_normalize_to(dst, dst, normalize, cols * rows);
    }
}

/**
 * Fills a 2d kernel thats filled with gauss curve in 2d;
 * Row Major packed ordering;
 * if plus, edges are cleared to 0
 * normalized used if >0
 */
o_inline void m_kernel_2d_gauss(float *dst, m_vec2 sigma, float normalize, int cols, int rows)
{
    sigma.x = sigma.x > 0 ? sigma.x : ((cols - 1) * 0.5 - 1) * 0.3 + 0.8;
    sigma.y = sigma.y > 0 ? sigma.y : ((rows - 1) * 0.5 - 1) * 0.3 + 0.8;

    float scale_x = -0.5 / (sigma.x * sigma.x);
    float scale_y = -0.5 / (sigma.y * sigma.y);

    int center_r = rows / 2;
    int center_c = cols / 2;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            float x = c - center_c;
            float y = r - center_r;
            float t_x = m_exp(scale_x * x * x);
            float t_y = m_exp(scale_y * y * y);

            float value = t_x * t_y;

            dst[r * cols + c] = m_clamp(value, 0, 1);
        }
    }

    if (normalize > 0.0f) {
        m_kernel_normalize_to(dst, dst, normalize, cols * rows);
    }
}

/**
 * Fills a 2d kernel thats filled with an ellipse with a hard border (either 0 or 1)
 * Row Major packed ordering;
 * norm_radius in (0:1.414], if <0 its gonne be 1.05 (which fits well to touch the border up to at least 64x64)
 * normalized used if >0
 */
o_inline void m_kernel_2d_ellipse(float *dst, float norm_radius, float normalize, int cols, int rows)
{
    norm_radius = norm_radius > 0 ? norm_radius : 1.05;
    m_vec2 radius = {{(cols-1)/2.0f, (rows-1)/2.0f}};
    for (int r=0; r<rows; r++) {
        for (int c=0; c<cols; c++) {
            m_vec2 center_offset = vec2_sub_v((m_vec2){{c, r}}, radius);
            m_vec2 norm = vec2_div_v(center_offset, radius);
            float len = vec2_norm(norm);
            if (len<=norm_radius) {
                dst[r*cols+c] = 1.0f;
            } else {
                dst[r*cols+c] = 0.0f;
            }
        }
    }
    if (normalize > 0.0f) {
        m_kernel_normalize_to(dst, dst, normalize, cols * rows);
    }
}


#endif //M_UTILS_FLT_KERNEL_H
