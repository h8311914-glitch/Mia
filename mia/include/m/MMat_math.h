#ifndef M_MMAT_MATH_H
#define M_MMAT_MATH_H

/**
 * @file MMat_math.h
 *
 * Object extension
 *
 * MMat_ namespace
 *
 * basic math operations
 *
 * @sa MMat_ex to import all MMat object extensions
 */

#include "MMat.h"


//
// Math Operations
//


/**
 * @param obj MMat object
 * @param out_min smallest values for each channel [0:1]
 * @note fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
void MMat_min_x(oobj obj, void *restrict out_min);
/**
 * @param obj MMat object
 * @return smallest values for each channel [0:1] (up to 4)
 * @note fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
m_vec4 MMat_min_vec4(oobj obj);


/**
 * @param obj MMat object
 * @param out_max highest values for each channel [0:1]
 * @note fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
void MMat_max_x(oobj obj, void *restrict out_max);
/**
 * @param obj MMat object
 * @return highest values for each channel [0:1] (op to 4)
 * @note fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
m_vec4 MMat_max_vec4(oobj obj);


/**
 * @param obj MMat object
 * @param out_min, out_max: smallest and highest values for each channel [0:1]
 * @note fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
void MMat_minmax_x(oobj obj, void *restrict out_min, void *restrict out_max);
/**
 * @param obj MMat object
 * @param out_min, out_max: smallest and highest values for each channel [0:1] (op to 4)
 * @note fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
void MMat_minmax_vec4(oobj obj, m_vec4 *out_min, m_vec4 *out_max);


/**
 * @param obj MMat object
 * @param add for each channel (in double, so it works well with integer based data)
 * @note resulting pixel values are clamped [0:1].
 *       fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
void MMat_add_dbl(oobj obj, const double *restrict add);
/**
 * @param obj MMat object
 * @param add for each channel, e. g. to add only alpha use vec4_(0, 0, 0, X)
 * @note resulting pixel values are clamped [0:1].
 *       fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
void MMat_add_vec4(oobj obj, m_vec4 add);


/**
 * Scales one or more channels in this matrix
 * @param obj MMat object
 * @param scale for each channel (in double, so it works well with integer based data)
 * @note resulting pixel values are clamped [0:1] or [0:255].
 *       fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
void MMat_scale_dbl(oobj obj, const double *restrict scale);
/**
 * Scales one or more channels in this matrix
 * @param obj MMat object
 * @param scale for each channel, e. g. to scale only alpha use vec4_(1, 1, 1, X)
 * @note resulting pixel values are clamped [0:1] or [0:255].
 *       fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32
 */
void MMat_scale_vec4(oobj obj, m_vec4 scale);


/**
 * result = (obj + add) * scale
 * @param obj MMat object
 * @param add for each channel (in double, so it works well with integer based data)
 * @param scale for each channel  (in double, so it works well with integer based data)
 * @note resulting pixel values are clamped [0:1] or [0:255].
 *       fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32.
 *       unlike vec4_add_scaled which is a+b*c not like this as (a+b)*c
 */
void MMat_add_and_scale_dbl(oobj obj, const double *restrict add, const double *restrict scale);
/**
 * result = (obj + add) * scale
 * @param obj MMat object
 * @param add for each channel, e. g. to add only alpha use vec4_(0, 0, 0, X)
 * @param scale for each channel, e. g. to scale only alpha use vec4_(1, 1, 1, X)
 * @note resulting pixel values are clamped [0:1] or [0:255].
 *       fast path for: M_FORMAT_1_U8, M_FORMAT_4_U8, M_FORMAT_1_F32, M_FORMAT_4_F32.
 *       unlike vec4_add_scaled which is a+b*c not like this as (a+b)*c
 */
void MMat_add_and_scale_vec4(oobj obj, m_vec4 add, m_vec4 scale);


/**
 * Normalizes the matrix, so that the previous min value gets 0 and max -> 1
 * @param obj MMat object
 */
void MMat_normalize(oobj obj);



#endif //M_MMAT_MATH_H
