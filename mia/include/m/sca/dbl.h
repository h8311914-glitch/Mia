#ifndef md_SCA_DBL_H
#define md_SCA_DBL_H

/**
 * @file sca/dbl.h
 *
 * scalar functions working on double in the md_ namespace
 */

#include "m/common.h"

/** maximum possible value */
#define md_MAX DBL_MAX

/** minimum possible value */
#define md_MIN (-DBL_MAX)

/** smallest positive value */
#define md_SMALL DBL_MIN

/** smallest positive value change */
#define md_EPS DBL_EPSILON

/* clone o some math.h constants */
#define md_E 2.718281828459045235360287471352662498 /* e */
#define md_LOG2E 1.442695040888963407359924681001892137 /* log_2 e */
#define md_LOG10E 0.434294481903251827651128918916605082 /* log_10 e */
#define md_LN2 0.693147180559945309417232121458176568 /* log_e 2 */
#define md_LN10 2.302585092994045684017991454684364208 /* log_e 10 */
#define md_PI 3.141592653589793238462643383279502884 /* pi */
#define md_PI_2 1.570796326794896619231321691639751442 /* pi/2 */
#define md_PI_4 0.785398163397448309615660845819875721 /* pi/4 */
#define md_1_PI 0.318309886183790671537767526745028724 /* 1/pi */
#define md_2_PI 0.636619772367581343075535053490057448 /* 2/pi */
#define md_2_SQRTPI 1.128379167095512573896158903121545172 /* 2/sqrt(pi) */
#define md_SQRT2 1.414213562373095048801688724209698079 /* sqrt(2) */
#define md_SQRT1_2 0.707106781186547524400844362104849039 /* 1/sqrt(2) */

/** for printing in IO */
#define md_PRI "g"


//
// macro copies of o/common
//

/** @return the absolute value */
#define md_abs(x) ((x) < 0 ? -(x) : (x))

/** @return the smaller value */
#define md_min(a, b) ((a)<(b)?(a):(b))

/** @return the bigger value */
#define md_max(a, b) ((a)>(b)?(a):(b))

/** @return x clamped in the range [min : max] (so can be exactly min and exactly max) */
#define md_clamp(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/** @return x>0: +1; x==0: 0; x<0: -1 (the sign of the value, or 0 on 0) */
#define md_sign(x) ((x) > 0 ? 1 : ((x) < 0 ? -1 : 0))

/** @return x>=edge? 1:0 (true or 1 if x has reached the step) */
#define md_step(edge, x) (((x) < (edge)) ? 0 : 1)

//
// integer based functions
//

/** dst = a * (1-t) + b * t */
#define md_mix(a, b, t) \
    ((double) (a) * (1.0 - (double) (t)) + (double) (b) * (double) (t))


/** a ~= b */
#define md_equals_eps(a, b, eps) \
    ((double) md_abs((double) (a) - (double) (b)) <= (double) (eps))

//
// floating point functions
//



/** dst = isnan(x) */
#define md_isnan(x) \
    ((double) isnan((double) (x)))


/** dst = isinf(x) */
#define md_isinf(x) \
    ((double) isinf((double) (x)))


/** dst = a * md_PI / 180 */
#define md_radians(deg) \
    ((double) (deg) * md_PI / 180.0)


/** dst = a * 180 / md_PI */
#define md_degrees(rad) \
    ((double) (rad) * 180.0 / md_PI)


/** dst = sin(angle_rad) */
#define md_sin(angle_rad) \
    ((double) sin((double) (angle_rad)))


/** dst = cos(angle_rad) */
#define md_cos(angle_rad) \
    ((double) cos((double) (angle_rad)))

/** dst = tan(angle_rad) */
#define md_tan(angle_rad) \
    ((double) tan((double) (angle_rad)))

/** dst = asin(x) */
#define md_asin(x) \
    ((double) asin((double) (x)))


/** dst = acos(x) */
#define md_acos(x) \
    ((double) acos((double) (x)))


/** dst = atan(x) */
#define md_atan(x) \
    ((double) atan((double) (x)))


/** dst = atan2(y, x) */
#define md_atan2(y, x) \
    ((double) atan2((double) (y), (double) (x)))


/** dst = sinh(angle_rad) */
#define md_sinh(angle_rad) \
    ((double) sinh((double) (angle_rad)))


/** dst = cosh(angle_rad) */
#define md_cosh(angle_rad) \
    ((double) cosh((double) (angle_rad)))


/** dst = tanh(angle_rad) */
#define md_tanh(angle_rad) \
    ((double) tanh((double) (angle_rad)))


/** dst = asinh(x) */
#define md_asinh(x) \
    ((double) asinh((double) (x)))


/** dst = acosh(x) */
#define md_acosh(x) \
    ((double) acosh((double) (x)))


/** dst = atanh(x) */
#define md_atanh(x) \
    ((double) atanh((double) (x)))


/** dst = pow(x, y) */
#define md_pow(x, y) \
    ((double) pow((double) (x), (double) (y)))


/** dst = exp(x) */
#define md_exp(x) \
    ((double) exp((double) (x)))


/** dst = log(x) */
#define md_log(x) \
    ((double) log((double) (x)))


/** dst = exp2(x) */
#define md_exp2(x) \
    ((double) exp2((double) (x)))


