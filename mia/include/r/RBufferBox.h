#ifndef R_RBUFFERBOX_H
#define R_RBUFFERBOX_H

/**
 * @file RBuffer.h
 *
 * Object
 *
 * An RBuffer initialized for struct r_box
 */

#include "RBuffer.h"


/** object id */
#define RBufferBox_ID RBuffer_ID "Box."

typedef struct {
    RBuffer super;

} RBufferBox;


/**
 * Initializes the object
 * @param obj RBuffer object
 * @param parent to inherit from
 * @return obj casted as RBuffer
 */
RBufferBox *RBufferBox_init(oobj obj, oobj parent);

/**
 * Creates a new RBuffer object
 * @param parent to inherit from
 * @return The new object
 */
o_inline RBufferBox *RBufferBox_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(RBufferBox, parent);
}


#endif //R_RBUFFERBOX_H
