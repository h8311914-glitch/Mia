#ifndef M_MMAT_DRAW_H
#define M_MMAT_DRAW_H

/**
 * @file MMat_draw.h
 *
 * Object extension
 *
 * MMat_ namespace
 *
 * Drawing on a MMat
 *
 * @sa MMat_ex to import all MMat object extensions
 */

#include "MMat.h"


//
// Drawing
//


/**
 * clears the given sub rect to the given color buffer
 * @param obj MMat object
 * @param clear_data buffer color to set, needs to have m_format_size length
 * @note pixel byte size up to M_FORMAT_MAX_SIZE
 */
void MMat_clear_rect_x(oobj obj, m_ivec4 rect, const void *clear_data);


/**
 * clears the given sub rect to the given color, up to channel 4. remaining channels >4 are cleared to 0
 * @param obj MMat object
 * @param clear_color color to set
 * @note pixel byte size up to M_FORMAT_MAX_SIZE
 */
void MMat_clear_rect_vec4(oobj obj, m_ivec4 rect, m_vec4 clear_color);


/**
 * clears the full matrix up to channel 4 to clear color, remaining channels >4 are cleared to 0
 * @param obj MMat object
 * @param clear_color color to set
 * @note pixel byte size up to M_FORMAT_MAX_SIZE
 */
o_inline void MMat_clear_vec4(oobj obj, m_vec4 clear_color)
{
    MMat_clear_rect_vec4(obj, MMat_rect_int(obj, 0, 0), clear_color);
}



void MMat_border_into(oobj obj, oobj into, m_ivec4 border_ltrb, m_vec4 border_color);
struct oobj_opt MMat_border_try(oobj obj, m_ivec4 border_ltrb, m_vec4 border_color, ou32 format);
/**
 * Creates a new MMat copy with a border around it
 * @param obj MMat object
 * @param border_ltrb border size in pixels for left, top, right, bottom (clamped to >= 0)
 * @param border_color in rgba, only up to channel 4, remaining channels are cleared to 0
 * @param format for the new generated MMat
 */
