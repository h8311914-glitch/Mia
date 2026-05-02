#ifndef O_OWEAKJOIN_H
#define O_OWEAKJOIN_H

/**
 * @file OWeakjoin.h
 *
 * Object
 *
 * Can create a weak reference to an OJoin object and become a temporary parent for it.
 *
 * Can be used for shared resources like a traditional weak ptr with ref counting
 *
 * @sa OJoin
 */


#include "OJoin.h"


/** object id */
#define OWeakjoin_ID OObj_ID "OWeakjoin."


typedef struct {
    OObj super;

    // this is a custom shared lock (uses OObjRoot), shared with OJoin
    // Cause the OWeakjoin needs to be able to lock and access an OJoin (which may be deleted)
    oobj lock;

    // OJoin object to be referenced weakly, or NULL if it got deleted
    oobj join;
} OWeakjoin;


/**
 * Initializes the object
 * @param obj OWeakjoin object
 * @param parent to inherit from
 * @param join OJoin object to reference
 * @return obj casted as OWeakjoin
 */
OWeakjoin *OWeakjoin_init(oobj obj, oobj parent, oobj join);

/**
 * Creates a new OWeakjoin object
 * @param parent to inherit from
 * @param join OJoin object to reference
 * @return The new object
 */
o_inline OWeakjoin *OWeakjoin_new(oobj parent, oobj join)
{
    OObj_DECL_IMPL_NEW(OWeakjoin, parent, join);
}

//
// virtual implementations:
//

/**
 * Removes the reference to the shared lock object and may also delete it
 * @param obj OWeakjoin object
 * @threadsafe
 */
void OWeakjoin__v_del(oobj obj);

//
// object functions:
//

/**
 * @param obj
 * @return true is the join is available and not deleted yet.
 *         Simply checks if .join != NULL...
 */
o_inline bool OWeakjoin_available(oobj obj)
{
    OObj_assert(obj, OWeakjoin);
    OWeakjoin *self = obj;
    bool available;
    o_lock_block(self) {
        available = self->join != NULL;
    }
    return available;
}

/**
 * Sets this object as parent for the OJoin, if possible
 * @param obj OWeakjoin object
 * @return the OJoin object if acquired, NULL if failed (already deleted)
 * @threadsafe
 */
struct oobj_opt OWeakjoin_acquire(oobj obj);

/**
 * Removes this object from the parent list of OJoin, if possible
 * @param obj OWeakjoin object
 * @return false on failure (was not acquired)
 * @threadsafe
 * @warn the oobj returned by OWeakjoin_acquire will get immediately invalid
 */
bool OWeakjoin_release(oobj obj);



#endif //O_OWEAKJOIN_H
