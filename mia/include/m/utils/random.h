#ifndef M_UTILS_FLT_RANDOM
#define M_UTILS_FLT_RANDOM

/**
 * @file utils/random.h
 *
 * Random math for the types
 */


#ifndef M_RANDOM_FUNCTION
#define M_RANDOM_FUNCTION o_rand
#endif

#ifndef M_RANDOM_FUNCTION_MAX
#define M_RANDOM_FUNCTION_MAX (ou32_MAX-1)
#endif

#include "m/m_types/flt.h"
#include "m/sca/flt.h"

/** dst = rand() [0:1] */
o_inline float m_random(void)
{
    return (float) 1 * M_RANDOM_FUNCTION() / (float) M_RANDOM_FUNCTION_MAX;
}

/** dst = rand() [a:b] */
o_inline float m_random_range(float a, float b)
{
    return a + (b - a) * m_random();
}

/** dst = mean +- amplitude * rand() */
o_inline float m_random_noise(float mean, float amplitude)
{
    return mean - amplitude + 2 * amplitude * m_random();
}

/** dst = mean +- sigma * normalized_rand() */
o_inline float m_random_normal(float mean, float sigma)
{
    float u1, u2;
    do {
        u1 = m_random();
    } while (u1 <= (float) 0.00000001);
    u2 = m_random();

    float mag = sigma * m_sqrt((float) -2 * log(u1));
    return mean + mag * m_cos(2 * m_PI * u2);
}

//
// vecn
//

/** dst = rand() [0:1] */
o_inline void vecn_random(float *dst, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_random();
    }
}

/** dst = rand() [a:b] */
o_inline void vecn_random_range(float *dst, float a, float b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_random_range(a, b);
    }
}

/** dst = rand() [a:b] */
o_inline void vecn_random_range_v(float *dst, const float *a, const float *b, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_random_range(a[i], b[i]);
    }
}

/** dst = mean +- amplitude * rand() */
o_inline void vecn_random_noise(float *dst, float mean, float amplitude, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_random_noise(mean, amplitude);
    }
}

/** dst = mean +- amplitude * rand() */
o_inline void vecn_random_noise_v(float *dst, const float *mean, const float *amplitude, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_random_noise(mean[i], amplitude[i]);
    }
}

/** dst = mean +- sigma * normalized_rand() */
o_inline void vecn_random_normal(float *dst, float mean, float sigma, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_random_normal(mean, sigma);
    }
}

/** dst = mean +- sigma * normalized_rand() */
o_inline void vecn_random_normal_v(float *dst, const float *mean, const float *sigma, int n)
{
    for (int i = 0; i < n; i++) {
        dst[i] = m_random_normal(mean[i], sigma[i]);
    }
}

//
// m_vec2
//

/** dst = rand() [0:1] */
o_inline m_vec2 vec2_random(void)
{
    return (m_vec2) {
        {
            m_random(),
            m_random()
        }};
}

/** dst = rand() [a:b] */
o_inline m_vec2 vec2_random_range(float a, float b)
{
    return (m_vec2) {
            {
                m_random_range (a, b),
                m_random_range (a, b)
            }};
}

