#include "mp/manager.h"

#include "a/common.h"

struct mp_manager *mp_manager_new(oobj parent,
                                  struct mp_history *history,
                                  struct mp_selection *selection,
                                  struct mp_surface *surface,
                                  oobj canvas)
{
    oobj container = OObj_new(parent);
    struct mp_manager *self = o_new0(container, *self, 1);
    self->container = container;

    self->history = history;
    self->selection = selection;
    self->surface = surface;
    self->canvas = canvas;

    return self;
}


void mp_manager_exit(struct mp_manager *self)
{
    a_scene_exit();
}

void mp_manager_clear(struct mp_manager *self)
{
    mp_surface_clear(self->surface);
    mp_history_commit(self->history);
}

void mp_manager_camera(struct mp_manager *self)
{
    // flip between canvas fit and selection fit, if selection is active
    bool fit_to_selection = false;
    if (!self->cam_was_fit_to_selection && self->selection->rect.width>0 && self->selection->rect.height>0) {
        fit_to_selection = true;
    }
    mp_canvas_cam_home(self->canvas, fit_to_selection);
    self->cam_was_fit_to_selection = fit_to_selection;
}

void mp_manager_undo(struct mp_manager *self)
{
    mp_history_undo(self->history, 0);
}
void mp_manager_redo(struct mp_manager *self)
{
    mp_history_redo(self->history);
}


void mp_manager_selection_state_set(struct mp_manager *self, enum mp_selection_state state)
{
    mp_selection_state_set(self->selection, state);
}
void mp_manager_tool_mode(struct mp_manager *self, enum mp_canvas_tool tool)
{
    mp_canvas_tool_set(self->canvas, tool);
}


void mp_manager_selection_copy(struct mp_manager *self, bool cut)
{
    mp_surface_paste_copy(self->surface, self->selection->rect, cut);
    mp_selection_state_set(self->selection, mp_selection_state_PASTE);
}

void mp_manager_selection_update_paste_position(struct mp_manager *self)
{
    self->surface->paste_mat_offset = self->selection->rect.xy;
}
void mp_manager_selection_rotate(struct mp_manager *self, bool left)
{
    mp_surface_paste_rotate(self->surface, left);
    mp_selection_rotate(self->selection);
}
void mp_manager_selection_mirror(struct mp_manager *self, bool h)
{
    mp_surface_paste_mirror(self->surface, h);
}
void mp_manager_selection_blend_set(struct mp_manager *self, bool blend)
{
    self->surface->paste_mat_blend = blend;
}
void mp_manager_selection_paste(struct mp_manager *self, bool done)
{
    mp_history_commit(self->history);
    if (done) {
        o_del(self->surface->paste_mat);
        mp_selection_state_set(self->selection, mp_selection_state_OFF);
    }
}



void mp_manager_update(struct mp_manager *self)
{
    if (self->selection->state == mp_selection_state_PASTE) {
        if (!self->surface->paste_mat) {
            mp_selection_state_set(self->selection, mp_selection_state_OFF);
        }
        self->surface->paste_mat_offset = self->selection->rect.xy;
    }
}
