#ifndef R_RTEX_IMGPROC_H
#define R_RTEX_IMGPROC_H

/**
 * @file RTex_imgproc.h
 *
 * Object extension
 *
 * RTex_ namespace
 *
 * Image processing functions
 *
 * @sa RTex_ex.h to import all RTex object extensions
 */

#include "RTex.h"


/**
 * Runs a convolution with the given kernel on this image and returns a new RTex with the result
 * @param obj RTex object
 * @param kernel RTex object (.r used) for example RTex_new_kernel
 * @param offset for the kernel
 * @param into RTex object to render into
 */
void RTex_conv_into(oobj obj, oobj kernel, ivec2 offset, oobj into);


/**
 * Runs a convolution with the given kernel on this image and returns a new RTex with the result
 * @param obj RTex object
 * @param kernel RTex object (.r used) for example RTex_new_kernel
 * @param offset for the kernel
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
o_inline RTex *RTex_conv(oobj obj, oobj kernel, ivec2 offset, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_conv_into(obj, kernel, offset, res);
    return res;
}


/**
 * Runs a convolution with the given kernel on this image and returns a new RTex with the result
 * @param obj RTex object
 * @param size for the kernel, asserts max
 * @param into RTex object to render into
 * @note calls RTex_conv internally
 */
void RTex_blur_into(oobj obj, ivec2 size, oobj into);


/**
 * Runs a convolution with the given kernel on this image and returns a new RTex with the result
 * @param obj RTex object
 * @param size for the kernel, asserts max
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 * @note calls RTex_conv internally
 */
o_inline RTex *RTex_blur(oobj obj, ivec2 size, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_blur_into(obj, size, res);
    return res;
}


/**
 * Runs a convolution with the given kernel on this image and returns a new RTex with the result
 * @param obj RTex object
 * @param size for the kernel, asserts max
 * @param sigma gauss factor for x and y axis, pass <=0 to auto scale (vec2_(-1))
 * @param into RTex object to render into
 * @note calls RTex_conv internally
 */
void RTex_gauss_into(oobj obj, ivec2 size, vec2 sigma, oobj into);


/**
 * Runs a convolution with the given kernel on this image and returns a new RTex with the result
 * @param obj RTex object
 * @param size for the kernel, asserts max
 * @param sigma gauss factor for x and y axis, pass <=0 to auto scale (vec2_(-1))
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 * @note calls RTex_conv internally
 */
o_inline RTex *RTex_gauss(oobj obj, ivec2 size, vec2 sigma, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_gauss_into(obj, size, sigma, res);
    return res;
}


/**
 * Runs a contouring shader and creates the contour based on the masked rgba values in the given color.
 * @param obj RTex object
 * @param kernel RTex object (.r used) for example RTex_new_kernel
 * @param offset for the kernel
 * @param mask vec4_(0, 0, 0, 1) to mask alpha (default)
 * @param color to render with the kernel
 * @param into RTex object to render into
 */
void RTex_dilate_into(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, oobj into);


/**
 * Runs a contouring shader and creates the contour based on the masked rgba values in the given color.
 * @param obj RTex object
 * @param kernel RTex object (.r used) for example RTex_new_kernel
 * @param offset for the kernel
 * @param mask vec4_(0, 0, 0, 1) to mask alpha (default)
 * @param color to render with the kernel
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
o_inline RTex *RTex_dilate(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_dilate_into(obj, kernel, offset, mask, color, res);
    return res;
}

/**
 * Runs a contouring "inv" shader and creates the contour based on the masked rgba values in the given color
 * @param obj RTex object
 * @param kernel RTex object (.r used) for example RTex_new_kernel
 * @param offset for the kernel
 * @param mask vec4_(0, 0, 0, 1) to mask alpha (default)
 * @param color to render with the kernel
 * @param into RTex object to render into
 */
void RTex_erode_into(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, oobj into);

/**
 * Runs a contouring "inv" shader and creates the contour based on the masked rgba values in the given color
 * @param obj RTex object
 * @param @param kernel RTex object (.r used) for example RTex_new_kernel
 * @param offset for the kernel
 * @param mask vec4_(0, 0, 0, 1) to mask alpha (default)
 * @param color to render with the kernel
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
o_inline RTex *RTex_erode(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_erode_into(obj, kernel, offset, mask, color, res);
    return res;
}

/**
 * Runs a contouring shader and creates the contour based on the masked rgba values in the given color.
 * Then blends on the obj RTex
 * @param obj RTex object
 * @param kernel RTex object (.r used) for example RTex_new_kernel
 * @param offset for the kernel
 * @param mask vec4_(0, 0, 0, 1) to mask alpha (default)
 * @param color to render with the kernel
 * @param into RTex object to render into
 * @note uses RTex_dilate and blends the obj RTex onto the dilated contour
 */
void RTex_contour_into(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, oobj into);

/**
 * Runs a contouring shader and creates the contour based on the masked rgba values in the given color.
 * Then blends on the obj RTex
 * @param obj RTex object
 * @param kernel RTex object (.r used) for example RTex_new_kernel
 * @param offset for the kernel
 * @param mask vec4_(0, 0, 0, 1) to mask alpha (default)
 * @param color to render with the kernel
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 * @note uses RTex_dilate and blends the obj RTex onto the dilated contour
 */
o_inline RTex *RTex_contour(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_contour_into(obj, kernel, offset, mask, color, res);
    return res;
}


#endif //R_RTEX_IMGPROC_H
