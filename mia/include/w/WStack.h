#ifndef W_WSTACK_H
#define W_WSTACK_H

/**
 * @file WStack.h
 *
 * Object (derives WObj)
 *
 * Widget that renders its children in a stack, but each child can set a custom order.
 * The higher that order, the later the child will get rendered / resorted to back.
 * Useful for multiple WWindows, also see window_auto_mode for that.
 */

#include "WObj.h"

/** object id */
#define WStack_ID WObj_ID "Stack."


#define WStack_order_KEY "WStack_order"

/**
 * Helper function to get the order option.
 * Calls for example WObj_option_set(obj_child, WStack_order, "10");
 * @param obj_child
 * @return order the higher: the later rendered / resorted to back (defaults to 0)
 */
osize WStack_child_order(oobj obj_child);

/**
 * Helper function to set the order option.
 * Calls for example WObj_option_set(obj_child, WStack_order_KEY, "10");
 * @param obj_child
 * @return order the higher: the later rendered / resorted to back (defaults to 0)
 */
osize WStack_child_order_set(oobj obj_child, osize order);


typedef struct {
    WObj super;

    bool window_auto_mode;
} WStack;


/**
 * Initializes the object
 * @param obj WStack object
 * @param parent to inherit from
 * @return obj casted as WStack
 */
WStack *WStack_init(oobj obj, oobj parent);


/**
 * Creates a new WStack object
 * @param parent to inherit from
 * @return The new object
 */
o_inline WStack *WStack_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(WStack, parent);
}

//
// virtual implementations:
//


/**
 * Virtual implementation that updates its children according to WObj_list (which is overridden).
 * Uses WObj__update_list_stacked with that list.
 */
vec2 WStack__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);

/**
 * Virtual implementation that returns a resorted list of children, according to the children order
 */
oobj *WStack__v_list(oobj obj, osize *opt_out_num);


//
// object functions:
//

/**
 * @param obj WStack object
 * @return if true and a WWindow child is currently dragged:
 *         That WWindow's order is set the max order +1
 */
OObj_DECL_GETSET(WStack, bool, window_auto_mode)


#endif //W_WSTACK_H