o_inline MMat *MMat_border(oobj obj, m_ivec4 border_ltrb, m_vec4 border_color, ou32 format)
{
    MMat *self = MMat_border_try(obj, border_ltrb, border_color, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}

/**
 * Helper for MMat_collage to get the grid sizes
 * @param sources list of MMat objects of cols * rows size, use NULL to leave that grid position in border color.
 *                Uses row-major order
 * @param cols, rows image concat grid size. For concatenating in h, just use rows=1, etc.
 * @param out_col_sizes, out_row_sizes array of cols and row size, set to the individual grid row and column sizes
 */
void MMat_collage_get_grid_size(const oobj *sources, int cols, int rows, int *out_col_sizes, int *out_row_sizes);
void MMat_collage_into(oobj into, const oobj *sources, int cols, int rows,
    m_ivec2 spacing, m_ivec4 border_ltrb, m_vec4 bg_color, bool blend);
struct oobj_opt MMat_collage_try(oobj parent, const oobj *sources, int cols, int rows,
    m_ivec2 spacing, m_ivec4 border_ltrb, m_vec4 bg_color, bool blend, ou32 format);
/**
 * Creates a collage by concatenating sources in a grid.
 * @param parent to inherit from
 * @param sources list of MMat objects of cols * rows size, use NULL to leave that grid position in border color.
 *                Uses row-major order
 * @param cols, rows image concat grid size. For concatenating in h, just use rows=1, etc.
 * @param spacing in pixels between the images, cleared to border_color
 * @param border_ltrb outer border size in pixels for left, top, right, bottom (clamped to >= 0)
 * @param bg_color in rgba, only up to channel 4, remaining channels are cleared to 0
 * @param blend if true MMat_blend is used (on top of the bg color) else MMat_blit (faster)
 * @param format for the new generated MMat, if 0 (keep) its set to the first not NULL sources MMat
 * @note The grid column and row sizes depent on the maximal item size in that row or column.
 *       Items are aligned left top if size is smaller than row or column grid size.
 */
o_inline MMat *MMat_collage(oobj parent, const oobj *sources, int cols, int rows,
    m_ivec2 spacing, m_ivec4 border_ltrb, m_vec4 bg_color, bool blend, ou32 format)
{
    MMat *self = MMat_collage_try(parent, sources, cols, rows, spacing, border_ltrb, bg_color, blend, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}




void MMat_replace_x(oobj obj, const void *restrict color_from, const void *restrict color_to);
/**
 * Replaces a color with a new one.
 * @param obj MMat object
 * @param color_from will be searched
 * @param color_to will be applied on all color_from's
 */
void MMat_replace(oobj obj, m_vec4 color_from, m_vec4 color_to);



// dont forget to o_del
oobj MMat_fill_task_x(oobj obj, int x, int y, bool mode8, const void *fill_color, int opt_step_runs);

void MMat_fill_x(oobj obj, int x, int y, bool mode8, const void *fill_color);

void MMat_fill_vec4(oobj obj, int x, int y, bool mode8, m_vec4 fill_color);



//
// draw functions
//


/**
 * Calls MMat_blend_dab for each dab in dabs.
 * @param obj MMat object
 * @param color for the brush
 * @param dabs, num array of brush centers for the stroke
 * @param opt_brush either an MMat object used as brush, or NULL to use a single white pixel instead
 */
void MMat_draw_dabs(oobj obj, m_vec4 color, const m_vec2 *dabs, int num, oobj opt_brush);

/**
 * Draws a single dab dot.
 * @param obj MMat object
 * @param color for the brush
 * @param pos to stroke
 * @param opt_brush either an MMat object used as brush, or NULL to use a single white pixel instead
 */
o_inline void MMat_draw_dot(oobj obj, m_vec4 color, m_vec2 pos, oobj opt_brush)
{
    MMat_draw_dabs(obj, color, &pos, 1, opt_brush);
}

/**
 * Draws a line stroke. (a line segment, not an infinite line)
 * @param obj MMat object
 * @param color for the brush
 * @param pos_a, pos_b line end points
 * @param opt_brush either an MMat object used as brush, or NULL to use a single white pixel instead
 * @note uses m_discr_grid_line for a pixel perfect line
 */
void MMat_draw_line(oobj obj, m_vec4 color, m_vec2 pos_a, m_vec2 pos_b, oobj opt_brush);

/**
 * Draws an aabb box border stroke
 * @param obj MMat object
 * @param color for the brush
 * @param pos_a, pos_b opposite end points
 * @param opt_brush either an MMat object used as brush, or NULL to use a single white pixel instead
 * @note uses m_discr_grid_box for a pixel perfect aabb box
 */
void MMat_draw_box(oobj obj, m_vec4 color, m_vec2 pos_a, m_vec2 pos_b, oobj opt_brush);

/**
 * Draws a full circle stroke
 * @param obj MMat object
 * @param color for the brush
 * @param center, radius for the circle
 * @param opt_brush either an MMat object used as brush, or NULL to use a single white pixel instead
 * @note uses m_discr_grid_circle for a pixel perfect full circle
 */
void MMat_draw_circle(oobj obj, m_vec4 color, m_vec2 center, float radius, oobj opt_brush);

/**
 * Draws a circle bow stroke
 * @param obj MMat object
 * @param color for the brush
 * @param center, radius for the circle
 * @param a_rad, b_rad the bow end arcs
 * @param opt_brush either an MMat object used as brush, or NULL to use a single white pixel instead
 * @note uses m_discr_grid_circle for a pixel perfect full circle bow
 */
void MMat_draw_circle_bow(oobj obj, m_vec4 color, m_vec2 center, float radius, float a_rad, float b_rad, oobj opt_brush);

#endif //M_MMAT_DRAW_H
