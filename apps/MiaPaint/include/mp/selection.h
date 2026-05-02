#ifndef MP_SELECTION_H
#define MP_SELECTION_H

/**
 * @file selection.h
 *
 * Shared context for a canvas selection and pasting
 */

#include "o/common.h"
#include "m/types/int.h"
#include "m/types/flt.h"
#include "m/types/byte.h"

// forward
struct mp_history;

enum mp_selection_state {
    mp_selection_state_OFF,
    mp_selection_state_CREATE,
    mp_selection_state_MOVE,
    mp_selection_state_APPLY,
    mp_selection_state_PASTE,
    mp_selection_state_ENUM_MAX,
};

struct mp_selection {
    oobj container;

    // commits on change
    struct mp_history *history;

    ivec4 canvas_rect;

    // read only, see mp_selection_state_set
    enum mp_selection_state state;

    // selection as rect left, top, width, height
    // w, h <= 0 if not created yet
    ivec4 rect;

    // l, t, <0 if not created yet
    ivec2 create_start;

    ivec2 move_offset;

    // 4 boxes as ltrb borders
    oobj border_ro;
    float border_t;
    // ltrb bool
    bvec4 border_moved;
};


/**
 * Creates a new selection with functions to draw on (MMat drawing, RTex generated for rendering)
 * @param parent to inherit from
 * @param history api to add selection history
 */
struct mp_selection *mp_selection_new(oobj parent, struct mp_history *history);

/**
 * Switches states.
 * CREATE -> resets the rect and waits for a new box rect
 * MOVE, APPLY, PASTE -> only applied if created yet
 */
void mp_selection_state_set(struct mp_selection *self, enum mp_selection_state state);

/**
 * @return true on states that handle the pointer
 */
o_inline bool mp_selection_uses_pointer(struct mp_selection *self)
{
    return self->state == mp_selection_state_CREATE
            || self->state == mp_selection_state_MOVE
            || self->state == mp_selection_state_PASTE;
}

/**
 * @return the selected rect if mode is mp_selection_state_APPLY, else ivec4_(0)
 */
o_inline ivec4 mp_selection_rect_applied(struct mp_selection *self)
{
    if (self->state != mp_selection_state_APPLY) {
        return ivec4_(0);
    }
    return self->rect;
}

/**
 * Swaps width and height
 */
void mp_selection_rotate(struct mp_selection *self);


/**
 * updates the selection rect, depending on the state:
 * OFF      -> NOOP
 * CREATE   -> use pointers to create the selection like a box
 * MOVE     -> pointer on edge -> resized move
 *              pointer in center -> move relative
 *              pointer outside -> move center to pointer
 *              The rect stays within the canvas limits
 * APPLY    -> NOOP
 * PASTE    -> pointer in center -> move relative
 *              pointer outside -> move center to pointer
 *              The rect may go outside the canvas limits
 * @note called by canvas
 */
void mp_selection_update(struct mp_selection *self, ivec4 canvas_rect);

/**
 * Renders the selection borders.
 * @param tex to be rendered to, passed from AView render fn.
 * @note called by canvas
 */
void mp_selection_render(struct mp_selection *self, oobj tex);





#endif //MP_SELECTION_H
