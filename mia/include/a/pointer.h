#ifndef A_POINTER_H
#define A_POINTER_H

/**
 * @file pointer.h
 *
 * struct for managing pointer input (mouse or multi q)
 */

#include "o/common.h"
#include "m/types/flt.h"


// maximal idx pointers
#define a_pointer_MAX 16

// maximal buttons (left, middle, right -> 0, 1, 2)
#define a_pointer_MAX_BUTTON_IDS 3

// maximal in between frames pointer
#define a_pointer_MAX_BACKTRACE 8


// for the bitmap field struct a_pointer_pen::state
#define a_pointer_pen_DOWN       (1u << 0)  /**< pen is pressed down */
#define a_pointer_pen_BUTTON_1   (1u << 1)  /**< button 1 is pressed */
#define a_pointer_pen_BUTTON_2   (1u << 2)  /**< button 2 is pressed */
#define a_pointer_pen_BUTTON_3   (1u << 3)  /**< button 3 is pressed */
#define a_pointer_pen_BUTTON_4   (1u << 4)  /**< button 4 is pressed */
#define a_pointer_pen_BUTTON_5   (1u << 5)  /**< button 5 is pressed */
#define a_pointer_pen_ERASER_TIP (1u << 30) /**< eraser tip is used */


/**
 * Possible pointer sources
 * @sa a_pointer_source()
 */
enum a_pointer_source {
    a_pointer_source_MOUSE,
    a_pointer_source_TOUCH,
    a_pointer_source_PEN,
    a_pointer_source_OVERRIDE,
    a_pointer_source_ENUM_MAX,
};

/**
 * pointer for mouse, touch and pen info with multitouch support.
 * If hovering is not available, pos will be set to NAN in xy.
 * .idx       Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * .pos in back buffer viewport [-1:1] (x, y, 0, 1)
 * .down if the pointer is currently "down" on the screen
 * .prev_down if the pointer was previously down.
 * .pressure [0:1] if available (else 1.0) (stylus)
 */
struct a_pointer {
    int idx;
    vec4 pos;
    bool down;
    bool prev_down;
    float pressure;
};

/**
 * For pressure sensitive pens.
 * Default is all 0.0.
 * As an example, the Samsung Galaxy S22 Ultra S-Pen only has tilt axles, all remaining stay default.
 */
struct a_pointer_pen {
    vec2 tilt;                  // Pen horizontal and vertical tilt angle.  Bidirectional: -90.0 to 90.0 (left-to-right).
    float distance;             // Pen distance to drawing surface.  Unidirectional: 0.0 to 1.0
    float rotation;             //Pen barrel rotation.  Bidirectional: -180 to 179.9 (clockwise, 0 is facing up, -180.0 is facing down). */
    float slider;               // Pen finger wheel or slider (e.g., Airbrush Pen).  Unidirectional: 0 to 1.0
    float tangential_pressure;  // Pressure from squeezing the pen ("barrel pressure")
    ou32 state;                 // Bitmap field, see a_pointer_pen_* (a_pointer_pen_DOWN); use with care may be to sensitive
};


/**
 * Default pointer function definition.
 * If hovering is not available, pos will be set to NAN in xy.
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @return the pointer for the given index (0=default)
 */
typedef struct a_pointer (*a_pointer__fn)(int idx, int history);


/**
 * Returns a pointer in back buffer gl / display coordinations [-1.0 : +1.0] (x, y, 0, 1)  (+y is up).
 * If hovering is not available, pos will be set to NAN in xy.
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @return the pointer for the given index (0=default)
 * @note asserts bounds
 * @example Using history, a typical for loop would be as the following:
 * ```c
 * for(int bt=a_pointer_history_num(0); bt>0; bt--) {
 *     struct a_pointer p = a_pointer_back(0, bt);
 *     // ...
 * }
 * ```
 */
struct a_pointer a_pointer_back(int idx, int history);



//
// AView's delegations
//


/**
 * Returns a pointer in current AView's tex gl coordinations [-1.0 : +1.0] (x, y, 0, 1).
 * If hovering is not available, pos will be set to NAN in xy.
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @return the pointer for the given index (0=default)
 * @note asserts bounds
 *       Calls AView_pointer with a_app_view
 * @example Using history, a typical for loop would be as the following:
 * ```c
 * for(int bt=a_pointer_history_num(0); bt>0; bt--) {
 *     struct a_pointer p = a_pointer_back(0, bt);
 *     // ...
 * }
 * ```
 */
struct a_pointer a_pointer_raw(int idx, int history);


