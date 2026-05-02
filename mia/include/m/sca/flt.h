#ifndef M_SCA_FLT_H
#define M_SCA_FLT_H

/**
 * @file sca/flt.h
 *
 * scalar functions working on floats in the m_ namespace
 */

#include "m/common.h"

/** maximum possible value */
#define m_MAX FLT_MAX

/** minimum possible value */
#define m_MIN (-FLT_MAX)

/** smallest positive value */
#define m_SMALL FLT_MIN

/** smallest positive value change */
#define m_EPS FLT_EPSILON

/* clone of some math.h constants */
#define m_E 2.718281828459045235360287471352662498f /* e */
#define m_LOG2E 1.442695040888963407359924681001892137f /* log_2 e */
#define m_LOG10E 0.434294481903251827651128918916605082f /* log_10 e */
#define m_LN2 0.693147180559945309417232121458176568f /* log_e 2 */
#define m_LN10 2.302585092994045684017991454684364208f /* log_e 10 */
#define m_PI 3.141592653589793238462643383279502884f /* pi */
#define m_PI_2 1.570796326794896619231321691639751442f /* pi/2 */
#define m_PI_4 0.785398163397448309615660845819875721f /* pi/4 */
#define m_1_PI 0.318309886183790671537767526745028724f /* 1/pi */
#define m_2_PI 0.636619772367581343075535053490057448f /* 2/pi */
#define m_2_SQRTPI 1.128379167095512573896158903121545172f /* 2/sqrt(pi) */
#define m_SQRT2 1.414213562373095048801688724209698079f /* sqrt(2) */
#define m_SQRT1_2 0.707106781186547524400844362104849039f /* 1/sqrt(2) */

/** for printing in IO */
#define m_PRI "g"

//
// macro copies of o/common
//

/** @return the absolute value */
#define m_abs(x) ((x) < 0 ? -(x) : (x))

/** @return the smaller value */
#define m_min(a, b) ((a)<(b)?(a):(b))

/** @return the bigger value */
#define m_max(a, b) ((a)>(b)?(a):(b))

/** @return x clamped in the range [min : max] (so can be exactly min and exactly max) */
#define m_clamp(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/** @return x>0: +1; x==0: 0; x<0: -1 (the sign of the value, or 0 on 0) */
#define m_sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/** @return x>=edge? 1:0 (true or 1 if x has reached the step) */
#define m_step(edge, x) (((x) < (edge)) ? 0 : 1)

//
// integer based functions
//

/** dst = a * (1-t) + b * t */
#define m_mix(a, b, t) \
    ((float) (a) * (1.0f - (float) (t)) + (float) (b) * (float) (t))


/** a ~= b */
#define m_equals_eps(a, b, eps) \
    ((float) m_abs((float) (a) - (float) (b)) <= (float) (eps))

//
// floating point functions
//



/** dst = isnan(x) */
#define m_isnan(x) \
    ((float) isnan((float) (x)))


/** dst = isinf(x) */
#define m_isinf(x) \
    ((float) isinf((float) (x)))


/** dst = a * M_PI / 180 */
#define m_radians(deg) \
    ((float) (deg) * m_PI / 180.0f)


/** dst = a * 180 / M_PI */
#define m_degrees(rad) \
    ((float) (rad) * 180.0f / m_PI)


/** dst = sin(angle_rad) */
#define m_sin(angle_rad) \
    ((float) sinf((float) (angle_rad)))


/** dst = cos(angle_rad) */
#define m_cos(angle_rad) \
    ((float) cosf((float) (angle_rad)))

/** dst = tan(angle_rad) */
#define m_tan(angle_rad) \
    ((float) tanf((float) (angle_rad)))

/** dst = asin(x) */
#define m_asin(x) \
    ((float) asinf((float) (x)))


/** dst = acos(x) */
#define m_acos(x) \
    ((float) acosf((float) (x)))


/** dst = atan(x) */
#define m_atan(x) \
    ((float) atanf((float) (x)))


/** dst = atan2(y, x) */
#define m_atan2(y, x) \
    ((float) atan2f((float) (y), (float) (x)))


/** dst = sinh(angle_rad) */
#define m_sinh(angle_rad) \
    ((float) sinhf((float) (angle_rad)))


/** dst = cosh(angle_rad) */
#define m_cosh(angle_rad) \
    ((float) coshf((float) (angle_rad)))


/** dst = tanh(angle_rad) */
#define m_tanh(angle_rad) \
    ((float) tanhf((float) (angle_rad)))


/** dst = asinh(x) */
#define m_asinh(x) \
    ((float) asinhf((float) (x)))


/** dst = acosh(x) */
#define m_acosh(x) \
    ((float) acoshf((float) (x)))


/** dst = atanh(x) */
#define m_atanh(x) \
    ((float) atanhf((float) (x)))


/** dst = pow(x, y) */
#define m_pow(x, y) \
    ((float) powf((float) (x), (float) (y)))


/** dst = exp(x) */
#define m_exp(x) \
    ((float) expf((float) (x)))


/** dst = log(x) */
#define m_log(x) \
    ((float) logf((float) (x)))


/** dst = exp2(x) */
#define m_exp2(x) \
    ((float) exp2f((float) (x)))


/** dst = log2(x) */
#define m_log2(x) \
    ((float) log2f((float) (x)))


/** dst = sqrt(x) */
#define m_sqrt(x) \
    ((float) sqrtf((float) (x)))


/** dst = 1 / sqrt(x) */
#define m_isqrt(x) \
    (1.0f / (float) sqrtf((float) (x)))


