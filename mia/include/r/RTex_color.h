#ifndef R_RTEX_COLOR_H
#define R_RTEX_COLOR_H

/**
 * @file RTex_color.h
 *
 * Object extension
 *
 * RTex_ namespace
 *
 * Recoloring an RTex
 *
 * @sa RTex_ex.h to import all RTex object extensions
 */

#include "RTex.h"



/**
 * Colorize the RTex into a new one
 * @param obj RTex object
 * @param rgba * color
 * @param hsva + color
 * @param into RTex object to render into
 */
void RTex_color_into(oobj obj, vec4 rgba, vec4 hsva, oobj into);

/**
 * Colorize the RTex into a new one
 * @param obj RTex object
 * @param rgba * color
 * @param hsva + color
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
o_inline RTex *RTex_color(oobj obj, vec4 rgba, vec4 hsva, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_color_into(obj, rgba, hsva, res);
    return res;
}

/**
 * RGBA -> HSVA
 * @param obj RTex object
 * @param into RTex object to render into
 */
void RTex_rgba_to_hsva_into(oobj obj, oobj into);

/**
 * RGBA -> HSVA
 * @param obj RTex object
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
o_inline RTex *RTex_rgba_to_hsva(oobj obj, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_rgba_to_hsva_into(obj, res);
    return res;
}

/**
 * HSVA -> RGBA
 * @param obj RTex object
 * @param into RTex object to render into
 */
void RTex_hsva_rgba_into(oobj obj, oobj into);

/**
 * HSVA -> RGBA
 * @param obj RTex object
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
o_inline RTex *RTex_hsva_rgba(oobj obj, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_hsva_rgba_into(obj, res);
    return res;
}


#endif //R_RTEX_COLOR_H
