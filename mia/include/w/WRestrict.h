#ifndef W_WRESTRICT_H
#define W_WRESTRICT_H

/**
 * @file WRestrict.h
 *
 * Object (derives WObj)
 *
 * Widget handles pointer events, depending on the mode, if that pointer (current) is in that widgets area.
 * Useful to avoid actions like drawing below that area.
 * Some widgets already handle pointer events, such as WBtn, but only (0, 0) so single touch without history.
 * WRestrict may also target multitouch and or history.
 *
 * @note another option may be to use "a_pointer_handled_forward_history()".
 *       Which forwards handled current events (from WBtn, etc.) to the full history.
 */

#include "WObj.h"
#include "a/pointer.h"

/** object id */
#define WRestrict_ID WObj_ID "Restrict."



enum WRestrict_pointer_mode {
    WRestrict_OFF,
    WRestrict_CURRENT,
    WRestrict_FULL_HISTORY,
    WRestrict_ENUM_MAX
};


typedef struct {
    WObj super;

    enum WRestrict_pointer_mode pointer_modes[a_pointer_MAX];
} WRestrict;


/**
 * Initializes the object.
 * Pointer mode defaults to 0==WRestrict_FULL_HISTORY, >0==WRestrict_OFF
 * @param obj WRestrict object
 * @param parent to inherit from
 * @return obj casted as WRestrict
 */
WRestrict *WRestrict_init(oobj obj, oobj parent);


/**
 * Creates a new WRestrict object.
 * Pointer mode defaults to 0==WRestrict_FULL_HISTORY, >0==WRestrict_OFF
 * @param parent to inherit from
 * @return The new object
 */
o_inline WRestrict *WRestrict_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(WRestrict, parent);
}

//
// virtual implementations:
//


/**
 * Virtual implementation handles pointer events.
 * Calls the default WObj__v_update (before handling, so children are able to receive events)
 */
vec2 WRestrict__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);


//
// object functions:
//

/**
 * @param obj WRestrict object
 * @param idx pointer idx
 * @return pointer mode for that pointer index, defaults to 0==WRestrict_FULL_HISTORY, >0==WRestrict_OFF
 * @note asserts bounds
 * @sa a/pointer.h
 */
o_inline enum WRestrict_pointer_mode WRestrict_pointer_mode(oobj obj, int idx)
{
    OObj_assert(obj, WRestrict);
    WRestrict *self = obj;
    assert(idx>=0 && idx<a_pointer_MAX);
    return self->pointer_modes[idx];
}

/**
 * @param obj WRestrict object
 * @param idx pointer idx, or <0 for all
 * @mode pointer mode for that pointer index, defaults to 0==WRestrict_FULL_HISTORY, >0==WRestrict_OFF
 * @note asserts bounds
 * @sa a/pointer.h
 */
void WRestrict_pointer_mode_set(oobj obj, int idx, enum WRestrict_pointer_mode mode);

#endif //W_WRESTRICT_H
