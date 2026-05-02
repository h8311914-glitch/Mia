#include "x/XViewDialog.h"
#include "o/OObj_builder.h"
#include "r/RTex.h"
#include "r/RCam.h"
#include "a/common.h"
#include "w/WTheme.h"
#include "w/WWindowDialog.h"
#include "w/WText.h"
#include "w/WIcon.h"


#undef O_LOG_LIB
#define O_LOG_LIB "x"

#include "o/log.h"

#define DIALOG_INIT_Y_WEIGHT 0.33f

//
// public
//

XViewDialog *XViewDialog_init(oobj obj, oobj parent, enum WTheme_indices icon, const char *title, oobj wobj,
                              OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok)
{
    AView *super = obj;
    XViewDialog *self = obj;
    o_clear(self, sizeof *self, 1);

    AView_init(obj, parent, XViewDialog__v_setup, XViewDialog__v_update, XViewDialog__v_render);
    OObj_id_set(self, XViewDialog_ID);


    self->theme = WTheme_new_tiny(self);
    self->dialog = WWindowDialog_new(self, title, opt_on_cancel != NULL, true);
    WIcon_icon_idx_set(WWindowDialog_icon(self->dialog), icon);
    o_move(wobj, WWindowDialog_body(self->dialog));

    self->on_cancel = opt_on_cancel;
    self->on_ok = opt_on_ok;

    return self;
}

XViewDialog *XViewDialog_new_text(oobj parent, enum WTheme_indices icon, const char *title, const char *text,
                                  OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok)
{
    oobj wobj = WText_new(parent, text);
    WObj_padding_set(wobj, vec4_(4, 8, 4, 0));
    return XViewDialog_new(parent, icon, title, wobj, opt_on_cancel, opt_on_ok);
}

XViewDialog *XViewDialog_new_question(oobj parent, const char *title, const char *text,
                                      OObj__event_fn on_cancel, OObj__event_fn on_ok)
{
    assert(on_cancel && on_ok);
    return XViewDialog_new_text(parent, WTheme_ICON_QUESTION, title, text, on_cancel, on_ok);
}

XViewDialog *XViewDialog_new_info(oobj parent, const char *text)
{
    return XViewDialog_new_text(parent, WTheme_ICON_EXCLAMATION, "INFO", text, NULL, NULL);
}


XViewDialog *XViewDialog_new_error(oobj parent, const char *text)
{
    XViewDialog *self = XViewDialog_new_text(parent, WTheme_ICON_CROSS, "ERROR", text, NULL, NULL);
    WIcon_color_set(WWindowDialog_icon(self->dialog), vec4_(0.8, 0.1, 0.1, 1.0));
    return self;
}


//
// virtual implementations
//

void XViewDialog__v_setup(oobj view)
{
    OObj_assert(view, XViewDialog);
    XViewDialog *self = view;

    // pre update to guess size for setting init lt
    vec2 size = WTheme_update(self->theme, self->dialog, vec2_(0), vec2_(0));
    vec2 cam_size = RCam_size(a_cam());

    vec2 room = vec2_max(vec2_sub_v(cam_size, size), 0);
    vec2 lt = vec2_(room.x*0.5f, room.y*DIALOG_INIT_Y_WEIGHT);

    WWindowDialog_bg_maximize_set(self->dialog, true);
    WWindow_lt_set(self->dialog, lt);
}

void XViewDialog__v_update(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewDialog);
    XViewDialog *self = view;

    WTheme_update_full_scene(self->theme, self->dialog, false);

    enum WWindowDialog_state state = WWindowDialog_state(self->dialog);
    if (state != WWindowDialog_RUNNING) {
        if (state == WWindowDialog_CANCEL && self->on_cancel) {
            self->on_cancel(self);
            self->on_cancel = NULL;
        }
        if (state == WWindowDialog_OK && self->on_ok) {
            self->on_ok(self);
            self->on_ok = NULL;
        }
        a_scene_exit();
    }

    a_pointer_handled(-1, -1);
}

void XViewDialog__v_render(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewDialog);
    XViewDialog *self = view;
    WTheme_render(self->theme, tex);
}

//
// object functions:
//
