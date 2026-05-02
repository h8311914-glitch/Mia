#ifndef W_WBOX_H
#define W_WBOX_H

/**
 * @file WBox.h
 *
 * Object (derives WObj)
 *
 * Widget that renders children in a boxed layout aligned next to each other.
 */

#include "WObj.h"

/** object id */
#define WBox_ID WObj_ID "Box."

/**
 * Different layout modes for the box.
 * H: simply align all children right to each other (horizontal); min_size.y is passed
 * V: as H, but down each other (vertical); min_size.x is passed
 * H_V: align children right to each other until min_size is reached, then add a new vertical line
 * V_H: as H_V but vertical first
 * H_WEIGTHS: makes use of min_size and weights to align the children, passes the new cell size
 * V_WEIGTHS: as H_WEIGHTS but vertical
 *
 * @note for layout *_WEIGHTS uses a min_size cache to estimate the available room.
 *       It auto resets if (and layout is *_WEIGHTS):
 *           - min_size <= 0 is passed on update
 *           - children num changed
 *           - WBox_weights_cached_min_size_clear is called
 */
enum WBox_layout {
    WBox_H,
    WBox_V,
    WBox_H_V,
    WBox_V_H,
    WBox_H_WEIGHTS,
    WBox_V_WEIGHTS,
    WBox_ENUM_MAX,
};

#define WBox_weight_KEY "WGrid_weight"

/**
 * Helper function to get the child weight option.
 * Calls for example WObj_option_set(obj_child, WBox_weight_KEY, "2.0");
 * @param obj_child WObj child of WBox
 * @param default_weight returned if not available
 * @return child weight
 */
float WBox_child_weight(oobj obj_child, float default_weight);

/**
 * Helper function to set the cell key option.
 * Calls for example WObj_option_set(obj_child, WBox_weight_KEY, "2.0");
 * @param obj_child WObj child of WBox
 * @return child weight, defaults to -1.0
 */
float WBox_child_weight_set(oobj obj_child, float weight);


typedef struct {
    WObj super;

    enum WBox_layout layout;
    vec2 spacing;
    
    float weight_default;
    
    // optional OArray of vec2
    oobj weights_cached_min_size;
} WBox;


/**
 * Initializes the object.
 * @param obj WBox object
 * @param parent to inherit from
 * @param layout order to place the children
 * @return obj casted as WBox
 */
WBox *WBox_init(oobj obj, oobj parent, enum WBox_layout layout);


/**
 * Creates a new WBox object
 * @param parent to inherit from
 * @param layout order to place the children
 * @return The new object
 */
o_inline WBox *WBox_new(oobj parent, enum WBox_layout layout)
{
    OObj_DECL_IMPL_NEW(WBox, parent, layout);
}

/**
 * Creates a new WBox object with a WBox_H (horizontal) layout
 * @param parent to inherit from
 * @return The new object
 */
o_inline WBox *WBox_new_h(oobj parent)
{
    return WBox_new(parent, WBox_H);
}

/**
 * Creates a new WBox object with a WBox_V (vertical) layout
 * @param parent to inherit from
 * @return The new object
 */
o_inline WBox *WBox_new_v(oobj parent)
{
    return WBox_new(parent, WBox_V);
}

//
// virtual implementations:
//

/**
 * Virtual implementation that updates the positions of the children.
 * Calls WObj_update for each child.
 * Uses min_size for H_V or V_H and passes 0 as min size to the children
 */
vec2 WBox__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);

//
// object functions:
//

/**
 * @param obj WBox object
 * @return current layout mode to order the children placement
 */
OObj_DECL_GETSET(WBox, enum WBox_layout, layout)

/**
 * @param obj WBox object
 * @return current spacing between each child
 */
OObj_DECL_GETSET(WBox, vec2, spacing)

/**
 * @param obj WBox object
 * @return default weight of the childs for *_WEIGHTS modes, defaults to -1.0f
 */
OObj_DECL_GETSET(WBox, float, weight_default)

/**
 * Clears the min size cache to reset it.
 * (For layout *_WEIGHTS only)
 * @param obj WBox object
 */
void WBox_weights_cached_min_size_clear(oobj obj);


#endif //W_WBOX_H
