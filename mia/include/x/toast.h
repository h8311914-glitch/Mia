#ifndef X_TOAST_H
#define X_TOAST_H

/**
 * @file toast.h
 *
 * Shows a small none interactive mini dialog on the screen.
 * Internally its just an RTex blended to a new layer on the apps main view.
 * Fades away after the given time.
 * Like with x_toast_text an RTex rendered from a WPane with a WText
 */

#include "o/common.h"
#include "m/types/flt.h"

/**
 * Install the toast syttem on a new a_app_view's layer.
 * So it works on top of the default scene stage.
 * @note asserts main thread
 */
void x_toast_install(void);

/**
 * @return true if already installed
 */
bool x_toast_installed(void);


/**
 * @return current left time of the toast.
 */
float x_toast_current_time(void);

/**
 * @return current left time of the toast.
 *         only sets if x_toast_current_tex != NULL
 * @note asserts main thread.
 *
 */
float x_toast_current_time_set(float set);

/**
 * @return the current displayed toast RTex (or NULL if nothing is shown)
 */
struct oobj_opt x_toast_current_tex(void);

/**
 * @param opt_parent optional parent on which an OPtr is installed, if deleted, immediately stops the toast.
 * @param tex_sink RTex to be displayed, o_moved into the toast system, auto deleted on times up
 * @param time to show the toast in seconds
 * @note asserts main thread
 */
void x_toast_tex(oobj opt_parent, oobj tex_sink, float time);

/**
 * @param opt_parent optional parent on which an OPtr is installed, if deleted, immediately stops the toast.
 * @param wobj WObj tree to be rendered into a new RTex that is passed to x_toast_tex.
 * @param time to show the toast in seconds.
 * @note uses WTheme_tiny internally.
 *       asserts main thread.
 */
void x_toast_wobj(oobj opt_parent, oobj wobj, float time);

/**
 * Creates a WPane and a WText and calls x_toast_wobj to display it
 * @param opt_parent optional parent on which an OPtr is installed, if deleted, immediately stops the toast.
 * @param text to be displayed
 * @param time to show the toast in seconds
 * @param opt_pane_color sets the WPane color if not NULL
 * @param opt_text_color sets the WText color if not NULL
 * @note uses WTheme_tiny internally.
 *       asserts main thread.
 */
void x_toast_text_ex(oobj opt_parent, const char *text, float time,
        const vec4 *opt_pane_color, const vec4 *opt_text_color);

/**
 * Creates a WPane and a WText and calls x_toast_wobj to display it
 * @param opt_parent optional parent on which an OPtr is installed, if deleted, immediately stops the toast.
 * @param text to be displayed
 * @param time to show the toast in seconds
 * @note uses WTheme_tiny internally.
 *       asserts main thread.
 */
o_inline void x_toast_text(oobj opt_parent, const char *text, float time)
{
    x_toast_text_ex(opt_parent, text, time, NULL, NULL);
}



#endif //X_TOAST_H
