#ifndef U_USCENEBLUR_H
#define U_USCENEBLUR_H

/**
 * @file USceneBlur.h
 *
 * Object (inherits AScene)
.*
 * Scene that creates a gauss blur from blitted scenes below the stack.
 */


#include "a/AScene.h"

/** object id */
#define USceneBlur_ID AScene_ID "USceneBlur."


typedef struct {
    AScene super;

    bool handle_events;
    
    // set by the animator
    vec2 sigma;

    // UAnimator to animate the gauss vec2 sigma
    // default snoothly fades it into 2.5 within 0.5 seconds
    oobj animator;
} USceneBlur;


/**
 * Initializes the object.
 * @param obj USceneBlur object
 * @param parent to inherit from
 * @return obj casted as USceneBlur
 */
USceneBlur *USceneBlur_init(oobj obj, oobj parent);

/**
 * Creates a new USceneBlur object
 * @param parent to inherit from
 * @return The new object
 */
o_inline USceneBlur *USceneBlur_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(USceneBlur, parent);
}


//
// object functions:
//

/**
 * @param obj USceneBlur object
 * @return handles all events if true (default)
 */
OObj_DECL_GETSET(USceneBlur, bool, handle_events)

/**
 * @param obj USceneBlur object
 * @return UAnimator to animate the gauss vec2 sigma
 *         default snoothly fades it into 2.5 within 0.5 seconds
 */
OObj_DECL_GETSET(USceneBlur, oobj, animator)



#endif //U_USCENEBLUR_H
