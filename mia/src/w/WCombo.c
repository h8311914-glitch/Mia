#include "w/WCombo.h"
#include "o/OObj_builder.h"
#include "o/OEvent.h"
#include "o/str.h"
#include "w/WBox.h"
#include "w/WBtn.h"
#include "w/WAlign.h"
#include "w/WIcon.h"
#include "w/WText.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"
#include "o/OArray.h"


static void combo_text_creator(oobj combo, oobj widget_parent, int idx)
{
    char **btn_text_list = WCombo_select_user_data(combo);
    char *btn_text = btn_text_list[idx];
    oobj text = WText_new(widget_parent, btn_text);
    //WObj_padding_set(text, vec4_(1));
}

//
// public
//

WCombo *WCombo_init(oobj obj, oobj parent, oobj toplevel_parent, WCombo__creator_fn creator_fn, int idx,
                    WSelect__creator_fn select_creator_fn, int select_num, void *select_user_data)
{
    WObj *super = obj;
    WCombo *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WCombo_ID);

    self->btn = WBtn_new(self);
    oobj btn_box = WBox_new(self->btn, WBox_H_WEIGHTS);
    WBox_spacing_set(btn_box, vec2_(1));
    self->btn_container = WObj_new(btn_box);
    oobj icon_align = WAlign_new(btn_box);
    WAlign_align_h_set(icon_align, WAlign_END);
    WBox_child_weight_set(icon_align, 1.0);
    self->btn_icon = WIcon_new(icon_align, WTheme_ICON_SMALL_DIR_DOWN);

    self->toplevel_parent = toplevel_parent;
    self->creator_fn = creator_fn;

    self->select_creator_fn = select_creator_fn;
    self->select_num = select_num;
    self->select_user_data = select_user_data;

    self->changed_idx = idx;

    WCombo_set(self, idx);

    // vfuncs
    super->v_update = WCombo__v_update;

    return self;
}

WCombo *WCombo_new_text(oobj parent, oobj toplevel_parent, char **btn_text_list, int opt_list_num, int idx)
{
    // protected
    void WSelect__text_creator(oobj select, oobj btn, int select_idx);
    int num = opt_list_num>0? opt_list_num : o_list_num(btn_text_list);
    // we need a deep copy of the string list, to cache it...
    // for that we create a new hidden OObj to later move into the WCombo
    oobj container = OObj_new(parent);
    char **text_list = o_new(container, char *, num);
    for (int i = 0; i < num; i++) {
        text_list[i] = o_str_clone(container, btn_text_list[i]);
    }
    WCombo *self = WCombo_new(parent, toplevel_parent, combo_text_creator, idx,
        WSelect__text_creator, num, text_list);
    o_move(container, self);
    return self;
}

//
// vfuncs
//

vec2 WCombo__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WCombo);
    WCombo *self = obj;

    vec2 size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);

    if (WBtn_toggled(self->btn)) {
        o_del(self->select_align);
        if (WBtn_down(self->btn)) {
            self->select_align = WAlign_new(self->toplevel_parent);
            WAlign_align_set(self->select_align, WAlign_AT_ABS, WAlign_AT_ABS);
            WObj_style_set(self->select_align, WObj_style(self), false);
            self->select = WSelect_new(self->select_align,
                                       self->select_creator_fn, self->select_num, self->select_user_data);
            if (self->select_on_new) {
                self->select_on_new(self);
            }
        }
    }

    if (self->select_align) {
        // may be from a previous frame...
        vec4 combo_rect = WObj_gen_rect(obj);
        vec2 align_pos = {{combo_rect.left, combo_rect.top + combo_rect.height}};
        WAlign_at_set(self->select_align, align_pos);

        int clicked = WSelect_clicked(self->select);
        if (clicked >= 0) {
            WCombo_set(obj, clicked);
        }
    }

    WIcon_icon_idx_set(self->btn_icon, self->select_align? WTheme_ICON_SMALL_DIR_UP : WTheme_ICON_SMALL_DIR_DOWN);

    return size;
}

//
// object functions
//


void WCombo_set(oobj obj, int idx)
{
    OObj_assert(obj, WCombo);
    WCombo *self = obj;


    // close WSelect in the next frame (to prevent mistakes...)
    OEvent_new_post_del(self, self->select_align);
    self->select_align = NULL;
    self->select = NULL;

    WBtn_down_set(self->btn, false);

    o_del(self->widget_parent);
    self->widget_parent = WObj_new(self->btn_container);

    self->idx = idx;
    self->creator_fn(self, self->widget_parent, idx);
}
