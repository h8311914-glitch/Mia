#ifndef W_WSELECT_H
#define W_WSELECT_H

/**
 * @file WSelect.h
 *
 * Object (derives WObj)
 *
 * Widget that renders a list of WBtns in a WPane to click to select an item.
 */

#include "WObj.h"

/** object id */
#define WSelect_ID WObj_ID "Select."


/**
 * Called for each list item to fill the button with widgets.
 * @param select WSelect object to be created,
 *               access to user data via o_user
 * @param btn WBtn to be used as parent for inmer widgets like a WText
 * @param i list number [0:list_num)
 */
typedef void (*WSelect__creator_fn)(oobj select, oobj btn, int idx);


typedef struct {
    WObj super;

    oobj pane;
    
    oobj box;
    
    oobj header;
    
    // array of WBtns
    oobj *btns;
    
    oobj footer;
    
    int btns_num;
    
    int clicked;
    
} WSelect;


/**
 * Initializes the object.
 * @param obj WSelect object
 * @param parent to inherit from
 * @param creator_fn called for each btn to create the widgets inside it
 * @param num list items
 * @param user_data passed to o_user of this WSelect (useable in the creator_fn)
 * @return obj casted as WSelect
 */
WSelect *WSelect_init(oobj obj, oobj parent, WSelect__creator_fn creator_fn, int num, void *user_data);


/**
 * Creates a new WSelect object.
 * @param parent to inherit from
 * @param creator_fn called for each btn to create the widgets inside it
 * @param num list items
 * @param user_data passed to o_user of this WSelect (useable in the creator_fn)
 * @return The new object
 */
o_inline WSelect *WSelect_new(oobj parent,  WSelect__creator_fn creator_fn, int num, void *user_data)
{
    OObj_DECL_IMPL_NEW(WSelect, parent, creator_fn, num, user_data);
}

/**
 * Creates a new WSelect object.
 * Each btn is created with a simple text inside it.
 * @param parent to inherit from
 * @param btn_text_list a text for each btn
 * @param opt_list_num either <=0 to use the list as null terminated list, or its size
 * @return The new object
 * @note Each WTexr is padded by 1 unit arount it
 */
WSelect *WSelect_new_text(oobj parent, char **btn_text_list, int opt_list_num);

//
// virtual implementations:
//


/**
 * Virtual implementation.
 * Calls the default WObj__v_update.
 */
vec2 WSelect__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);


//
// object functions:
//


/**
 * @param obj WSelect object
 * @return WObj above (before) the buttons
 *         Useful to display a header WText
 */
OObj_DECL_GET(WSelect, oobj, header)

/**
 * @param obj WSelect object
 * @return WObj behind (after) the buttons
 *         Useful to display a footer WText
 */
OObj_DECL_GET(WSelect, oobj, footer)


/**
 * @param obj WSelect object
 * @return Last clicked WBtn index, inits with -1
 */
OObj_DECL_GET(WSelect, int, clicked);


#endif //W_WSELECT_H
