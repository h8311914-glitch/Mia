#ifndef M_UTILS_DBL_DRANDOM
#define M_UTILS_DBL_DRANDOM

/**
 * @file utils/random.h
 *
 * Random math for the types
 */


#ifndef M_RANDOM_FUNCTION
#define M_RANDOM_FUNCTION o_rand
#endif

#ifndef M_RANDOM_FUNCTION_MAX
#define M_RANDOM_FUNCTION_MAX o_rand_MAX
#endif

#include "m/m_types/dbl.h"
#include "m/sca/dbl.h"

/** dst = rand() [0:1] */
o_inline double md_random(void)
{
    return (double) 1 * M_RANDOM_FUNCTION() / (double) M_RANDOM_FUNCTION_MAX;
}

/** dst = rand() [a:b] */
o_inline double md_random_range(double a, double b)
{
    return a + (b - a) * md_random();
}

/** dst = mean +- amplitude * rand() */
o_inline double md_random_noise(double mean, double amplitude)
{
    return mean - amplitude + 2 * amplitude * md_random();
}

/** dst = mean +- sigma * normalized_rand() */
o_inline double md_random_normal(double mean, double sigma)
{
    double u1, u2;
    do {
        u1 = md_random();
    } while (u1 <= (double) 0.00000001);
    u2 = md_random();

    double mag = sigma * md_sqrt((double) -2 * log(u1));
    return mean + mag * md_cos(2 * md_PI * u2);
}

//
// vecn
//

/** dst = rand() [0:1] */
o_inline void dvecn_random(double *dst, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = md_random();
    }
}

/** dst = rand() [a:b] */
o_inline void dvecn_random_range(double *dst, double a, double b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = md_random_range(a, b);
    }
}

/** dst = rand() [a:b] */
o_inline void dvecn_random_range_vec(double *dst, const double *a, const double *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = md_random_range(a[i], b[i]);
    }
}

/** dst = mean +- amplitude * rand() */
o_inline void dvecn_random_noise(double *dst, double mean, double amplitude, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = md_random_noise(mean, amplitude);
    }
}

/** dst = mean +- amplitude * rand() */
o_inline void dvecn_random_noise_vec(double *dst, const double *mean, const double *amplitude, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = md_random_noise(mean[i], amplitude[i]);
    }
}

/** dst = mean +- sigma * normalized_rand() */
o_inline void dvecn_random_normal(double *dst, double mean, double sigma, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = md_random_normal(mean, sigma);
    }
}

/** dst = mean +- sigma * normalized_rand() */
o_inline void dvecn_random_normal_vec(double *dst, const double *mean, const double *sigma, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = md_random_normal(mean[i], sigma[i]);
    }
}

//
// m_dvec2
//

/** dst = rand() [0:1] */
o_inline m_dvec2 dvec2_random(void)
{
    return (m_dvec2) {
        {
            md_random(),
            md_random()
        }};
}

/** dst = rand() [a:b] */
o_inline m_dvec2 dvec2_random_range(double a, double b)
{
    return (m_dvec2) {
            {
                md_random_range (a, b),
                md_random_range (a, b)
            }};
}

