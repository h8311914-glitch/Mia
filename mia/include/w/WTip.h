#ifndef W_WTIP_H
#define W_WTIP_H

/**
 * @file WTip.h
 *
 * Object (derives WObj)
 *
 * Widget that renders a tooltip like text in a floating alpha blended WPane.
 * It aligns the pane within its min_size with an offset to the given (pointer) pos.
 * Defaults to centered below.
 */

#include "WObj.h"

/** object id */
#define WTip_ID WObj_ID "Tip."


typedef struct {
    WObj super;

    // options
    bool lt_on_unit;
    vec2 pos;
    float offset;
    float alpha;

    oobj pane;
    oobj text;
} WTip;


/**
 * Initializes the object
 * @param obj WTip object
 * @param parent to inherit from
 * @return obj casted as WTip
 */
WTip *WTip_init(oobj obj, oobj parent);


/**
 * Creates a new WTip object
 * @param parent to inherit from
 * @return The new object
 */
o_inline WTip *WTip_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(WTip, parent);
}

//
// virtual implementations:
//


/**
 * Virtual implementation that renders the floating tip
 * Does >not< update WObj childern, only its internal WPane!
 *     Typically, all widgets do update all children,
 *     but there seems to be no need for this special Widget to do so.
 */
vec2 WTip__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);


//
// object functions:
//

/**
 * @param obj WTip object
 * @return if true, lt is rounded to a unit (default)
 */
OObj_DECL_GETSET(WTip, bool, lt_on_unit)

/**
 * @param obj WTip object
 * @return (pointer) pos
 */
OObj_DECL_GETSET(WTip, vec2, pos)

/**
 * @param obj WTip object
 * @return offset of the tooltip from the (pointer) pos
 */
OObj_DECL_GETSET(WTip, float, offset)

/**
 * @param obj WTip object
 * @return blended alpha value
 */
OObj_DECL_GETSET(WTip, float, alpha)

/**
 * @param obj WTip object
 * @return Internal WPane object
 */
OObj_DECL_GET(WTip, oobj, pane)

/**
 * @param obj WTip object
 * @return Internal WText object
 */
OObj_DECL_GET(WTip, oobj, text)

/**
 * Just a wrapper to:
 *     WText_text_set(WTip_text(obj), text);
 * @param obj WTip object
 * @param text tooltip text to set (wrapped via WORD)
 */
void WTip_set(oobj obj, const char *text);

#endif //W_WTIP_H
