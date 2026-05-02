#include "mp/toolbar.h"

#include "o/log.h"
#include "m/MMat_ex.h"
#include "r/RTex.h"
#include "u/color.h"
#include "w/w.h"
#include "x/cursor.h"

static char *TOOL_MODES[] = {
    "PICK",
    "FREE",
    "LINE",
    "BOX",
    "CIRC",
    "FILL4",
    "FILL8",
    "REPL",
    NULL
};
_Static_assert(o_stack_array_num(TOOL_MODES)-1 == mp_canvas_TOOL_ENUM_MAX, "must match");



#define BRUSH_MAX_SIZE 5

static oobj brush_create_square(oobj parent, int size)
{
    oobj mat = MMat_new(parent, NULL, size, size, M_FORMAT_RGBA_U8);
    MMat_clear_vec4(mat, R_WHITE);
    return mat;
}

static oobj brush_create_round(oobj parent, int size)
{
    oobj mat = MMat_new(parent, NULL, size, size, M_FORMAT_RGBA_U8);
    float max_radi = size/2.0-0.1;
    vec2 center = vec2_(size/2.0);
    for (int r=0; r<size; r++) {
        for (int c=0; c<size; c++) {
            bvec4 *at = (bvec4*) MMat_at(mat, c, r);
            float radi = vec2_distance(center, vec2_(c+0.5, r+0.5));
            if (radi <= max_radi) {
                *at = U_WHITE;
            } else {
                *at = U_TRANSPARENT;
            }
        }
    }
    return mat;
}

static void brush_set(struct mp_toolbar *self)
{
    self->brush_size = o_clamp(self->brush_size, 1, BRUSH_MAX_SIZE);
    o_log("size: %i, round: %i", self->brush_size, self->brush_round);
    int brush_idx = (self->brush_size-1) * 2 + (self->brush_round? 1 : 0);
    assert(brush_idx>=0 && brush_idx<=BRUSH_MAX_SIZE*2);
    self->brush_tex_current = self->brush_texs[brush_idx];
    self->manager->surface->brush = self->brush_mats[brush_idx];
}

//
// public
//

struct mp_toolbar *mp_toolbar_new(oobj parent,
                                  struct mp_options *options,
                                  struct mp_manager *manager)
{
    oobj container = OObj_new(parent);
    struct mp_toolbar *self = o_new0(container, *self, 1);
    self->container = container;

    self->options = options;
    self->manager = manager;
    
    self->theme = WTheme_new_tiny(self->container);
    self->gui = WObj_new(self->container);

    oobj main_vbox = WBox_new(self->gui, WBox_V);
    oobj box = WBox_new(main_vbox, WBox_H_V);
    WObj_padding_set(box, vec4_(2));
    WBox_spacing_set(box, vec2_(2));

    
    oobj tool, icon;

    
    
    oobj style = WObj_style(box);
    WStyle_btn_style_set(style, WBtn_DEF_ROUND);

    if (self->options->show_exit_btn) {
        tool = WBtn_new(box);
        oobj lbl = WText_new(tool, " X ");
        WText_color_set(lbl, R_RED);
        self->btn_exit = tool;
    }

    // tool = WBtn_new(self->box);
    // WIcon_new(tool, WTheme_ICON_SAVE);
    //
    // tool = WBtn_new(self->box);
    // WIcon_new(tool, WTheme_ICON_LOAD);
    
    
    self->btn_cam = WBtn_new_text(box, "CAM", NULL);
    self->btn_clear = WBtn_new_text(box, "CL", NULL);
    
    oobj undo_redo_box = WBox_new_h(box);
    
    self->btn_undo = WBtn_new_text(undo_redo_box, "UNDO", NULL);
    self->btn_redo = WBtn_new_text(undo_redo_box, "REDO", NULL);
    WBtn_style_set(self->btn_undo, WBtn_DEF_ROUND_DUAL_R);
    WBtn_style_set(self->btn_redo, WBtn_DEF_ROUND_DUAL_L);
    
    self->btn_select = WBtn_new_text(box, "SEL", NULL);
    
    
    WStyle_btn_color_set(style, vec4_(0.6, 0.8, 0.6, 1.0));
    
    self->btn_brush_prev = WBtn_new_text(box, " - ", NULL);
    
    self->btn_brush_type = WBtn_new(box);
    self->btn_brush_type_area = WObj_new(self->btn_brush_type);
    WObj_fixed_size_set(self->btn_brush_type_area, vec2_(5));
    
    self->btn_brush_next = WBtn_new_text(box, " + ", NULL);
    

    WStyle_btn_color_set(style, vec4_(0.6, 0.6, 0.8, 1.0));
    //WStyle_btn_style_set(style, WBtn_DEF_ROUND);


    self->btn_tool_mode = WCombo_new_text(box, self->gui, TOOL_MODES, 0, 1);
    mp_manager_tool_mode(self->manager, mp_canvas_TOOL_FREE);

    WStyle_btn_color_set(style, vec4_(0.6, 0.8, 0.6, 0.75));
    
