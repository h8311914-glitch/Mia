#ifndef X_CURSOR_H
#define X_CURSOR_H

/**
 * @file cursor.h
 *
 * Virtual cursors from either multitouch or gamepad.
 * Multitouch: Swipe from the left or right border into the center to create a virtual cursor.
 *             The cursor sits on top of your thumb and stays there while its (first thumb) down.
 *             Use another finger / thumb to press the cursor down.
 * Gamepad: Press the start button to toggle the virtual cursor mode.
 *          The left analog stick moves the cursor while pressing A presses it.
 * The virtual cursor is installed as a layer in the app view (so if not changed, it sits on top of all scenes.
 * Also, the virtual cursor hacks into a/pointer. So all stuff using it should work fine.
 */

#include "o/common.h"
#include "m/types/flt.h"
#include "a/AView.h"

struct x_cursor_options {
    bool enable_multitouch;
    bool enable_gamepad;

    // to activate multitouch mode start in the padding border left or right from the screen borders.
    // and move at least distance units
    float multitouch_start_border;
    float multitouch_start_distance;
    float multitouch_offset;
    float multitouch_analog_radius;
    float multitouch_gesture_dist;
    float multitouch_gesture_time;

    // true (default) to render the default cursor
    bool render_cursor;
    // if not NULL, called in render (even if render_cursor is false, as alternative)
    // called before the default cursor (if render_cursor is true)
    AView__fn opt_render_fn;

    vec4 color_hover, color_down;

};

/**
 * @return Default options
 */
struct x_cursor_options x_cursor_options_default(void);

/**
 * Installs the virtual cursor on the app main view as a new layer.
 * @param opt_options NULL or passed options to begin with
 */
void x_cursor_install(const struct x_cursor_options *opt_options);

/**
 * @return true if already installed
 */
bool x_cursor_installed(void);

/**
 * @return Reference to the used options
 */
struct x_cursor_options *x_cursor_options_ref(void);

/**
 * @return true if currently active (cursor overriding pointer)
 */
bool x_cursor_active(void);

/**
 * @return true if gamepad mode is active. Use this to render a custom info layer
 */
bool x_cursor_gamepad_active(void);


/**
 * Aborts the startup state.
 * As an example some sort of ui needs to be
 *     dragged from the border,
 *     which else may start the (multitouch) cursor.
 * @note currently only aborts the multitouch cursor.
 */
void x_cursor_start_abort(void);

/**
 * @return Current position in main views camera
 */
vec2 x_cursor_pos(void);

/**
 * @return true if cursor is pressed down
 */
bool x_cursor_down(void);


/**
 * @return either second finger rel position from start press or gamepad right stick.
 *         clamped to [-1:+1]
 */
vec2 x_cursor_analog(void);

//
// Gestures
// Either fast second finger swipe or gamepad dpad pressed
// And second finger double tap or gamepad stick r pressed
//

/**
 * @return true in the frame in which the second finger got swiped up or gamepad dpad up was pressed
 */
bool x_cursor_gesture_up(void);

/**
 * @return true in the frame in which the second finger got swiped left or gamepad dpad left was pressed
 */
bool x_cursor_gesture_left(void);

/**
 * @return true in the frame in which the second finger got swiped down or gamepad dpad down was pressed
 */
bool x_cursor_gesture_down(void);

/**
 * @return true in the frame in which the second finger got swiped right or gamepad dpad right was pressed
 */
bool x_cursor_gesture_right(void);


//
// bindings
//

/**
 * @param opt_action_x, opt_action_y key string from "a/action.h" to bind with a scale of 1.0.
 *                                   may be NULL to ignore (just use x or y not both)
 * @note asserts main thread
 */
void x_cursor_analog_bind_action(const char *opt_action_x, const char *opt_action_y);

/**
 * @param action key string from "a/action.h" to bind to with scale +1.0f for the up gesture
 * @note so call a_action_pressed or down to handle the cursor gesture together with other action sources.
 *       asserts main thread.
 */
void x_cursor_gesture_up_bind_action(const char *action);

/**
 * @param action key string from "a/action.h" to bind to with scale -1.0f for the left gesture
 * @note so call a_action_pressed or down to handle the cursor gesture together with other action sources.
 *       asserts main thread.
 */
void x_cursor_gesture_left_bind_action(const char *action);

/**
 * @param action key string from "a/action.h" to bind to with scale -1.0f for the down gesture
 * @note so call a_action_pressed or down to handle the cursor gesture together with other action sources.
 *       asserts main thread.
 */
void x_cursor_gesture_down_bind_action(const char *action);

/**
 * @param action key string from "a/action.h" to bind to with scale +1.0f for the right gesture
 * @note so call a_action_pressed or down to handle the cursor gesture together with other action sources.
 *       asserts main thread.
 */
void x_cursor_gesture_right_bind_action(const char *action);

/**
 * Removes all gesture binds installed before
 * @note asserts main thread.
 */
void x_cursor_gesture_bind_remove_all(void);

#endif //X_CURSOR_H