/** dst = rand() [a:b] */
o_inline m_vec2 vec2_random_range_v(m_vec2 a, m_vec2 b)
{
    return (m_vec2) {
                {
                    m_random_range (a.v0, b.v0),
                    m_random_range (a.v1, b.v1)
                }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_vec2 vec2_random_noise(float mean, float amplitude)
{
    return (m_vec2) {
                {
                    m_random_noise (mean, amplitude),
                    m_random_noise (mean, amplitude)
                }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_vec2 vec2_random_noise_v(m_vec2 mean, m_vec2 amplitude)
{
    return (m_vec2) {
                    {
                        m_random_noise (mean.v0, amplitude.v0),
                        m_random_noise (mean.v1, amplitude.v1)
                    }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_vec2 vec2_random_normal(float mean, float sigma)
{
    return (m_vec2) {
                    {
                        m_random_normal (mean, sigma),
                        m_random_normal (mean, sigma)
                    }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_vec2 vec2_random_normal_v(m_vec2 mean, m_vec2 sigma)
{
    return (m_vec2) {
                        {
                            m_random_normal (mean.v0, sigma.v0),
                            m_random_normal (mean.v1, sigma.v1)
                        }};
}

//
// m_vec3
//


/** dst = rand() [0:1] */
o_inline m_vec3 vec3_random(void)
{
    return (m_vec3) {
                {
                    m_random(),
                    m_random(),
                    m_random()
                }};
}

/** dst = rand() [a:b] */
o_inline m_vec3 vec3_random_range(float a, float b)
{
    return (m_vec3) {
                {
                    m_random_range (a, b),
                    m_random_range (a, b),
                    m_random_range (a, b)
                }};
}

/** dst = rand() [a:b] */
o_inline m_vec3 vec3_random_range_v(m_vec3 a, m_vec3 b)
{
    return (m_vec3) {
                    {
                        m_random_range (a.v0, b.v0),
                        m_random_range (a.v1, b.v1),
                        m_random_range (a.v2, b.v2)
                    }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_vec3 vec3_random_noise(float mean, float amplitude)
{
    return (m_vec3) {
                    {
                        m_random_noise (mean, amplitude),
                        m_random_noise (mean, amplitude),
                        m_random_noise (mean, amplitude)
                    }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_vec3 vec3_random_noise_v(m_vec3 mean, m_vec3 amplitude)
{
    return (m_vec3) {
                        {
                            m_random_noise (mean.v0, amplitude.v0),
                            m_random_noise (mean.v1, amplitude.v1),
                            m_random_noise (mean.v2, amplitude.v2)
                        }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_vec3 vec3_random_normal(float mean, float sigma)
{
    return (m_vec3) {
                        {
                            m_random_normal (mean, sigma),
                            m_random_normal (mean, sigma),
                            m_random_normal (mean, sigma)
                        }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_vec3 vec3_random_normal_v(m_vec3 mean, m_vec3 sigma)
{
    return (m_vec3) {
                            {
                                m_random_normal (mean.v0, sigma.v0),
                                m_random_normal (mean.v1, sigma.v1),
                                m_random_normal (mean.v2, sigma.v2)
                            }};
}


//
// m_vec4
//

/** dst = rand() [0:1] */
o_inline m_vec4 vec4_random(void)
{
    return (m_vec4) {
                {
                    m_random(),
                    m_random(),
                    m_random(),
                    m_random()
                }};
}

/** dst = rand() [a:b] */
o_inline m_vec4 vec4_random_range(float a, float b)
{
    return (m_vec4) {
                {
                    m_random_range (a, b),
                    m_random_range (a, b),
                    m_random_range (a, b),
                    m_random_range (a, b)
                }};
}

/** dst = rand() [a:b] */
o_inline m_vec4 vec4_random_range_v(m_vec4 a, m_vec4 b)
{
    return (m_vec4) {
                    {
                        m_random_range (a.v0, b.v0),
                        m_random_range (a.v1, b.v1),
                        m_random_range (a.v2, b.v2),
                        m_random_range (a.v3, b.v3)
                    }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_vec4 vec4_random_noise(float mean, float amplitude)
{
    return (m_vec4) {
                    {
                        m_random_noise (mean, amplitude),
                        m_random_noise (mean, amplitude),
                        m_random_noise (mean, amplitude),
                        m_random_noise (mean, amplitude)
                    }};
}

/** dst = mean +- amplitude * rand() */
o_inline m_vec4 vec4_random_noise_v(m_vec4 mean, m_vec4 amplitude)
{
    return (m_vec4) {
                        {
                            m_random_noise (mean.v0, amplitude.v0),
                            m_random_noise (mean.v1, amplitude.v1),
                            m_random_noise (mean.v2, amplitude.v2),
                            m_random_noise (mean.v3, amplitude.v3)
                        }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_vec4 vec4_random_normal(float mean, float sigma)
{
    return (m_vec4) {
                        {
                            m_random_normal (mean, sigma),
                            m_random_normal (mean, sigma),
                            m_random_normal (mean, sigma),
                            m_random_normal (mean, sigma)
                        }};
}

/** dst = mean +- sigma * normalized_rand() */
o_inline m_vec4 vec4_random_normal_v(m_vec4 mean, m_vec4 sigma)
{
    return (m_vec4) {
                            {
                                m_random_normal (mean.v0, sigma.v0),
                                m_random_normal (mean.v1, sigma.v1),
                                m_random_normal (mean.v2, sigma.v2),
                                m_random_normal (mean.v3, sigma.v3)
                            }};
}

#endif // M_UTILS_FLT_RANDOM
