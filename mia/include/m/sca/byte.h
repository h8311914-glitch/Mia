#ifndef M_SCA_BYTE_H
#define M_SCA_BYTE_H

/**
 * @file sca/byte.h
 *
 * some defines in the mb_ namespace
 */

#include "m/common.h"

/** maximum possible value */
#define mb_MAX UINT8_MAX

/** minimum possible value */
#define mb_MIN 0

/** for printing in IO */
#define mb_PRI PRIu8


//
// macro copies of o/common
//

/** @return the smaller value */
#define mb_min(a, b) ((a)<(b)?(a):(b))

/** @return the bigger value */
#define mb_max(a, b) ((a)>(b)?(a):(b))

/** @return x clamped in the range [mbn : max] (so can be exactly mbn and exactly max) */
#define mb_clamp(x, mbn, max) ((x) < (mbn) ? (mbn) : ((x) > (max) ? (max) : (x)))

/** @return x % y as positive version for integers */
#define mb_mod(x, y) (((x) % (y) + (y)) % (y))

/** @return x>=edge? 1:0 (true or 1 if x has reached the step) */
#define mb_step(edge, x) (((x) < (edge)) ? 0 : 1)

//
// integer based functions
//

/** dst = a * (1-t) + b * t */
#define mb_mix(a, b, t) \
((obyte) ((float) (a) * (1.0f - (float) (t)) + (float) (b) * (float) (t)))

/** a ~= b */
#define mb_equals_eps(a, b, eps) \
    (((obyte) mb_max((obyte) (a), (obyte) (b)) - (obyte) mb_min((obyte) (a), (obyte) (b))) <= (obyte) (eps))


#endif //M_SCA_BYTE_H
