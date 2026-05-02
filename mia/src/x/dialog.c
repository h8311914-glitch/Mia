#include "x/dialog.h"
#include "a/common.h"
#include "a/AScene.h"

#define VIEWDIALOG_INJECT "x_dialog_context"

struct viewdialog_context {
    oobj scene;
    OObj__event_fn ok, cancel;
};


static void viewdialog_on_cancel(oobj view)
{
    OObj_assert(view, XViewDialog);
    XViewDialog *self = view;
    struct oobj_opt inject = OObj_find(view, OObj, VIEWDIALOG_INJECT, 0);
    if(!inject.o || !o_user(inject.o)) {
        return;
    }
    struct viewdialog_context *C = o_user(inject.o);
    if (C->cancel) {
        C->cancel(C->scene);
    }
}
static void viewdialog_on_ok(oobj view)
{
    OObj_assert(view, XViewDialog);
    XViewDialog *self = view;
    struct oobj_opt inject = OObj_find(view, OObj, VIEWDIALOG_INJECT, 0);
    if(!inject.o || !o_user(inject.o)) {
        return;
    }
    struct viewdialog_context *C = o_user(inject.o);
    if (C->ok) {
        C->ok(C->scene);
    }
}



static oobj dialog_create_scene(oobj parent, oobj view, OObj__event_fn on_cancel, OObj__event_fn on_ok)
{
    AScene *scene = AScene_new(parent, view, true, AScene_UNSAFE);
    AScene_opaque_set(scene, false);
    AScene_escape_event_set_exit(scene);

    oobj inject = OObj_new(view);
    OObj_name_set(inject, VIEWDIALOG_INJECT);
    struct viewdialog_context *C = o_user_new0(inject, *C, 1);
    C->scene = scene;
    C->ok = on_ok;
    C->cancel = on_cancel;
    return scene;
}

//
// public
//

oobj x_dialog(oobj opt_parent, enum WTheme_indices icon, const char *title, oobj wobj,
              OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok)
{
    oobj parent = o_or(opt_parent, a_root());
    OObj__event_fn passed_cancel = opt_on_cancel? viewdialog_on_cancel : NULL;
    OObj__event_fn passed_ok = opt_on_ok? viewdialog_on_ok : NULL;
    XViewDialog *view = XViewDialog_new(parent, icon, title, wobj, passed_cancel, passed_ok);
    return dialog_create_scene(parent, view, opt_on_cancel, opt_on_ok);
}

oobj x_dialog_text(oobj opt_parent, enum WTheme_indices icon, const char *title, const char *text,
                   OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok)
{
    oobj parent = o_or(opt_parent, a_root());
    OObj__event_fn passed_cancel = opt_on_cancel? viewdialog_on_cancel : NULL;
    OObj__event_fn passed_ok = opt_on_ok? viewdialog_on_ok : NULL;
    XViewDialog *view = XViewDialog_new_text(parent, icon, title, text, passed_cancel, passed_ok);
    return dialog_create_scene(parent, view, opt_on_cancel, opt_on_ok);
}

oobj x_dialog_question(oobj opt_parent, const char *title, const char *text,
    OObj__event_fn on_cancel, OObj__event_fn on_ok)
{
    oobj parent = o_or(opt_parent, a_root());
    OObj__event_fn passed_cancel = on_cancel? viewdialog_on_cancel : NULL;
    OObj__event_fn passed_ok = on_ok? viewdialog_on_ok : NULL;
    XViewDialog *view = XViewDialog_new_question(parent, title, text, passed_cancel, passed_ok);
    return dialog_create_scene(parent, view, on_cancel, on_ok);
}

oobj x_dialog_info(oobj opt_parent, const char *text)
{
    oobj parent = o_or(opt_parent, a_root());
    XViewDialog *view = XViewDialog_new_info(parent, text);
    return dialog_create_scene(parent, view, NULL, NULL);
}

oobj x_dialog_error(oobj opt_parent, const char *text)
{
    oobj parent = o_or(opt_parent, a_root());
    XViewDialog *view = XViewDialog_new_error(parent, text);
    return dialog_create_scene(parent, view, NULL, NULL);
}