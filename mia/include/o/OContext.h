#ifndef O_OCONTEXT_H
#define O_OCONTEXT_H

/**
 * @file OContext.h
 *
 * Object
 *
 * Context holding some state infos of a given time.
 * Like current global a_stage, a_view, ...
 * Nothing here is thread safe, or using OPtr or OJoin or smth.
 * So its on you to use it with caution and not apply with dead / deleted objs
 */


#include "OObj.h"

/** object id */
#define OContext_ID OObj_ID "OContext."


struct OContext_state {
    OObj__event_fn fn;
    oobj obj;
};

typedef struct {
    OObj super;

    // ODict of struct OCentext_state with string keys like "a_cam"
    oobj map;
} OContext;


/**
 * Initializes the object
 * @param obj OContext object
 * @param parent to inherit from
 * @param opt_clone_from OContext to clome the map from (not full deep)
 * @return obj casted as OContext
 */
OContext *OContext_init(oobj obj, oobj parent, oobj opt_clone_from);

/**
 * Creates a new OContext object
 * @param parent to inherit from
 * @param opt_clone_from OContext to clome the map from (not full deep)
 * @return The new object
 */
o_inline OContext *OContext_new(oobj parent, oobj opt_clone_from)
{
    OObj_DECL_IMPL_NEW(OContext, parent, opt_clone_from);
}


//
// object functions:
//

/**
 * @param obj OContext object
 * @return the internal OMap object
 */
OObj_DECL_GET(OContext, oobj, map)

/**
 * Applys current context (calls all fn with its obj for all states in the map)
 * @param obj OContext object
 */
void OContext_apply(oobj obj);

/**
 * Cached fast path, preferable
 * @param obj OContext object
 * @param idx already created map pair idx (OContext_set_key)
 * @param state_fn executed with state_obj on apply (pass NULL to disable)
 * @param state_obj obj passed with the state_fn (NULL is fine and fn will still be executed)
 */
void OContext_set_at(oobj obj, osize idx, OObj__event_fn state_fn, oobj state_obj);

/**
 * @param obj OContext object
 * @param key to create or overwrite (must be a const static string, ala "")
 * @param state_fn executed with state_obj on apply (pass NULL to disable)
 * @param state_obj obj passed with the state_fn (NULL is fine and fn will still be executed)
 * @return Associated map pair index
 * @note consider using the return index for upcomming calls of OContext_set_at
 */
osize OContext_set_key(oobj obj, const char *key, OObj__event_fn state_fn, oobj state_obj);

/**
 * Calls OMap_clear to remove all entries fron the map
 * @param obj OContext object
 */
void OContext_clear(oobj obj);

/**
 * @param obj OContext object
 * @param clone_from an OContext to be cloned from.
 * @note old entries are either overwritten or still avaible.
 *       Call OContext_clear to remove em
 */
void OContext_clone(oobj obj, oobj clone_from);

#endif //O_OCONTEXT_H
