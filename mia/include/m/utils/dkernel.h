#ifndef M_UTILS_DBL_KERNEL_H
#define M_UTILS_DBL_KERNEL_H

/**
 * @file utils/kernel.h
 *
 * setup some basic convolution kernels
 */

#include "m/vec/dvecn.h"
#include "m/vec/dvec2.h"
#include "m/types/flt.h"

/** dst = normalize(src) to sum=normalize */
o_inline void md_kernel_normalize_to(double *dst, const double *src, double normalize, int n)
{
    double sum = dvecn_sum(src, n);
    dvecn_scale(dst, src, normalize / sum, n);
}

/** mute: dst = 0 */
o_inline void md_kernel_clear(double *dst, int n)
{
    dvecn_set(dst, 0, n);
}

/** identity: dst = 0; dst[n-1] = 1 */
o_inline void md_kernel_eye(double *dst, int n)
{
    dvecn_set(dst, 0, n - 1);
    dst[n - 1] = 1;
}

/**
 * blur: dst = 1;
 * allows for subwindows located at end with fractions (size);
 * normalized used if >0
 */
o_inline void md_kernel_blur(double *dst, double size, double normalize, int n)
{
    if (size >= n || size < 0) {
        dvecn_set(dst, 1.0, n);
    } else {
        int full = md_floor(size);
        double fract = md_fract(size);
        assert(full < n);
        int clear = n - full - 1;
        if (clear > 0) {
            dvecn_set(dst, 0, clear);
        }
        dst[clear] = fract;
        if (full > 0) {
            dvecn_set(dst + clear + 1, 1, full);
        }
    }
    if (normalize > 0) {
        md_kernel_normalize_to(dst, dst, normalize, n);
    }
}

/**
 * blur high: dst[end]=1.0 dst=-1/n;
 * allows for subwindows located at end with fractions (size)
 * normalized used if >0
 */
o_inline void md_kernel_blur_high(double *dst, double size, double normalize, int n)
{
    md_kernel_blur(dst, size, normalize, n);
    dvecn_neg(dst, dst, n);
    dst[n - 1] += 1.0;
}

/**
 * gauss: dst = gaussian curve;
 * pass sigma<=0 to auto scale
 * normalized used if >0
 */
o_inline void md_kernel_gauss(double *dst, double sigma, double normalize, int n)
{
    sigma = sigma > 0 ? sigma : ((n - 1) * 0.5 - 1) * 0.3 + 0.8;

    double scale = -0.5 / (sigma * sigma);

    int center = n / 2;

    for (int k = 0; k < n; k++) {
        double x = k - center;
        double value = md_exp(scale * x * x);
        dst[k] = md_clamp(value, 0, 1);
    }

    if (normalize > 0) {
        md_kernel_normalize_to(dst, dst, normalize, n);
    }
}

/**
 * gauss high: dst[center]=1.0;
 * dst=-gaussian curve;
 * pass sigma<=0 to auto scale
 * normalized used if >0
 */
o_inline void md_kernel_gauss_high(double *dst, double sigma, double normalize, int n)
{
    md_kernel_gauss(dst, sigma, normalize, n);
    dvecn_neg(dst, dst, n);
    dst[n / 2] += 1.0;
}

/**
 * gauss band: dst[center]=1.0;
 * dst=gauss_narrow - gauss wide
 * pass sigma<=0 to auto scale
 * normalized used if >0
 * @note needs tmp_kernel_space of size n to calc (avoids m to allocate... :D)
 */
o_inline void md_kernel_gauss_band(double *dst, double *tmp_kernel_space, double simga_narrow, double sigma_wide, int n)
{
    md_kernel_gauss(tmp_kernel_space, simga_narrow, 1, n);
    md_kernel_gauss(dst, sigma_wide, 1, n);
    dvecn_sub_v(dst, tmp_kernel_space, dst, n);
}

/**
 * Fills a 2d kernel thats filled with set;
 * Row Major packed ordering;
 * if plus, edges are cleared to 0
 * normalized used if >0
 */
o_inline void md_kernel_2d(double *dst, double set, bool plus, double normalize, int cols, int rows)
{
    dvecn_set(dst, set, cols * rows);
    if (plus) {
        dst[0] = 0.0;
        dst[cols - 1] = 0.0;
        dst[(rows - 1) * cols] = 0.0;
        dst[rows * cols - 1] = 0.0;
    }

    if (normalize > 0.0) {
        md_kernel_normalize_to(dst, dst, normalize, cols * rows);
    }
}

/**
 * Fills a 2d kernel thats filled with gauss curve in 2d;
 * Row Major packed ordering;
 * if plus, edges are cleared to 0
 * normalized used if >0
 */
o_inline void md_kernel_2d_gauss(double *dst, m_dvec2 sigma, double normalize, int cols, int rows)
{
    sigma.x = sigma.x > 0 ? sigma.x : ((cols - 1) * 0.5 - 1) * 0.3 + 0.8;
    sigma.y = sigma.y > 0 ? sigma.y : ((rows - 1) * 0.5 - 1) * 0.3 + 0.8;

    double scale_x = -0.5 / (sigma.x * sigma.x);
    double scale_y = -0.5 / (sigma.y * sigma.y);

    int center_r = rows / 2;
    int center_c = cols / 2;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            double x = c - center_c;
            double y = r - center_r;
            double t_x = md_exp(scale_x * x * x);
            double t_y = md_exp(scale_y * y * y);

            double value = t_x * t_y;

            dst[r * cols + c] = md_clamp(value, 0, 1);
        }
    }

    if (normalize > 0.0) {
        md_kernel_normalize_to(dst, dst, normalize, cols * rows);
    }
}

/**
 * Fills a 2d kernel thats filled with an ellipse with a hard border (either 0 or 1)
 * Row Major packed ordering;
 * norm_radius in (0:1.414], if <0 its gonne be 1.05 (which fits well to touch the border up to at least 64x64)
 * normalized used if >0
 */
o_inline void md_kernel_2d_ellipse(double *dst, double norm_radius, double normalize, int cols, int rows)
{
    norm_radius = norm_radius > 0 ? norm_radius : 1.05;
    m_dvec2 radius = {{(cols-1)/2.0, (rows-1)/2.0}};
    for (int r=0; r<rows; r++) {
        for (int c=0; c<cols; c++) {
            m_dvec2 center_offset = dvec2_sub_v((m_dvec2){{c, r}}, radius);
            m_dvec2 norm = dvec2_div_v(center_offset, radius);
            double len = dvec2_norm(norm);
            if (len<=norm_radius) {
                dst[r*cols+c] = 1.0;
            } else {
                dst[r*cols+c] = 0.0;
            }
        }
    }
    if (normalize > 0.0) {
        md_kernel_normalize_to(dst, dst, normalize, cols * rows);
    }
}

#endif //M_UTILS_DBL_KERNEL_H
