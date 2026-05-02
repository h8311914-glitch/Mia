#ifndef O_OPTR_H
#define O_OPTR_H

/**
 * @file OPtr.h
 *
 * Object
 *
 * Creates a weak reference to an OObj.
 * WARNING: THIS OBJECT IS >NOT< THREADSAFE!
 *          If you want something like a threadsafe ptr, have a look at OJoin and OWeakjoin.
 */


#include "OObj.h"

/** object id */
#define OPtr_ID OObj_ID "OPtr."


typedef struct {
    OObj super;

    // may be set to NULL with an ODelcallback
    oobj ptr;

    // ODelcallback installed on ptr
    oobj ptr_delcallback;
} OPtr;


/**
 * Initializes the object
 * @param obj OPtr object
 * @param parent to inherit from
 * @param ptr OObj to get the pointer from, an ODelcallback is installed on it (NULL safe)
 * @return obj casted as OPtr
 */
OPtr *OPtr_init(oobj obj, oobj parent, oobj ptr);

/**
 * Creates a new OPtr object
 * @param parent to inherit from
 * @param ptr OObj to get the pointer from, an ODelcallback is installed on it (NULL safe)
 * @return The new object
 */
o_inline OPtr *OPtr_new(oobj parent, oobj ptr)
{
    OObj_DECL_IMPL_NEW(OPtr, parent, ptr);
}

//
// virtual implementations:
//

/**
 * Object deletor that updates the ODelcallback installed on ptr
 * @param obj OPtr object
 */
void OPtr__v_del(oobj obj);

//
// object functions:
//


/**
 * @param obj OPtr object
 * @return the OObj ptr or NULL, if it was deleted
 */
o_inline struct oobj_opt OPtr_get(oobj obj)
{
    OObj_assert(obj, OPtr);
    OPtr *self = obj;
    return oobj_opt(self->ptr);
}

/**
 * @param obj OPtr object
 * @return true if OPtr_get would return the OObj, false if NULL
 */
o_inline bool OPtr_available(oobj obj)
{
    return OPtr_get(obj).o != NULL;
}

/**
 * @param obj OPtr object
 * @param ptr a new OObj to point to (in which an ODelcallback will be installed) or NULL
 * @note o_del's the old callback
 */
void OPtr_set(oobj obj, oobj ptr);

#endif //O_OPTR_H