    oobj toolbar_align = WAlign_new_center_h(main_vbox);
    self->pane_select_create = WPane_new(toolbar_align);
    WPane_color_set(self->pane_select_create, vec4_(0.3, 0.5, 0.3, 0.5));
    WPane_style_set(self->pane_select_create, WPane_ROUND);
    WObj_hide_set(self->pane_select_create, true);
    oobj restr = WRestrict_new(self->pane_select_create);
    oobj toolbar_box = WBox_new_h(restr);
    WBox_spacing_set(toolbar_box, vec2_(1));
    
    oobj label = WTextShadow_new(toolbar_box, "SELECT:");
    WObj_padding_ref(label)->v1 = 2;
    self->btn_select_create_create = WBtn_new_text(toolbar_box, "NEW", NULL);
    self->btn_select_create_move = WBtn_new_text(toolbar_box, "MOV", NULL);
    self->btn_select_create_copy = WBtn_new_text(toolbar_box, "CPY", NULL);
    self->btn_select_create_cut = WBtn_new_text(toolbar_box, "CUT", NULL);
    
    self->pane_select_paste = WPane_new(toolbar_align);
    WPane_color_set(self->pane_select_paste, vec4_(0.3, 0.5, 0.3, 0.5));
    WPane_style_set(self->pane_select_paste, WPane_ROUND);
    WObj_hide_set(self->pane_select_paste, true);
    restr = WRestrict_new(self->pane_select_paste);
    toolbar_box = WBox_new_h(restr);
    WBox_spacing_set(toolbar_box, vec2_(1));
    
    label = WTextShadow_new(toolbar_box, "SELECT:");
    WObj_padding_ref(label)->v1 = 2;
    oobj vbox = WBox_new_v(toolbar_box);
    self->btn_select_paste_rotate_left = WBtn_new_text(vbox, "ROT L", NULL);
    self->btn_select_paste_rotate_right = WBtn_new_text(vbox, "ROT R", NULL);
    vbox = WBox_new_v(toolbar_box);
    self->btn_select_paste_mirror_h = WBtn_new_text(vbox, "MIR H", NULL);
    self->btn_select_paste_mirror_v = WBtn_new_text(vbox, "MIR V", NULL);
    vbox = WBox_new_v(toolbar_box);
    self->btn_select_paste_color = WBtn_new_text(vbox, "COLOR", NULL);
    self->btn_select_paste_blend = WBtn_new_text(vbox, "BLEND", NULL);
    vbox = WBox_new_v(toolbar_box);
    self->btn_select_paste_copy = WBtn_new_text(vbox, "CPY", NULL);
    self->btn_select_paste_ok = WBtn_new_text(vbox, "OK", NULL);


    // tool = WBtn_new(self->box);
    // WText_new(tool, "-");

    // tool = WImg_new(self->box, u_atlas_rect(WTheme_atlas(self->theme), WTheme_CUSTOM_8));

    // tool = WBtn_new(self->box);
    // WText_new(tool, "+");

    // tool = WBtn_new(self->box);
    // WText_new(tool, "SEL");

    // tool = WBtn_new(self->box);
    // WText_new(tool, "PASTE");

    
    
    for(int i=0; i<BRUSH_MAX_SIZE; i++) {
        int size = i+1;
        oobj square = brush_create_square(self->container, size);
        oobj round = brush_create_round(self->container, size);

        self->brush_mats[i*2] = square;
        self->brush_mats[i*2+1] = round;
        self->brush_texs[i*2] = RTex_new_mat(self->container, square, 0);
        self->brush_texs[i*2+1] = RTex_new_mat(self->container, round, 0);;
    }

    self->brush_size = 1;
    self->brush_round = true;
    brush_set(self);

    return self;
}

