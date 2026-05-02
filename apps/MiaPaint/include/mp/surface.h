#ifndef MP_SURFACE_H
#define MP_SURFACE_H

/**
 * @file surface.h
 *
 * Shared context for canvas drawing
 */

#include "o/common.h"
#include "m/types/int.h"
#include "m/types/flt.h"

#define MP_SURFACE_HISTORY_MAX 16

// forward
struct mp_history;

struct mp_surface {
    oobj container;

    vec4 color;
    vec4 color_2nd;
    oobj brush;
    float dashing;

    oobj draw_dabs_offset_array;

    // MMat of the full canvas
    oobj canvas_mat;
    // sub view MMat of canvas_mat, to draw on
    oobj canvas_mat_view;
    vec2 canvas_mat_view_offset;
    ivec2 canvas_mat_view_offset_int;

    // RTex of canvas_mat to render, may be dirty (see below)
    oobj canvas_tex;
    bool canvas_dirty;

    // MMat to blend or blit, may be NULL
    oobj paste_mat;
    ivec2 paste_mat_offset;
    bool paste_mat_blend;
};


/**
 * Creates a new surface with functions to draw on (MMat drawing, RTex generated for rendering)
 * @param parent to inherit from
 * @param history api to add surface history
 */
struct mp_surface *mp_surface_new(oobj parent, struct mp_history *history);

/**
 * @return canvas_tex, but updated if dirty
 */
oobj mp_surface_canvas_tex(struct mp_surface *self);

/**
 * Swaps color and color_2nd
 */
void mp_surface_color_swap(struct mp_surface *self);


/**
 * Sets self->canvas_mat_view to either a selection view or to self->canvas_mat
 * @param view_rect ltwh rect from canvas_mat origin or ivec4_(0) to turn off / for all
 */
void mp_surface_view_rect_set(struct mp_surface *self, ivec4 view_rect);

void mp_surface_update(struct mp_surface *self);

//
// tools
//

/**
 * Clears the whole image to transparent
 */
void mp_surface_clear(struct mp_surface *self);

/**
 * @param out_picked the picked result (untouched if pos out of bounds)
 * @param pos to select a color from
 * @return true on succes (pos in image bounds)
 */
bool mp_surface_pick(struct mp_surface *self, vec4 *out_picked, vec2 pos);

/**
 * Draws num dabs to the surface
 */
void mp_surface_draw_dabs(struct mp_surface *self, const vec2 *dabs, int num);

/**
 * Draws a dot / single dab on a pos
 * @param pos draw position
 */
void mp_surface_draw_dot(struct mp_surface *self, vec2 pos);

/**
 * Draws a line from a to b
 * @param a, b line end points, a==b draws a single point
 */
void mp_surface_draw_line(struct mp_surface *self, vec2 a, vec2 b);

/**
 * Draws an aabb outline from a to b
 * @param a, b rect end points, a==b draws a single point
 */
void mp_surface_draw_box(struct mp_surface *self, vec2 a, vec2 b);

/**
 * Draws a circle with center and radius
 * @param center is the circle center
 * @param radius for the circle <= 0 to draw a single point
 */
void mp_surface_draw_circle(struct mp_surface *self, vec2 center, float radius);

/**
 * Fills in a blob or line
 * @param pos to begin the filling
 * @param mode8 false -> mode4: uses left top right bottom neighboors (fill blobs)
 *              true: also uses diagonals (replace line/border color)
 */
void mp_surface_fill(struct mp_surface *self, vec2 pos, bool mode8);

/**
 * Replaces all colors from pos' color to the selected
 * @param pos to select the color from being replaced
 */
void mp_surface_replace(struct mp_surface *self, vec2 pos);


//
// paste_mat
//

/**
 * Pastes the current MMat paste_mat onto the canvas.
 * Either blends or blit, according to paste_mat_blend.
 */
void mp_surface_paste_draw(struct mp_surface *self);

/**
 * Copies the given rect from canvas_mat into paste_mat
 * @param rect ltwh in canvas_mat, clamped, width<=0 || height<=0 to ignore
 * @param cut if true, clears the rect on canvas_mat after copying.
 */
void mp_surface_paste_copy(struct mp_surface *self, ivec4 rect, bool cut);

void mp_surface_paste_rotate(struct mp_surface *self, bool left);
void mp_surface_paste_mirror(struct mp_surface *self, bool h);

#endif //MP_SURFACE_H
