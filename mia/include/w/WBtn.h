#ifndef W_WBTN_H
#define W_WBTN_H

/**
 * @file WBtn.h
 *
 * Object (derives WObj)
 *
 * Widget that renders a button around its widgets.
 *
 * @note as with u/btn, only switches mode using the event functions (clicked, toggled, pressed)
 *       or use the auto_ mode|event
 */

#include "WObj.h"

/** object id */
#define WBtn_ID WObj_ID "Btn."

enum WBtn_auto {
    WBtn_auto_OFF,
    WBtn_auto_CLICKED,
    WBtn_auto_PRESSED,
    WBtn_auto_TOGGLED,
    WBtn_auto_LONG_PRESSED,
    WBtn_auto_CLICKED_AND_LONG_PRESSED,
    WBtn_auto_ENUM_MAX
};

enum WBtn_style {
    WBtn_FLAT,
    WBtn_FLAT_ROUND,
    WBtn_DEF,
    WBtn_DEF_ROUND,
    WBtn_DEF_DUAL,
    WBtn_DEF_DUAL_L,
    WBtn_DEF_DUAL_R,
    WBtn_DEF_ROUND_DUAL_L,
    WBtn_DEF_ROUND_DUAL_R,
    WBtn_BIG,
    WBtn_BIG_ROUND,
    WBtn_BIG_DUAL,
    WBtn_BIG_DUAL_L,
    WBtn_BIG_DUAL_R,
    WBtn_BIG_ROUND_DUAL_L,
    WBtn_BIG_ROUND_DUAL_R,
    WBtn_RADIO,
    WBtn_CHECK,
    WBtn_SWITCH,
    WBtn_ENUM_MAX
};

typedef struct {
    WObj super;
    
    enum WBtn_style style;
    vec4 color;

    enum WBtn_auto auto_mode;
    OObj__event_fn auto_event;
    bool auto_action;
    
    bool slideable;

    // 0 for up, 1 for down, >1 for custom
    int mode;
    
    ou64 down_timer;
    float long_pressed_time;

    struct a_pointer pointer;
    vec2 pointer_prev_pos;
    
} WBtn;


/**
 * Initializes the object
 * @param obj WBtn object
 * @param parent to inherit from
 * @return obj casted as WBtn
 */
WBtn *WBtn_init(oobj obj, oobj parent);


/**
 * Creates a new WBtn object
 * @param parent to inherit from
 * @return The new object
 */
o_inline WBtn *WBtn_new(oobj parent)

{
    OObj_DECL_IMPL_NEW(WBtn, parent);
}

/**
 * Creates a new WBtn object with an internal WText
 * @param parent to inherit from
 * @param text for the WText
 * @param opt_out_text if not NULL, set to the WText
 * @return The new object
 */
WBtn *WBtn_new_text(oobj parent, const char *text, oobj *opt_out_text);

/**
 * Creates a new WBtn object with an internal WTextShadow
 * @param parent to inherit from
 * @param text for the WTextShadow
 * @param opt_out_text_shadow if not NULL, set to the WTextShadow
 * @return The new object
 */
WBtn *WBtn_new_text_shadow(oobj parent, const char *text, oobj *opt_out_text_shadow);

//
// virtual implementations:
//


/**
 * Virtual implementation that sets the button boxes and checks the pointer for clicked.
 * Calls WObj__update_children_stacked.
 * The children are placed in the button.
 */
vec2 WBtn__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);

/**
 * Virtual implementation that sets the color and style
 */
void WBtn__v_style_apply(oobj obj);


//
// object functions:
//

/**
 * @param obj WBtn object
 * @return style for the button
 * @note with DUAL styles, btns can be put directly next to each other
 */
OObj_DECL_GETSET(WBtn, enum WBtn_style, style)


/**
 * @param obj WBtn object
 * @return color for the button
 */
OObj_DECL_GETSET(WBtn, vec4, color)


/**
 * @param obj WBtn object
 * @return automatically call clicked, pressed, toggled (disabled by default)
 */
OObj_DECL_GETSET(WBtn, enum WBtn_auto, auto_mode)

/**
 * @param obj WBtn object
 * @return event that is triggered in an auto_mode
 */
OObj_DECL_SET(WBtn, OObj__event_fn, auto_event)

/**
 * @param obj WBtn object
 * @return false (default): needs pointer pressed >in bounds<
 *         true: gets pressed if pointer was already down (sliding in)
 * @note for clicked and pressed...
 */
OObj_DECL_GETSET(WBtn, bool, slideable)

/**
 * @param obj WBtn object
 * @return current btn mode, 0 for up, 1 for down, >1 for custom
 */
OObj_DECL_GETSET(WBtn, int, mode)

/**
 * @param obj WBtn object
 * @return timer (a_timer() ) which is reset if !WBtn_down().
 *         results in counting the down time
 * @note WBtn_long_pressed may set it += 1000 seconds.
 */
OObj_DECL_GETSET(WBtn, ou64, down_timer)

/**
 * @param obj WBtn object
 * @return time needed for a long press, defaults to 0.5 [s]
 */
OObj_DECL_GETSET(WBtn, float, long_pressed_time)

/**
 * sets down_timer to a_timer()
 * @param obj WBtn object
 * @returns WBtn_down_timer
 */
ou64 WBtn_down_timer_reset(oobj obj);


/**
 * @param obj WBtn object
 * @return true if down (pressed down); else up or custom
 */
o_inline bool WBtn_down(oobj obj) {
    return WBtn_mode(obj) == 1;
}

/**
 * @param obj WBtn object
 * @param down sets mode to 1 (pressed down) if true, else 0
 */
o_inline void WBtn_down_set(oobj obj, bool down) {
    WBtn_mode_set(obj, down? 1 : 0);
}

/**
 * Sets all in group, except obj/self.
 * @param obj WBtn object
 * @param group Array of oobj's all WBtn's in that are set, except of the passed obj
 * @param group_num Array size, <0 (-1) for null terminated list
 */
void WBtn_set_group(oobj obj, oobj *group, osize group_num, bool pressed);

/**
 * Checks if the button got clicked (down and released up again) and will update the sprites
 * @param obj WBtn object
 * @return true if button is pressed down and released up again (click)
 */
bool WBtn_clicked(oobj obj);

/**
 * Checks if the button got pressed down and will update the sprites
 * @param obj WBtn object
 * @return true if button got pressed down
 */
bool WBtn_pressed(oobj obj);

/**
 * Checks if the button got toggled and will update the sprites
 * @param obj WBtn object
 * @return true if button got toggled(switched pressed down; released up mode)
 */
bool WBtn_toggled(oobj obj);

/**
 * Checks if the button is pressed down for a specific amount of time (using a_timer() ).
 * @param obj WBtn object
 * @return true if button got long pressed, 'fires' a single time
 *         Checks for WBtn_long_pressed_time()
 * @note sets down_time to += 1000 seconds if it reached the long press time.
 *       to avoid returning true the next call.
 * @sa WBtn_down_timer_reset to avoid manually down sets to be counted
 */
bool WBtn_long_pressed(oobj obj);

#endif //W_WBTN_H
