#ifndef W_WALIGN_H
#define W_WALIGN_H

/**
 * @file WAlign.h
 *
 * Object (derives WObj)
 *
 * Widget that tries to align its children (stacked).
 * Uses min_size for the align field.
 * Returns o_max(min_size, children_size) but may render outside it for a frame to calc the offset
 *
 * Each child of WAlign may have its own WAlign_mode.
 * Default, see WAlign_align_h|v.
 * For a specific child, set the child WObj_option to one of the WAlign_VALUES
 * with the key of WAlign_KEY_H|V
 * for example:
 * ```c
 *     WObj_option_set(child, WAlign_KEY_H, WAlign_VALUE_CENTER); 
       WObj_option_set(child, WAlign_KEY_V, WAlign_VALUE_CENTER);   
 * ```
 *
 */

#include "WObj.h"

/** object id */
#define WAlign_ID WObj_ID "Align."

// START -> left and top
// END -> right and bottom
enum WAlign_mode {
    WAlign_START,
    WAlign_CENTER,
    WAlign_END,
    WAlign_FIT,
    WAlign_AT_REL,
    WAlign_AT_ABS,
    WAlign_NUM_MODES
};

#define WAlign_KEY_H "WAlign_h"
#define WAlign_KEY_V "WAlign_v"

static const char *WAlign_VALUES[] = {
    "start",
    "center",
    "end",
    "fit",
    "at_rel",
    "at_abs"
};

#define WAlign_VALUE_START   (WAlign_VALUES[0])
#define WAlign_VALUE_CENTER  (WAlign_VALUES[1])
#define WAlign_VALUE_END     (WAlign_VALUES[2])
#define WAlign_VALUE_FIT     (WAlign_VALUES[3])
#define WAlign_VALUE_AT_REL  (WAlign_VALUES[4])
#define WAlign_VALUE_AT_ABS  (WAlign_VALUES[5])


typedef struct {
    WObj super;
    
    // defaults
    enum WAlign_mode align_h, align_v;

    bool on_unit;
    vec2 at;
} WAlign;


/**
 * Initializes the object
 * @param obj WAlign object
 * @param parent to inherit from
 * @return obj casted as WAlign
 */
WAlign *WAlign_init(oobj obj, oobj parent);


/**
 * Creates a new WAlign object.
 * Alignment default is set to start.
 * @param parent to inherit from
 * @return The new object
 */
o_inline WAlign *WAlign_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(WAlign, parent);
}


/**
 * Creates a new WAlign object.
 * Centered in both horizontal and vertical.
 * @param parent to inherit from
 * @return The new object
 */
WAlign *WAlign_new_center(oobj parent);


/**
 * Creates a new WAlign object.
 * Centered horizontal.
 * @param parent to inherit from
 * @return The new object
 */
WAlign *WAlign_new_center_h(oobj parent);

/**
 * Creates a new WAlign object.
 * Centered vertical.
 * @param parent to inherit from
 * @return The new object
 */
WAlign *WAlign_new_center_v(oobj parent);

//
// virtual implementations:
//


/**
 * Virtual implementation that updates the children aligned in its min_size.
 * Calls WObj_update for each child.
 */
vec2 WAlign__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);


//
// object functions:
//

/**
 * @param obj WAlign object
 * @return horizontal default align mode (left : right) (defaults to start)
 */
OObj_DECL_GETSET(WAlign, enum WAlign_mode, align_h)

/**
 * @param obj WAlign object
 * @return vertical default align mode (top : bottom) (defaults to start)
 */
OObj_DECL_GETSET(WAlign, enum WAlign_mode, align_v)

/**
 * @param obj WAlign object
 * @return if true (default) lt and min_size are floored to a unit
 */
OObj_DECL_GETSET(WAlign, bool, on_unit)

/**
 * @param obj WAlign object
 * @return WAlign_AT_REL: relative coords for lt.
 *                        like padding lt.
 *         WAlign_AT_ABS: absolute coords for lt.
 *                        summed up from parents.
 *                        so whats really on the screen / RTex / AView
 *         Both try to achieve it, bjt clamped to edges
 *         defaults to 0, 0
 */
OObj_DECL_GETSETREF(WAlign, vec2, at)


/**
 * @param obj WAlign object
 * @param align_h horizontal default align mode (left : right) (defaults to start)
 * @param align_v vertical default align mode (top : bottom) (defaults to start)
 */
o_inline void WAlign_align_set(oobj obj, enum WAlign_mode align_h, enum WAlign_mode align_v)
{
    WAlign_align_h_set(obj, align_h);
    WAlign_align_v_set(obj, align_v);
}


#endif //W_WALIGN_H
