#ifndef O_OEVENT_H
#define O_OEVENT_H

/**
 * @file OEvent.h
 *
 * Object (derives OJoin)
 * 
 * Events can be used for (threadsafe) callbacks.
 * As an example OEventFetch which runs async until the response is available.
 * In contrast to a simple stupid callback, 
 *   it makes use of the object tree and does not call the function if it gets deleted.
 * A user can either pass a callback function, 
 *   which is called on the main thread (typically at the start of a frame)
 *   Or just poll for OEvent_ready() to check if the event got posted (or both...).
 * To work with a custom event, either pass data to o_user, or use named object children or inherit from OEvent.
 *   (Like OEventFetch or OEventFiledialog)
 *
 * An OEvent uses a heap allocator to enable threas safety. 
 * That allocator may differ from the one you are using 
 *     (like a faster pool allocator; default in a/app stuff).
 * To keep the result data, either don't delete the OEvent (OEvent_done) or create a new OJoin on your tree using the heap allocator.
 *     And then move the desired result into that.
 *     (Cause moving is only valid within the same allocator...)
 *
 * If the event is used by another thread, so not forget to enable threadsafe for the OEvent
 *     With OObj_threadsafe_set
 *
 * @warn callbacks work only by default if mia's app module is used.
 *       To handle em without see OEvent.c for the protected functions.
 */

#include "OJoin.h"


/** object id */
#define OEvent_ID OJoin_ID "OEvent."


typedef struct {
    OJoin super;
    oobj parent;
    bool ready;
    bool auto_done;
    OObj__event_fn opt_on_ready;
    oobj opt_on_ready_context;
} OEvent;


/**
 * Creates a new OEvent object.
 * This function can be used to init a derived OJoin (for thread safe stuff as an example)
 * @param object_size size to allocate (asserts >= sizeof(OJoin)
 * @param parent to inherit from
 * @param opt_on_ready event on_ready function, or NULL to ignore
 * @param opt_context optional OContext applied before calling on_ready will be clones, like o_context()
 *                    If apps need a_view, or a_cam, or a_pointer, or want to create an AScene's,
 *                        a valid app state is needed, pass o_context() to capture the current.
 * @return The new object
 * @note do not forget to set OObj_threadsafe if used in threads
 * @sa o_user_set to set void* data for the on_ready function
 * @threadsafe
 */
OEvent* OEvent_new_super(osize object_size, oobj parent, OObj__event_fn opt_on_ready, oobj opt_context);

/**
 * Creates a new OEvent object
 * @param parent to inherit from
 * @param opt_on_ready event on_ready function, or NULL to ignore
 * @param opt_context optional OContext applied before calling on_ready will be cloned, like o_context()
 *                    If apps need a_view, or a_cam, or a_pointer, or want to create an AScene's,
 *                        a valid app state is needed, pass o_context() to capture the current.
 * @return The new object
 * @note do not forget to set OObj_threadsafe if used in threads
 * @sa o_user_set to set void* data for the on_ready function
 * @threadsafe
 */
o_inline OEvent *OEvent_new(oobj parent, OObj__event_fn opt_on_ready, oobj opt_context)
{
    return OEvent_new_super(sizeof(OEvent), parent, opt_on_ready, opt_context);
}


/**
 * Creates a new OEvent object and directly calls OEvent_post.
 * @param parent to inherit from
 * @param opt_on_ready event on_ready function, or NULL to ignore
 * @param opt_on_ready_context optional OContext applied before calling on_ready
 *                    If apps need a_view, or a_cam, or a_pointer, or want to create an AScene's,
 *                        a valid app state is needed, pass o_context() to capture the current.
 * @param user_data passed via o_user_set to the event
 * @return The new object
 * @warn not threadsafe
 * @sa o_user_set to set void* data for the on_ready function
 */
OEvent *OEvent_new_post(oobj parent, OObj__event_fn opt_on_ready, oobj opt_on_ready_context, void *user_data);


/**
 * Creates a new OEvent object, to o_del an object.
 * Directly posts this OEvent!
 * @param parent to inherit from
 * @param obj_to_del_deferred OObj object to delete deferred
 * @return The new object. If created in another thread, it's directly invalid
 * @note uses o_user to save the obj for deletion
 *       Calls OEvent_post internally
 * @warn not threadsafe
 */
OEvent *OEvent_new_post_del(oobj parent, oobj obj_to_del_deferred);


//
// virtual implementations:
//

/**
 * Checks if its ready and logs if not (may be a bug; not posted)
 * @param obj OEvent object
 */
void OEvent__v_del(oobj obj);


//
// object functions:
//

/**
 * Sets the event to ready.
 * If a callback was passed, this gets called in the o_looper.
 * Also OEvent_ready gets true to consume the result.
 * @param obj OEvent object
 * @threadsafe
 * @warning In a threaded context, the OEvent should only be accessed from the consuming thread after calling this!
 */
void OEvent_post(oobj obj);

/**
 * Checks if the event data is ready to be consumed (OEvent_post() was called)
 * @param obj OEvent object, NULL safe (returns false)
 * @return true if ready (OEvent_post() was called)
 * @threadsafe
 */
bool OEvent_ready(oobj obj);

/**
 * Waits for the event to be completed.
 * @param obj OEvent object
 * @param opt_timeout_ms timeout in millis, or <=0 to auto set it to oi32_MAX
 * @return true if ready (false on timeout)
 */
bool OEvent_ready_wait(oobj obj, oi32 opt_timeout_ms);

/**
 * @param obj OEvent object
 * @return the parent who is responsible for this OEvent.
 * @note this function should be called from the parent side, not thread side
 */
OObj_DECL_GET(OEvent, oobj, parent)

/**
 * @param obj OEvent object
 * @return (default=false) Calls OEvent_done automatically after the (optional) callback (if OEvent_post)
 * @note this function should be called from the parent side, not thread side
 */
OObj_DECL_GETSET(OEvent, bool, auto_done)

/**
 * Removes the main parent from the OEvent join.
 * This is typically called after handling the event after it was ready. In that case it may delete it.
 */
o_inline void OEvent_done(oobj obj)
{
    OObj_assert(obj, OEvent);
    OEvent *self = obj;
    OJoin_remove(self, self->parent);
}

#endif //O_OEVENT_H