/** dst = log2(x) */
#define md_log2(x) \
    ((double) log2((double) (x)))


/** dst = sqrt(x) */
#define md_sqrt(x) \
    ((double) sqrt((double) (x)))


/** dst = 1 / sqrt(x) */
#define md_isqrt(x) \
    (1.0 / (double) sqrt((double) (x)))


/** dst = round(x) */
#define md_round(x) \
    ((double) round((double) (x)))
    

/** dst = floor(x) */
#define md_floor(x) \
    ((double) floor((double) (x)))


/** dst = ceil(x) */
#define md_ceil(x) \
    ((double) ceil((double) (x)))


/** dst = x - floor(x) */
#define md_fract(x) \
    ((double) (x) - (double) floor((double) (x)))
    

/** dst = x - y * floor(x/y) */
#define md_mod(x, y) \
((double) (x) - (double) (y) * (double) floor((double) (x) / (double) (y)))

/** dst = godot like ease(x, y) */
o_inline double md_ease(double x, double y)
{
    x = md_clamp(x, 0.0, 1.0);
    if (y > 0.0) {
        return y<1.0?
                1.0 - md_pow(1.0 - x, 1.0 / y)
                : md_pow(x, y);
    } else if (y < 0.0) {
        return x<0.5f?
                md_pow(x * 2.0, -y) * 0.5f
                : (1.0 - md_pow(1.0 - (x - 0.5f) * 2.0, -y)) * 0.5f + 0.5f;
    }
    return 0.0;
}



/** dst = x < edge1 ? 0 : (x > edge2 ? 1 : x * x * (3 - 2 * x)) Hermite polynomials */
o_inline double md_smoothstep(double edge1, double edge2, double x)
{
    x = o_clamp((x - edge1) / (edge2 - edge1), 0.0, 1.0);
    return x * x * (3.0 - 2.0 * x);
}

/** dst = cubic spline (hermite) between a and b, t in seconds as well as time (or use the same scaling, not [0:1]) **/
o_inline double md_spline(double a, double tangent_a, double b, double tangent_b, double t, double time)
{
    // normalize
    t /= time;
    return (2.0 * t * t * t - 3.0 * t * t + 1.0) * a +
           (t * t * t - 2.0 * t * t + t) * tangent_a * time +
           (-2.0 * t * t * t + 3.0 * t * t) * b +
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
o_inline double md_signal_wave(double x)
{
    return md_sin(x * 2.0 * md_PI);
}

/**
 * dst = -1 or 1, x: [0:1] (0-0.5 is -1)
 *  X   ->   Y
 * 0.25 -> -1.0
 * 0.75 -> +1.0
 */
o_inline double md_signal_block(double x)
{
    return 2.0 * o_step(0.5, md_mod(x, 1.0)) - 1.0;
}

/**
 * dst = saw like signal, linear up, step down, x: [0:1] -> dst [-1:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 0.50 ->  0.0
 */
o_inline double md_signal_saw(double x)
{
    return 2.0 * md_mod(x, 1.0) - 1.0;
}

/**
 * dst = linear up and down signal, x: [0:1] -> dst [-1:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 0.25 ->  0.0
 * 0.50 -> +1.0
 * 0.75 ->  0.0
 */
o_inline double md_signal_ramp(double x)
{
    x = md_mod(x + 0.5, 1.0);
    return 4.0 * o_abs(x - 0.5) - 1.0;
}

/**
 * dst = saw like signal, linear up, step down, x: [0:1] -> dst [-1:1]
 * smoothed edge at [1-edge:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 1.00 -> +1.0
 */
o_inline double md_signal_smoothsaw_single(double x, double edge)
{
    x = md_mod(x, 1.0);

    // linear: y = a*x
    // smooth: y = 1-b*(x-1)^2
    // equations solved, so that f==(1-edge) -> y and derivate are equal
    double f = 1.0 - edge;
    double b = -1.0 / (2.0 * (f - 1.0));
    double a = (1.0 - b * md_pow(f - 1.0, 2.0)) / f;

    double signal;
    // linear up
    if (x < f) {
        signal = a * x;
    } else {
        // smooth end
        signal = 1.0 - b * md_pow(x - 1.0, 2.0);
    }
    return -1.0 + 2.0 * signal;
}

/*
 * dst = saw like signal, linear up, step down, x: [0:1] -> dst [-1:1]
 * smoothed edges between [0:edge] , [1-edge:1]
 *  X   ->   Y
 * 0.00 -> -1.0
 * 0.50 ->  0.0
 */
o_inline double md_signal_smoothsaw(double x, double edge)
{
    x = md_mod(x, 1.0);
    if (x < 0.5) {
        // 0.5*2 is exactly 1.0, which will mod to 0, which will result in 1.0 instead of -1.0
        return -0.5 - md_signal_smoothsaw_single((0.5 - x) * 1.99999, edge * 2.0) / 2.0;
    }
    return 0.5 + md_signal_smoothsaw_single((x - 0.5) * 2.0, edge * 2.0) / 2.0;
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
o_inline double md_signal_smoothramp(double x, double edge)
{
    x = md_mod(x, 1.0);
    if (x < 0.5) {
        return md_signal_smoothsaw(x * 2.0, edge * 2.0);
    }
    return -md_signal_smoothsaw((x - 0.5) * 2.0, edge * 2.0);
}

#endif //md_SCA_DBL_H
