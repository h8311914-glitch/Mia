#ifndef W_WCOMBO_H
#define W_WCOMBO_H

/**
 * @file WCombo.h
 *
 * Object (derives WObj)
 *
 * Stylized WBtn that opens a WSelect to select from.
 *
 * @note uses the WCombo's WStyle for the WSelect, not of widget_parent
 */

#include "WObj.h"
#include "WSelect.h"

/** object id */
#define WCombo_ID WObj_ID "Combo."

/**
 * Called for on change to fill the button with widgets.
 * @param combo WCombo object
 * @param widget_parent WObj to be used as parent for inner widgets like a WText.
 *                      Its a child of WBtn which gets recreated on each change.
 * @param idx list number to be applied
 */
typedef void (*WCombo__creator_fn)(oobj combo, oobj widget_parent, int idx);


typedef struct {
    WObj super;

    // WBtn (widget_container) and WIcon (dir down or up)
    oobj btn;
    oobj btn_container;
    oobj btn_icon;

    // WObj as child of btn, recreated on each change.
    oobj widget_parent;

    // some WObj with a wide field to create the WSelect in, like the main gui.
    // needs to have min_size set for aligning the WSelect accordingly
    // and needs to live as long as this WCombo lives
    oobj toplevel_parent;

    // WAlign and WSelect, or NULL
    oobj select_align;
    oobj select;

    // to create the WSelect
    WSelect__creator_fn select_creator_fn;
    int select_num;
    void *select_user_data;
    OObj__event_fn select_on_new;

    int idx;
    int changed_idx;

    WCombo__creator_fn creator_fn;
} WCombo;


/**
 * Initializes the object
 * @param obj WCombo object
 * @param parent to inherit from
 * @param toplevel_parent some WObj to create the WSelect (aligned) in.
 *                        Needs to live as long as this WCombo lives (some parent up the tree).
 *                        Also needs an adequate min_size room for creating and aligning the WSelect.
 *                        As an example, the toplevel gui as WObj.
 *                        But be aware that its not some layout widget like WBox...
 * @param creator_fn function to set the WBtn widgets on change
 * @param idx list number to be applied, passed to the creator_fn
 * @param select_creator_fn, select_num, select_user_data see WSelect
 * @return obj casted as WCombo
 */
WCombo *WCombo_init(oobj obj, oobj parent, oobj toplevel_parent, WCombo__creator_fn creator_fn, int idx,
                    WSelect__creator_fn select_creator_fn, int select_num, void *select_user_data);


/**
 * Creates a new WCombo object
 * @param parent to inherit from
 * @param toplevel_parent some WObj to create the WSelect (aligned) in.
 *                        Needs to live as long as this WCombo lives (some parent up the tree).
 *                        Also needs an adequate min_size room for creating and aligning the WSelect.
 *                        As an example, the toplevel gui as WObj.
 *                        But be aware that its not some layout widget like WBox...
 * @param creator_fn function to set the WBtn widgets on change
 * @param idx list number to be applied, passed to the creator_fn
 * @param select_creator_fn, select_num, select_user_data see WSelect
 * @return The new object
 */
o_inline WCombo *WCombo_new(oobj parent, oobj toplevel_parent, WCombo__creator_fn creator_fn, int idx,
                            WSelect__creator_fn select_creator_fn, int select_num, void *select_user_data)
{
    OObj_DECL_IMPL_NEW(WCombo, parent, toplevel_parent, creator_fn, idx, select_creator_fn, select_num, select_user_data);
}

/**
 * Creates a new WCombo object.
 * Each WSelect btn and the WCombo widgets are created with a simple text inside it.
 * @param parent to inherit from
 * @param toplevel_parent some WObj to create the WSelect (aligned) in.
 *                        Needs to live as long as this WCombo lives (some parent up the tree).
 *                        Also needs an adequate min_size room for creating and aligning the WSelect.
 *                        As an example, the toplevel gui as WObj.
*                        But be aware that its not some layout widget like WBox...
 * @param btn_text_list a text for each btn
 * @param opt_list_num either <=0 to use the list as null terminated list, or its size
 * @param idx list number to be applied, passed to the creator_fn
 * @return The new object
 */
WCombo *WCombo_new_text(oobj parent, oobj toplevel_parent, char **btn_text_list, int opt_list_num, int idx);

//
// virtual implementations:
//


/**
 * Virtual implementation.
 * Calls the default WObj__v_update.
 */
vec2 WCombo__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);


//
// object functions:
//

/**
 * @param obj WCombo object
 * @return WBtn for the combo button.
 */
OObj_DECL_GET(WCombo, oobj, btn)


/**
 * @param obj WCombo object
 * @return creator function to set the WBtn widgets on change
 */
OObj_DECL_SET(WCombo, WCombo__creator_fn, creator_fn)


/**
 * @param obj WCombo object
 * @return to create the WSelect
 */
OObj_DECL_SET(WCombo, WSelect__creator_fn, select_creator_fn)

/**
 * @param obj WCombo object
 * @return to create the WSelect
 */
OObj_DECL_GETSET(WCombo, int, select_num)

/**
 * @param obj WCombo object
 * @return to create the WSelect
 */
OObj_DECL_GETSET(WCombo, void*, select_user_data)

/**
 * @param obj WCombo object
 * @return to create the WSelect, called with this WCombo object if not NULL if select got created
 */
OObj_DECL_SET(WCombo, OObj__event_fn, select_on_new)


/**
 * @param obj WCombo object
 * @return WSelect if currently visible, or NULL
 */
o_inline struct oobj_opt WCombo_select(oobj obj)
{
    OObj_assert(obj, WCombo);
    WCombo *self = obj;
    return oobj_opt(self->select);
}


/**
 * @param obj WCombo object
 * @param idx list number to be applied, passed to the creator_fn
 */
void WCombo_set(oobj obj, int idx);

/**
 * @param obj WCombo object
 * @return current selected idx
 */
OObj_DECL_GET(WCombo, int, idx)

/**
 * @param obj WCombo object
 * @return true (a single time) if the idx has changed
 */
o_inline bool WCombo_changed(oobj obj)
{
    WCombo *self = obj;
    bool changed = self->changed_idx != self->idx;
    self->changed_idx = self->idx;
    return changed;
}

#endif //W_WCOMBO_H
