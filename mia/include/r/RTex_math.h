#ifndef R_RTEX_MATH_H
#define R_RTEX_MATH_H

/**
 * @file RTex_math.h
 *
 * Object extension
 *
 * RTex_ namespace
 *
 * basic math operations
 *
 * @sa RTex_ex.h to import all RTex object extensions
 */

#include "RTex.h"


/**
 * @param obj RTex object
 * @param into RTex object to render into
 * @return RTex allocated on obj, with each pixel: (obj + add) * scale
 */
void RTex_add_scaled_into(oobj obj, vec4 add, vec4 scale, oobj into);

/**
 * @param obj RTex object
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj, with each pixel: (obj + add) * scale
 */
o_inline RTex *RTex_add_scaled(oobj obj, vec4 add, vec4 scale, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_add_scaled_into(obj, add, scale, res);
    return res;
}


/**
 * @param obj RTex object
 * @param inv_mask channel mask, 0 to clear, 1 to keep, -1 to inv (or in between for scaled)
 * @param into RTex object to render into
 * @note just calls RTex_mixer with an inv_mask as diagonal values
 */
void RTex_inv_into(oobj obj, vec4 inv_mask, oobj into);

/**
 * @param obj RTex object
 * @param inv_mask channel mask, 0 to clear, 1 to keep, -1 to inv (or in between for scaled)
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj, with channels inverted
 * @note just calls RTex_mixer with an inv_mask as diagonal values
 */
o_inline RTex *RTex_inv(oobj obj, vec4 inv_mask, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_inv_into(obj, inv_mask, res);
    return res;
}

/**
 * Runs a min operation with the given tex_b image and this image and returns a new RTex with the result
 * @param obj RTex object
 * @param tex_b second operand image
 * @param into RTex object to render into
 */
void RTex_op_min_into(oobj obj, oobj tex_b, oobj into);


/**
 * Runs a min operation with the given tex_b image and this image and returns a new RTex with the result
 * @param obj RTex object
 * @param tex_b second operand image
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
o_inline RTex *RTex_op_min(oobj obj, oobj tex_b, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_op_min_into(obj, tex_b, res);
    return res;
}

#endif //R_RTEX_MATH_H
