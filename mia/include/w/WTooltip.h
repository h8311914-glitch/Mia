#ifndef W_WTOOLTIP_H
#define W_WTOOLTIP_H

/**
 * @file WTooltip.h
 *
 * Object (derives WObj)
 *
 * Widget that gathers WObj_tooltip via WObj_pick automatically from the children tree.
 * Uses WTip to render it
 */

#include "WObj.h"

/** object id */
#define WTooltip_ID WObj_ID "Tooltip."


typedef struct {
    WObj super;

    // options
    bool hidden;
    bool auto_gather;

    // OObj to break the WObj update hierarchy (to allocate tip)
    oobj container;

    // WTip, not part of WObj_list
    oobj tip;

    // OList of WObj trees to auto check for showing tooltips
    oobj gui_list;

    int gather_state;
    float gather_delay;
    float gather_delay_add;
    oobj gather_current;
    vec2 gather_pos_start;
    bool gather_ignore;

} WTooltip;


/**
 * Initializes the object
 * @param obj WTooltip object
 * @param parent to inherit from
 * @return obj casted as WTooltip
 */
WTooltip *WTooltip_init(oobj obj, oobj parent);


/**
 * Creates a new WTooltip object
 * @param parent to inherit from
 * @return The new object
 */
o_inline WTooltip *WTooltip_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(WTooltip, parent);
}

//
// virtual implementations:
//


/**
 * Virtual implementation.
 * Calls the default WObj__v_update.
 * And updates its internal WTip.
 */
vec2 WTooltip__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);


//
// object functions:
//

/**
 * @param obj WTooltip object
 * @return true to hide the toolbox (but not the children in contast to WObj_hide)
 *         defaults to false
 */
OObj_DECL_GETSET(WTooltip, bool, hidden)

/**
 * @param obj WTooltip object
 * @return true to let automatically set WTip according to the picked WObj_tooltip.
 *              Including fade effects and all.
 *              (Default)
 */
OObj_DECL_GETSET(WTooltip, bool, auto_gather)



/**
 * @param obj WTooltip object
 * @return Internal WTip object
 */
OObj_DECL_GET(WTooltip, oobj, tip)

/**
 * @param obj WTooltip object
 * @return OList of WObj gui trees to auto check for showing pointer tooltips automatically gathered.
 *         Defaults to {self}, clear to disable the check
 *         The gui's in that list must all share the same pointer_fn (same camera / view)
 *         To disable auto gathering, just call OList_clear(...)
 */
OObj_DECL_GET(WTooltip, oobj, gui_list)

#endif //W_WTOOLTIP_H