/** dst = round(x) */
#define m_round(x) \
    ((float) roundf((float) (x)))


/** dst = floor(x) */
#define m_floor(x) \
    ((float) floorf((float) (x)))


/** dst = ceil(x) */
#define m_ceil(x) \
    ((float) ceilf((float) (x)))


/** dst = x - floor(x) */
#define m_fract(x) \
    ((float) (x) - (float) floorf((float) (x)))


/** dst = x - y * floor(x/y) */
#define m_mod(x, y) \
    ((float) (x) - (float) (y) * (float) floorf((float) (x) / (float) (y)))


/** dst = godot like ease(x, y) */
o_inline float m_ease(float x, float y)
{
    x = m_clamp(x, 0.0f, 1.0f);
    if (y > 0.0f) {
        return y<1.0f?
                1.0f - m_pow(1.0f - x, 1.0f / y)
                : m_pow(x, y);
    } else if (y < 0.0f) {
        return x<0.5f?
                m_pow(x * 2.0f, -y) * 0.5f
                : (1.0f - m_pow(1.0f - (x - 0.5f) * 2.0f, -y)) * 0.5f + 0.5f;
    }
    return 0.0f;
}


/** dst = x < edge1 ? 0 : (x > edge2 ? 1 : x * x * (3 - 2 * x)) Hermite polynomials */
o_inline float m_smoothstep(float edge1, float edge2, float x)
{
    x = o_clamp((x - edge1) / (edge2 - edge1), 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

/** dst = cubic spline (hermite) between a and b, t in seconds as well as time (or use the same scaling, not [0:1]) **/
o_inline float m_spline(float a, float tangent_a, float b, float tangent_b, float t, float time)
{
    // normalize
    t /= time;
    return (2.0f * t * t * t - 3.0f * t * t + 1.0f) * a +
           (t * t * t - 2.0f * t * t + t) * tangent_a * time +
           (-2.0f * t * t * t + 3.0f * t * t) * b +
           (t * t * t - t * t) * tangent_b * time;

}

/**
 * dst = sin(x*2pi)
 *  X   ->   Y
 * 0.00 ->  0.0
 * 0.25 -> +1.0
 * 0.50 ->  0.0
 * 0.75 -> -1.0
 */
o_inline float m_signal_wave(float x)
{
    return m_sin(x * 2.0f * m_PI);
}

/**
 * dst = -1 or 1, x: [0:1] (0-0.5 is -1)
 *  X   ->   Y
 * 0.25 -> -1.0
 * 0.75 -> +1.0
 */
o_inline float m_signal_block(float x)
{
    return 2.0f * o_step(0.5, m_mod(x, 1.0f)) - 1.0f;
}

/**
 * dst = saw like signal, linear up, step down, x: [0:1] -> dst [-1:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 0.50 ->  0.0
 */
o_inline float m_signal_saw(float x)
{
    return 2.0f * m_mod(x, 1.0f) - 1.0f;
}

/**
 * dst = linear up and down signal, x: [0:1] -> dst [-1:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 0.25 ->  0.0
 * 0.50 -> +1.0
 * 0.75 ->  0.0
 */
o_inline float m_signal_ramp(float x)
{
    x = m_mod(x + 0.5f, 1.0f);
    return 4.0f * o_abs(x - 0.5f) - 1.0f;
}

/**
 * dst = saw like signal, linear up, step down, x: [0:1] -> dst [-1:1]
 * smoothed edge at [1-edge:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 1.00 -> +1.0
 */
o_inline float m_signal_smoothsaw_single(float x, float edge)
{
    x = m_mod(x, 1.0f);

    // linear: y = a*x
    // smooth: y = 1-b*(x-1)^2
    // equations solved, so that f==(1-edge) -> y and derivate are equal
    float f = 1.0f - edge;
    float b = -1.0f / (2.0f * (f - 1.0f));
    float a = (1.0f - b * m_pow(f - 1.0f, 2.0f)) / f;

    float signal;
    // linear up
    if (x < f) {
        signal = a * x;
    } else {
        // smooth end
        signal = 1.0f - b * m_pow(x - 1.0f, 2.0f);
    }
    return -1.0f + 2.0f * signal;
}

/*
 * dst = saw like signal, linear up, step down, x: [0:1] -> dst [-1:1]
 * smoothed edges between [0:edge] , [1-edge:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 0.50 ->  0.0
 */
o_inline float m_signal_smoothsaw(float x, float edge)
{
    x = m_mod(x, 1.0f);
    if (x < 0.5f) {
        // 0.5*2 is exactly 1.0f, which will mod to 0, which will result in 1.0 instead of -1.0
        return -0.5f - m_signal_smoothsaw_single((0.5f - x) * 1.99999f, edge * 2.0f) / 2.0f;
    }
    return 0.5f + m_signal_smoothsaw_single((x - 0.5f) * 2.0f, edge * 2.0f) / 2.0f;
}

/**
 * dst = ramp like signal, linear up, linear down, x: [0:1] -> dst [-1:1]
 * smoothed edges between [0:edge] , [0.5-edge:0.5+edge] , [1-edge:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 0.25 ->  0.0
 * 0.50 -> +1.0
 * 0.75 ->  0.0
 */
o_inline float m_signal_smoothramp(float x, float edge)
{
    x = m_mod(x, 1.0f);
    if (x < 0.5f) {
        return m_signal_smoothsaw(x * 2.0f, edge * 2.0f);
    }
    return -m_signal_smoothsaw((x - 0.5f) * 2.0f, edge * 2.0f);
}


#endif //M_SCA_FLT_H