/**
 * Returns a pointer in current AView's cam viewport coordinations
 * with only the perspective cam (p_only) (x, y, 0, 1).
 * If hovering is not available, pos will be set to NAN in xy.
 * @param obj AView object
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @return pointer pos (x, y, 0, 1) in views cam, down=false if not within scenes quad
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @note asserts bounds
 *       Calls AView_pointer with a_app_view
 */
struct a_pointer a_pointer_p(int idx, int history);


/**
 * Returns a pointer in current AView's cam viewport coordinations (x, y, 0, 1).
 * If hovering is not available, pos will be set to NAN in xy.
 * @param obj AView object
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @return pointer pos (x, y, 0, 1) in views cam, down=false if not within scenes quad
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @note asserts bounds
 *       Calls AView_pointer with a_app_view
 */
struct a_pointer a_pointer(int idx, int history);

/**
 * @param idx, history as with a_pointer.
 * @return additional pen fields for pressure sensitive pointer pen's
 */
struct a_pointer_pen a_pointer_pen(int idx, int history);


//
// utility functions
//

/**
 * @return the current pointer source
 */
enum a_pointer_source a_pointer_source(void);

/**
 * Returns the number of valid history pointers.
 * @param idx see a_pointer
 * @return number of new history pointers, may be 0 if nothing changed
 */
int a_pointer_history_num(int idx);


/**
 * Sets a pointer as handled, so scenes down the stack will get an "up" pointer.
 * @param idx see a_pointer, -1 for all
 * @param history see a_pointer, -1 for all
 * @param full if true, ignores current a_view() bounds.
 *             if false, only handles if the pointer was in current bounds.
 * @note asserts idx and history bounds
 */
void a_pointer_handled_ex(int idx, int history, bool full);

/**
 * Sets a pointer as handled, so scenes down the stack will get an "up" pointer.
 * Only handles if the pointer is in the current a_view() bounds
 * @param idx see a_pointer, -1 for all
 * @param history see a_pointer, -1 for all
 * @note asserts idx and history bounds.
 */
o_inline void a_pointer_handled(int idx, int history)
{
    a_pointer_handled_ex(idx, history, false);
}

/**
 * @param idx see a_pointer
 * @param history see a_pointer
 * @return true if a_pointer_handled was called that frame
 * @note asserts bounds
 */
bool a_pointer_was_handled(int idx, int history);


/**
 * Calls a_pointer_handled(idx, -1) (full history),
 * if that pointer (idx, 0) was handled (!down).
 * Usecase: An WObj like WBtn handles only pointer 0, 0.
 *          Something "below" uses the history, for drawing, etc.
 *          We forward it so the full history gets handled.
 *          Thus avoiding (as wished) to draw below that widget.
 * @param idx see a_pointer, -1 for all
 * @note asserts bounds
 */
void a_pointer_handled_forward_history(int idx);


//
// touch and pen states
//

/**
 * Will reset all touchscreen data deferred to the next frame
 */
void a_pointer_reset_touch(void);

/**
 * @return true for touch screens
 * @note may be changed during runtime to true, after first usage of the touch screen
 */
bool a_pointer_touch_used(void);

/**
 * @return the a_timer() value of the frame that had the last event.
 *         Compare with o_timer_diff_s()
 */
ou64 a_pointer_timer(void);

/**
 * @return true if the pen was used once
 */
bool a_pointer_pen_available(void);

/**
 * @return true if the pen is currently used
 */
bool a_pointer_pen_used(void);

/**
 * @return pressure deadzone, if above pen is down, may be hardware dependent so may need user configuration
 *         defaults to 0.001
 */
float a_pointer_pen_deadzone(void);

/**
 * @return pressure deadzone, if above pen is active, may be hardware dependent so may need user configuration
 *         defaults to 0.001
 */
float a_pointer_pen_deadzone_set(float set);

//
// change pointer data for upcomming requests
//

/**
 * @param pointer to be replaced
 * @param idx, history: see other pointer functions
 * @note pointer in back buffer gl / display coordinations [-1.0 : +1.0] (x, y, 0, 1)  (+y is up)
 */ 
void a_pointer_override_back(struct a_pointer pointer, int idx, int history);

/**
 * @param pen additional pen fields to be replaced
 * @param idx, history: see other pointer functions
 */ 
void a_pointer_override_pen(struct a_pointer_pen pen, int idx, int history);

//
// little helper to check if a pointer is pressed (got down) or released (got up)
//

/**
 * @param self current frames pointer
 * @return true if the pointer changed to active -> DOWN
 */
o_inline bool a_pointer_pressed(struct a_pointer self)
{
    return self.down && !self.prev_down;
}

/**
 * @param self current frames pointer
 * @return true if the pointer changed to NOT active -> UP
 */
o_inline bool a_pointer_released(struct a_pointer self)
{
    return !self.down && self.prev_down;
}


#endif //A_POINTER_H
