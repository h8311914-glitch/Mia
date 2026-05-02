#ifndef X_DIALOG_H
#define X_DIALOG_H

/**
 * @file dialog.h
 *
 * Create a scene with a XViewDialog as AView
 */

#include "XViewDialog.h"


/**
 * Creates a full opaque scene, which shows XViewDialog
 * @param opt_parent to inherit from, (if not NULL)
 * @param icon for the Dialog to show, like WTheme_ICON_EXCLAMATION
 * @param wobj WObj tree to be displayed, o_move'd into WWindowDialog_body()
 * @param title for the WWindow
 * @param opt_on_cancel optional event called if cancel is clicked, needed to show the cancel WBtn
 * @param opt_on_ok optional event called if ok is clickked, the ok WBtn is always visible, even if NULL is passed
 * @return AScene object
 * @note passed event obj is the new AScene, not the internal view
 */
oobj x_dialog(oobj opt_parent, enum WTheme_indices icon, const char *title, oobj wobj,
              OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok);


/**
 * Creates a full opaque scene, which shows XViewDialog.
 * Internally uses a WText and only shows the ok WBtn
 * @param opt_parent to inherit from, (if not NULL)
 * @param icon for the Dialog to show, like WTheme_ICON_EXCLAMATION
 * @param title for the WWindow
 * @param text used for the WText to display
 * @param opt_on_cancel optional event called if cancel is clicked, needed to show the cancel WBtn
 * @param opt_on_ok optional event called if ok is clickked, the ok WBtn is always visible, even if NULL is passed
 * @return AScene object
 * @note passed event obj is the new AScene, not the internal view
 */
oobj x_dialog_text(oobj opt_parent, enum WTheme_indices icon, const char *title, const char *text,
                   OObj__event_fn opt_on_cancel, OObj__event_fn opt_on_ok);

/**
 * Creates a full opaque scene, which shows XViewDialog with a WText as question prompt.
 * @param opt_parent to inherit from, (if not NULL)
 * @param title for the WWindow
 * @param text to be displayed
 * @param on_cancel, on_ok events called if cancel or ok is clicked
 * @return The new object
 * @note passed event obj is the new AScene, not the internal view
 */
oobj x_dialog_question(oobj opt_parent, const char *title, const char *text,
                       OObj__event_fn on_cancel, OObj__event_fn on_ok);


/**
 * Creates a full opaque scene, which shows XViewDialog with a WText as info prompt.
 * @param opt_parent to inherit from, (if not NULL)
 * @param text to be displayed
 * @return AScene object
 */
oobj x_dialog_info(oobj opt_parent, const char *text);

/**
 * Creates a full opaque scene, which shows XViewDialog with a WText as error prompt.
 * @param opt_parent to inherit from, (if not NULL)
 * @param text to be displayed
 * @return AScene object
 */
oobj x_dialog_error(oobj opt_parent, const char *text);


#endif //X_DIALOG_H
