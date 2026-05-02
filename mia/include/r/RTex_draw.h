#ifndef R_RTEX_DRAW_H
#define R_RTEX_DRAW_H

/**
 * @file RTex_draw.h
 *
 * Object extension
 *
 * RTex_ namespace
 *
 * Drawing on a RTex
 *
 * @sa RTex_ex.h to import all RTex object extensions
 */

#include "RTex.h"


/**
 * Runs a outlining contour shader and creates the contour based on the masked rgba values in the given color.
 * Creates a plus kernel and runs RTex_contour_into with it.
 * Then blends on the obj RTex
 * @param obj RTex object
 * @param size of the kernel
 * @param offset for the kernel
 * @param mask vec4_(0, 0, 0, 1) to mask alpha (default)
 * @param color to render with the kernel
 * @param into RTex object to render into
 * @note uses RTex_contour_into
 */
void RTex_outline_into(oobj obj, ivec2 size, ivec2 offset, vec4 mask, vec4 color, oobj into);

/**
 * Runs a outlining contour shader and creates the contour based on the masked rgba values in the given color.
 * Creates a plus kernel and runs RTex_contour_into with it.
 * Then blends on the obj RTex
 * @param obj RTex object
 * @param size of the kernel
 * @param offset for the kernel
 * @param mask vec4_(0, 0, 0, 1) to mask alpha (default)
 * @param color to render with the kernel
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 * @note uses RTex_contour_into
 */
o_inline RTex *RTex_outline(oobj obj, ivec2 size, ivec2 offset, vec4 mask, vec4 color, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_outline_into(obj, size, offset, mask, color, res);
    return res;
}


/**
 * Creates a collage of all src RTex's
 * @param srcs array of RTex objects
 * @param n size of the srcs array; asserts(n>=1)
 * @param cols columns, row major order, starting from top left
 * @param spacing additional border around an RTex sub image
 * @param bg_color background and border color
 * @param format for the resulting tex, if 0 it will use the format of srcs[0]
 * @return RTex allocated on srcs[0]
 * @note the grid size is formed by the maximal RTex width and height of all srcs + margin*2
 *       "into" version does not make much sense...
 */
RTex *RTex_collage(const oobj *srcs, int n, int cols, ivec2 margin, vec4 bg_color, ou32 format);


/**
 * Creates a new copy of the RTex obj, with the given border around it
 * @param obj RTex object
 * @param lrbt border size in left, right, bottom, top (e. g. ivec4_(8) for all to be 8 pixels wide)
 * @param color border color, for example R_TRANSPARENT, or R_BLACK
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj
 */
RTex *RTex_border(oobj obj, ivec4 lrbt, vec4 color, ou32 format);



//
// draw functions
//


/**
 * Calls RTex_blend_dab for each dab in dabs.
 * @param obj RTex object
 * @param color for the brush
 * @param dabs, num array of brush centers for the stroke
 * @param opt_brush either an RTex object used as brush, or NULL to use a single white pixel instead
 * @note does NOT use QuadDab shader btw.
 */
void RTex_draw_dabs(oobj obj, vec4 color, const vec2 *dabs, int num, oobj opt_brush);

/**
 * Draws a single dab dot.
 * @param obj RTex object
 * @param color for the brush
 * @param pos to stroke
 * @param opt_brush either an RTex object used as brush, or NULL to use a single white pixel instead
 */
o_inline void RTex_draw_dot(oobj obj, vec4 color, vec2 pos, oobj opt_brush)
{
    RTex_draw_dabs(obj, color, &pos, 1, opt_brush);
}

/**
 * Draws a line stroke. (a line segment, not an infinite line)
 * @param obj RTex object
 * @param color for the brush
 * @param pos_a, pos_b line end points
 * @param opt_brush either an RTex object used as brush, or NULL to use a single white pixel instead
 * @note uses m_discr_grid_line for a pixel perfect line
 */
void RTex_draw_line(oobj obj, vec4 color, vec2 pos_a, vec2 pos_b, oobj opt_brush);

/**
 * Draws an aabb box border stroke
 * @param obj RTex object
 * @param color for the brush
 * @param pos_a, pos_b opposite end points
 * @param opt_brush either an RTex object used as brush, or NULL to use a single white pixel instead
 * @note uses m_discr_grid_box for a pixel perfect aabb box
 */
void RTex_draw_box(oobj obj, vec4 color, vec2 pos_a, vec2 pos_b, oobj opt_brush);

/**
 * Draws a full circle stroke
 * @param obj RTex object
 * @param color for the brush
 * @param center, radius for the circle
 * @param opt_brush either an RTex object used as brush, or NULL to use a single white pixel instead
 * @note uses m_discr_grid_circle for a pixel perfect full circle
 */
void RTex_draw_circle(oobj obj, vec4 color, vec2 center, float radius, oobj opt_brush);

/**
 * Draws a circle bow stroke
 * @param obj RTex object
 * @param color for the brush
 * @param center, radius for the circle
 * @param a_rad, b_rad the bow end arcs
 * @param opt_brush either an RTex object used as brush, or NULL to use a single white pixel instead
 * @note uses m_discr_grid_circle for a pixel perfect full circle bow
 */
void RTex_draw_circle_bow(oobj obj, vec4 color, vec2 center, float radius, float a_rad, float b_rad, oobj opt_brush);


#endif //R_RTEX_DRAW_H
