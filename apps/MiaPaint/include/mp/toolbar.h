#ifndef MP_TOOLBAR_H
#define MP_TOOLBAR_H

/**
 * @file toolbar.h
 *
 * Creates and manages the top toolbar
 */

#include "mp/main.h"
#include "mp/manager.h"


#define BRUSH_MAX_SIZE 5


struct mp_toolbar {
    oobj container;

    struct mp_options *options;
    struct mp_manager *manager;

    oobj theme;
    oobj gui;

    // tools
    oobj btn_exit;
    oobj btn_cam;
    oobj btn_clear;
    oobj btn_undo;
    oobj btn_redo;
    oobj btn_select;
    
    oobj btn_brush_prev;
    oobj btn_brush_type;
    oobj btn_brush_type_area;
    oobj btn_brush_next;

    oobj btn_tool_mode;
    
    oobj pane_select_create;
    oobj btn_select_create_create;
    oobj btn_select_create_move;
    oobj btn_select_create_copy;
    oobj btn_select_create_cut;

    oobj pane_select_paste;
    oobj btn_select_paste_rotate_left;
    oobj btn_select_paste_rotate_right;
    oobj btn_select_paste_mirror_h;
    oobj btn_select_paste_mirror_v;
    oobj btn_select_paste_color;
    oobj btn_select_paste_blend;
    oobj btn_select_paste_copy;
    oobj btn_select_paste_ok;


    // todo...
    int brush_size;
    bool brush_round;
    oobj brush_mats[BRUSH_MAX_SIZE*2];
    oobj brush_texs[BRUSH_MAX_SIZE*2];
    oobj brush_tex_current;
};


/**
 * Creates a new toolbar context
 * @param parent to inherit from
 */
struct mp_toolbar *mp_toolbar_new(oobj parent,
                                  struct mp_options *options,
                                  struct mp_manager *manager);

void mp_toolbar_update(struct mp_toolbar *self);

void mp_toolbar_render(struct mp_toolbar *self, oobj tex);


#endif //MP_TOOLBAR_H
