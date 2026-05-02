#ifndef X_XVIEWDIALOG_H
#define X_XVIEWDIALOG_H

/**
 * @file XViewDialog.h
 *
 * object.
 *
 * AView which shows an WWindowDialog
 *
 * Subclass of the AView object
 */

#include "m/vec/vec2.h"
#include "a/AView.h"
#include "w/WTheme.h"

/** object id */
#define XViewDialog_ID AView_ID "XViewDialog."


typedef struct {
    AView super;

    oobj theme;
    oobj dialog;

    OObj__event_fn on_cancel;
    OObj__event_fn on_ok;
} XViewDialog;


/**
 * Initializes the object.
 * Creates an AView that renders a WWindowDialog.
 * @param obj XViewDialog object
 * @param parent to inherit from
 * @param icon for the Dialog to show, like WTheme_ICON_EXCLAMATION
 * @param wobj WObj tree to be displayed, o_move'd into WWindowDialog_body()
 * @param title for the WWindow
 * @param opt_on_cancel optional event called if cancel is clicked, needed to show the cancel WBtn
 * @param opt_on_ok optional event called if ok is clickked, the ok WBtn is always visible, even if NULL is passed
 * @return obj casted as XViewDialog
 */
XViewDialog *XViewDialog_init(oobj obj, oobj parent, enum WTheme_indices icon, const char *title, oobj wobj,
                              OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok);

/**
 * Creates an AView that renders a WWindowDialog.
 * @param parent to inherit from
 * @param icon for the Dialog to show, like WTheme_ICON_EXCLAMATION
 * @param wobj WObj tree to be displayed, o_move'd into WWindowDialog_body()
 * @param title for the WWindow
 * @param opt_on_cancel optional event called if cancel is clicked, needed to show the cancel WBtn
 * @param opt_on_ok optional event called if ok is clickked, the ok WBtn is always visible, even if NULL is passed
 * @return The new object
 */
o_inline XViewDialog *XViewDialog_new(oobj parent, enum WTheme_indices icon, const char *title, oobj wobj,
                                      OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok)
{
    OObj_DECL_IMPL_NEW(XViewDialog, parent, icon, title, wobj, opt_on_cancel, opt_on_ok);
}

/**
 * Creates an AView that renders a WWindowDialog, internally uses a WText and only shows the ok WBtn
 * @param parent to inherit from
 * @param icon for the Dialog to show, like WTheme_ICON_EXCLAMATION
 * @param title for the WWindow
 * @param text used for the WText to display
 * @param opt_on_cancel optional event called if cancel is clicked, needed to show the cancel WBtn
 * @param opt_on_ok optional event called if ok is clickked, the ok WBtn is always visible, even if NULL is passed
 * @return The new object
 */
XViewDialog *XViewDialog_new_text(oobj parent, enum WTheme_indices icon, const char *title, const char *text,
                                  OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok);

/**
 * Creates an AView that renders a WWindowDialog with a WText as question prompt
 * @param parent to inherit from
 * @param title for the WWindow
 * @param text to be displayed
 * @param on_cancel, on_ok events called if cancel or ok is clicked
 * @return The new object
 */
XViewDialog *XViewDialog_new_question(oobj parent, const char *title, const char *text,
                                      OObj__event_fn on_cancel, OObj__event_fn on_ok);


/**
 * Creates an AView that renders a WWindowDialog with a WText as info prompt
 * @param parent to inherit from
 * @param text to be displayed
 * @return The new object
 */
XViewDialog *XViewDialog_new_info(oobj parent, const char *text);

/**
 * Creates an AView that renders a WWindowDialog with a WText as error prompt
 * @param parent to inherit from
 * @param text to be displayed
 * @return The new object
 */
XViewDialog *XViewDialog_new_error(oobj parent, const char *text);

//
// virtual implementations
//

void XViewDialog__v_setup(oobj view);

void XViewDialog__v_update(oobj view, oobj tex, float dt);

void XViewDialog__v_render(oobj view, oobj tex, float dt);


//
// object functions:
//

/**
 * @param obj XViewDialog object
 * @return cancel event to set, to enable the cancel btn, set it in the constructor!
 */
OObj_DECL_SET(XViewDialog, OObj__event_fn, on_cancel)

/**
 * @param obj XViewDialog object
 * @return ok event to set
 */
OObj_DECL_SET(XViewDialog, OObj__event_fn, on_ok)

/**
 * @param obj XViewDialog object
 * @return WWindowDialog used
 */
OObj_DECL_SET(XViewDialog, oobj, dialog)


#endif //X_XVIEWDIALOG_H