/** dst = rand() [a:b] */
o_inline m_dvec2 dvec2_random_range_v(m_dvec2 a, m_dvec2 b)
{
    return (m_dvec2) {
                {
                    md_random_range (a.v0, b.v0),
                    md_random_range (a.v1, b.v1)
                }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_dvec2 dvec2_random_noise(double mean, double amplitude)
{
    return (m_dvec2) {
                {
                    md_random_noise (mean, amplitude),
                    md_random_noise (mean, amplitude)
                }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_dvec2 dvec2_random_noise_v(m_dvec2 mean, m_dvec2 amplitude)
{
    return (m_dvec2) {
                    {
                        md_random_noise (mean.v0, amplitude.v0),
                        md_random_noise (mean.v1, amplitude.v1)
                    }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_dvec2 dvec2_random_normal(double mean, double sigma)
{
    return (m_dvec2) {
                    {
                        md_random_normal (mean, sigma),
                        md_random_normal (mean, sigma)
                    }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_dvec2 dvec2_random_normal_v(m_dvec2 mean, m_dvec2 sigma)
{
    return (m_dvec2) {
                        {
                            md_random_normal (mean.v0, sigma.v0),
                            md_random_normal (mean.v1, sigma.v1)
                        }};
}

//
// m_dvec3
//


/** dst = rand() [0:1] */
o_inline m_dvec3 dvec3_random(void)
{
    return (m_dvec3) {
                {
                    md_random(),
                    md_random(),
                    md_random()
                }};
}

/** dst = rand() [a:b] */
o_inline m_dvec3 dvec3_random_range(double a, double b)
{
    return (m_dvec3) {
                {
                    md_random_range (a, b),
                    md_random_range (a, b),
                    md_random_range (a, b)
                }};
}

/** dst = rand() [a:b] */
o_inline m_dvec3 dvec3_random_range_v(m_dvec3 a, m_dvec3 b)
{
    return (m_dvec3) {
                    {
                        md_random_range (a.v0, b.v0),
                        md_random_range (a.v1, b.v1),
                        md_random_range (a.v2, b.v2)
                    }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_dvec3 dvec3_random_noise(double mean, double amplitude)
{
    return (m_dvec3) {
                    {
                        md_random_noise (mean, amplitude),
                        md_random_noise (mean, amplitude),
                        md_random_noise (mean, amplitude)
                    }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_dvec3 dvec3_random_noise_v(m_dvec3 mean, m_dvec3 amplitude)
{
    return (m_dvec3) {
                        {
                            md_random_noise (mean.v0, amplitude.v0),
                            md_random_noise (mean.v1, amplitude.v1),
                            md_random_noise (mean.v2, amplitude.v2)
                        }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_dvec3 dvec3_random_normal(double mean, double sigma)
{
    return (m_dvec3) {
                        {
                            md_random_normal (mean, sigma),
                            md_random_normal (mean, sigma),
                            md_random_normal (mean, sigma)
                        }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_dvec3 dvec3_random_normal_v(m_dvec3 mean, m_dvec3 sigma)
{
    return (m_dvec3) {
                            {
                                md_random_normal (mean.v0, sigma.v0),
                                md_random_normal (mean.v1, sigma.v1),
                                md_random_normal (mean.v2, sigma.v2)
                            }};
}


//
// m_dvec4
//

/** dst = rand() [0:1] */
o_inline m_dvec4 dvec4_random(void)
{
    return (m_dvec4) {
                {
                    md_random(),
                    md_random(),
                    md_random(),
                    md_random()
                }};
}

/** dst = rand() [a:b] */
o_inline m_dvec4 dvec4_random_range(double a, double b)
{
    return (m_dvec4) {
                {
                    md_random_range (a, b),
                    md_random_range (a, b),
                    md_random_range (a, b),
                    md_random_range (a, b)
                }};
}

/** dst = rand() [a:b] */
o_inline m_dvec4 dvec4_random_range_v(m_dvec4 a, m_dvec4 b)
{
    return (m_dvec4) {
                    {
                        md_random_range (a.v0, b.v0),
                        md_random_range (a.v1, b.v1),
                        md_random_range (a.v2, b.v2),
                        md_random_range (a.v3, b.v3)
                    }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_dvec4 dvec4_random_noise(double mean, double amplitude)
{
    return (m_dvec4) {
                    {
                        md_random_noise (mean, amplitude),
                        md_random_noise (mean, amplitude),
                        md_random_noise (mean, amplitude),
                        md_random_noise (mean, amplitude)
                    }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_dvec4 dvec4_random_noise_v(m_dvec4 mean, m_dvec4 amplitude)
{
    return (m_dvec4) {
                        {
                            md_random_noise (mean.v0, amplitude.v0),
                            md_random_noise (mean.v1, amplitude.v1),
                            md_random_noise (mean.v2, amplitude.v2),
                            md_random_noise (mean.v3, amplitude.v3)
                        }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_dvec4 dvec4_random_normal(double mean, double sigma)
{
    return (m_dvec4) {
                        {
                            md_random_normal (mean, sigma),
                            md_random_normal (mean, sigma),
                            md_random_normal (mean, sigma),
                            md_random_normal (mean, sigma)
                        }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_dvec4 dvec4_random_normal_v(m_dvec4 mean, m_dvec4 sigma)
{
    return (m_dvec4) {
                            {
                                md_random_normal (mean.v0, sigma.v0),
                                md_random_normal (mean.v1, sigma.v1),
                                md_random_normal (mean.v2, sigma.v2),
                                md_random_normal (mean.v3, sigma.v3)
                            }};
}

#endif // M_UTILS_DBL_DRANDOM