void mp_toolbar_update(struct mp_toolbar *self)
{
    WTheme_update_full_scene(self->theme, self->gui, true);
    // in case the tools are used, 
    // abort starting the (multitouch) cursor
    if(a_pointer_was_handled(0, 0)) {
        x_cursor_start_abort();
    }
    // w stuff only sets current pointer handled,
    //     so we forward that to the full history.
    //     (which is used in the canvas for drawing)
    a_pointer_handled_forward_history(0);


    
    if (self->btn_exit && WBtn_clicked(self->btn_exit)) {
        mp_manager_exit(self->manager);
    }

    if (WBtn_clicked(self->btn_cam)) {
        mp_manager_camera(self->manager);
    }

    if (WBtn_clicked(self->btn_clear)) {
        mp_manager_clear(self->manager);
    }

    WObj_enable_set(self->btn_undo, mp_manager_undo_available(self->manager));
    if (mp_manager_undo_available(self->manager)) {
        if (WBtn_clicked(self->btn_undo)) {
            o_log("undo clicked");
            mp_manager_undo(self->manager);
        }
    } else {
        WBtn_down_set(self->btn_undo, true);
    }

    WObj_enable_set(self->btn_redo, mp_manager_redo_available(self->manager));
    if (mp_manager_redo_available(self->manager)) {
        if (WBtn_clicked(self->btn_redo)) {
            o_log("redo clicked");
            mp_manager_redo(self->manager);
        }
    } else {
        WBtn_down_set(self->btn_redo, true);
    }

    enum mp_selection_state selection_state = mp_manager_selection_state(self->manager);
    WBtn_down_set(self->btn_select, selection_state != mp_selection_state_OFF);
    if(WBtn_toggled(self->btn_select)) {
        o_log("select toggled");
        if (WBtn_down(self->btn_select)) {
            mp_manager_selection_state_set(self->manager, mp_selection_state_CREATE);
        } else {
            mp_manager_selection_state_set(self->manager, mp_selection_state_OFF);
        }
    }
    
    // brush tools
    WObj_enable_set(self->btn_brush_prev, self->brush_size>1);
    if(self->brush_size>1) {
        if (WBtn_clicked(self->btn_brush_prev)) {
            self->brush_size--;
            brush_set(self);
        }
        if(WBtn_long_pressed(self->btn_brush_prev)) {
            o_log("first brush");
            self->brush_size = 0;
            brush_set(self);
        
        }
    } else {
        WBtn_down_set(self->btn_brush_prev, true);
        WBtn_down_timer_reset(self->btn_brush_prev);
    }
    
    if(WBtn_clicked(self->btn_brush_type)) {
        self->brush_round = !self->brush_round;
        brush_set(self);
    }
    
    WObj_enable_set(self->btn_brush_next, self->brush_size<BRUSH_MAX_SIZE);
    if(self->brush_size<BRUSH_MAX_SIZE) {
        if (WBtn_clicked(self->btn_brush_next)) {
            self->brush_size++;
            brush_set(self);
        }
        if(WBtn_long_pressed(self->btn_brush_next)) {
            o_log("last brush");
            self->brush_size = BRUSH_MAX_SIZE-1;
            brush_set(self);
        
        }
    
    } else {
        WBtn_down_set(self->btn_brush_next, true);
        WBtn_down_timer_reset(self->btn_brush_next);
    }

    // canvas tools
    if (WCombo_changed(self->btn_tool_mode)) {
        int idx = WCombo_idx(self->btn_tool_mode);
        // should match the enum mp_canvas_TOOL_*
        mp_manager_tool_mode(self->manager, idx);
    }

    if (selection_state != mp_selection_state_OFF && selection_state != mp_selection_state_PASTE) {
        

        WBtn_down_set(self->btn_select_create_create, selection_state == mp_selection_state_CREATE);
        if(WBtn_pressed(self->btn_select_create_create)) {
            mp_manager_selection_state_set(self->manager, mp_selection_state_CREATE);
        }
        WBtn_down_set(self->btn_select_create_move, selection_state == mp_selection_state_MOVE);
        if(WBtn_toggled(self->btn_select_create_move)) {
            if (WBtn_down(self->btn_select_create_move)) {
                mp_manager_selection_state_set(self->manager, mp_selection_state_MOVE);
            } else {
                mp_manager_selection_state_set(self->manager, mp_selection_state_APPLY);
            }
        }
        if(WBtn_clicked(self->btn_select_create_copy)) {
            mp_manager_selection_copy(self->manager, false);
        }
        if(WBtn_clicked(self->btn_select_create_cut)) {
            mp_manager_selection_copy(self->manager, true);
        }
        
        WObj_hide_set(self->pane_select_create, false);
    } else {
        WObj_hide_set(self->pane_select_create, true);
    }


    if (selection_state == mp_selection_state_PASTE) {
        if (WBtn_clicked(self->btn_select_paste_rotate_left)) {
            mp_manager_selection_rotate(self->manager, true);
        }
        if (WBtn_clicked(self->btn_select_paste_rotate_right)) {
            mp_manager_selection_rotate(self->manager, false);
        }
        if (WBtn_clicked(self->btn_select_paste_mirror_h)) {
            mp_manager_selection_mirror(self->manager, true);
        }
        if (WBtn_clicked(self->btn_select_paste_mirror_v)) {
            mp_manager_selection_mirror(self->manager, false);
        }

        WBtn_toggled(self->btn_select_paste_blend);
        mp_manager_selection_blend_set(self->manager, WBtn_down(self->btn_select_paste_blend));

        if (WBtn_clicked(self->btn_select_paste_copy)) {
            mp_manager_selection_paste(self->manager, false);
        }
        if (WBtn_clicked(self->btn_select_paste_ok)) {
            mp_manager_selection_paste(self->manager, true);
        }

        WObj_hide_set(self->pane_select_paste, false);
    } else {
        WObj_hide_set(self->pane_select_paste, true);
    }

}

void mp_toolbar_render(struct mp_toolbar *self, oobj tex)
{
    WTheme_render(self->theme, tex);
    
    // brush preview
    {
        vec2 space = WObj_gen_size(self->btn_brush_type_area);
        vec2 brush_size = RTex_size(self->brush_tex_current);
        vec2 size = vec2_min_v(space, brush_size);

        vec4 rect = WObj_gen_rect(self->btn_brush_type_area);
        rect.zw = size;
        vec4 uv = RTex_rect(self->brush_tex_current, 0, 0);

        RTex_blend_rect(tex, self->brush_tex_current, rect, uv);
    }
}
