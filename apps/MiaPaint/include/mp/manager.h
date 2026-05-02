#ifndef MP_MANAGER_H
#define MP_MANAGER_H

/**
 * @file manager.h
 *
 * Shared context to connect all subsystem actions.
 * Like undo click from the tool button -> history undo
 */

#include "mp/history.h"
#include "mp/selection.h"
#include "mp/surface.h"
#include "mp/canvas.h"


struct mp_manager {
    oobj container;

    struct mp_history *history;
    struct mp_selection *selection;
    struct mp_surface *surface;
    oobj canvas;

    bool cam_was_fit_to_selection;
};


/**
 * Creates a new manager context
 * @param parent to inherit from
 */
struct mp_manager *mp_manager_new(oobj parent,
                                  struct mp_history *history,
                                  struct mp_selection *selection,
                                  struct mp_surface *surface,
                                  oobj canvas);


void mp_manager_exit(struct mp_manager *self);
void mp_manager_clear(struct mp_manager *self);
void mp_manager_camera(struct mp_manager *self);

o_inline bool mp_manager_undo_available(struct mp_manager *self)
{
    return mp_history_undo_available(self->history);
}
o_inline bool mp_manager_redo_available(struct mp_manager *self)
{
    return mp_history_redo_available(self->history);
}
void mp_manager_undo(struct mp_manager *self);
void mp_manager_redo(struct mp_manager *self);

o_inline enum mp_selection_state mp_manager_selection_state(struct mp_manager *self)
{
    return self->selection->state;
}
void mp_manager_selection_state_set(struct mp_manager *self, enum mp_selection_state state);

void mp_manager_tool_mode(struct mp_manager *self, enum mp_canvas_tool tool);

// pane selection create
void mp_manager_selection_copy(struct mp_manager *self, bool cut);

// pane selection paste
void mp_manager_selection_update_paste_position(struct mp_manager *self);
void mp_manager_selection_rotate(struct mp_manager *self, bool left);
void mp_manager_selection_mirror(struct mp_manager *self, bool h);
void mp_manager_selection_blend_set(struct mp_manager *self, bool blend);
void mp_manager_selection_paste(struct mp_manager *self, bool done);


// called in main
void mp_manager_update(struct mp_manager *self);

#endif //MP_MANAGER_H
