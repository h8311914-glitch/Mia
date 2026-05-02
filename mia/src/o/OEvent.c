#include "o/OEvent.h"
#include "o/OObj_builder.h"
#include "o/OObjRoot.h"
#include "o/OList.h"
#include "o/OContext.h"
#include "o/OWeakjoin.h"


#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


//
// protected callback handler
//

static struct {
    oobj root;

    // OList of OWeakjoin's of Event's
    oobj list;
} o_event_L;

// protected, called by a/app
void o__event_init(void)
{
    assert(!o_event_L.root);
    // new unregistered root
    o_event_L.root = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    OObj_threadsafe_set(o_event_L.root, true);
    o_event_L.list = OList_new(o_event_L.root, NULL, 0);
}


static void event_run(OEvent *event, oobj undo_context)
{
    o_lock_block(event) {
        if (event->ready && OJoin_is_parent(event, event->parent)) {
            if (event->opt_on_ready) {
                if (event->opt_on_ready_context) {
                    OContext_apply(event->opt_on_ready_context);
                }
                event->opt_on_ready(event);
                if (event->opt_on_ready_context) {
                    OContext_apply(undo_context);
                }
                event->opt_on_ready = NULL;
            }
            if (event->auto_done) {
                o_log_debug_s("OEvent__run", "auto done");
                OEvent_done(event);
            }
        }
    }
}

// protected, called by a/app
void o__event_run_callbacks(oobj undo_context)
{
    o_thread_assert_main();

    o_lock(o_event_L.list);

    // the list may grow from posting stuff in events.
    // so recollect OList_num each comparsion
    // critical is creating the first AScene
    for (osize i = 0; i < OList_num(o_event_L.list); i++) {
        oobj weak = OList_at(o_event_L.list, i);
        oobj ev = OWeakjoin_acquire(weak).o;
        if (ev) {
            event_run(ev, undo_context);
            OWeakjoin_release(weak);
        }
        o_del(weak);
    }

    OList_clear(o_event_L.list);

    o_unlock(o_event_L.list);
}

static void o__event_add_callback(oobj obj)
{
    o_lock_block(o_event_L.list) {
        oobj weak = OWeakjoin_new(o_event_L.root, obj);
        OList_push(o_event_L.list, weak);
    }
}

//
// private
//

static void deferred_delete(oobj obj)
{
    oobj to_delete = o_user(obj);
    o_del(to_delete);
    OEvent_done(obj);
}

//
// public
//

OEvent* OEvent_new_super(osize object_size, oobj parent, OObj__event_fn opt_on_ready, oobj opt_on_ready_context)
{
    assert(object_size >= (osize) sizeof(OEvent));
    OJoin *super = OJoin_new_super(object_size, &parent, 1, o_allocator_heap_new());
    OEvent *self = (OEvent *) super;
    OObj_id_set(self, OEvent_ID);

    OObj_threadsafe_set(self, true);

    self->parent = parent;
    self->opt_on_ready = opt_on_ready;
    self->ready = false;
    self->auto_done = false;

    if (opt_on_ready_context) {
        self->opt_on_ready_context = OContext_new(self, opt_on_ready_context);
    }

    // deletor
    self->super.super.v_del = OEvent__v_del;

    return self;
}


OEvent *OEvent_new_post(oobj parent, OObj__event_fn opt_on_ready, oobj opt_on_ready_context, void *user_data)
{
    OEvent *self = OEvent_new_super(sizeof(OEvent), parent, opt_on_ready, opt_on_ready_context);
    o_user_set(self, user_data);
    OEvent_post(self);
    return self;
}

OEvent *OEvent_new_post_del(oobj parent, oobj obj_to_del_deferred)
{
    OEvent *self = OEvent_new(parent, deferred_delete, NULL);
    o_user_set(self, obj_to_del_deferred);
    OEvent_post(self);
    return self;
}

//
// virtual implementations:
//

void OEvent__v_del(oobj obj)
{
    OObj_assert(obj, OEvent);
    OEvent *self = obj;

    if (!self->ready) {
        o_log_debug_s(__func__, "Forgot to call OEvent_post?");
    }

    OJoin__v_del(self);
}


//
// object functions:
//

void OEvent_post(oobj obj)
{
    OObj_assert(obj, OEvent);
    OEvent *self = obj;
    o_lock_block(self) {
        if (o_unlikely(self->ready)) {
            o_log_warn_s(__func__, "already posted");
        } else {
            o__event_add_callback(self);
        }
        self->ready = true;
    }
}


bool OEvent_ready(oobj obj)
{
    if (!obj) {
        return false;
    }
    OObj_assert(obj, OEvent);
    OEvent *self = obj;
    bool ready;
    o_lock_block(self) {
        ready = self->ready;
    }
    return ready;
}

bool OEvent_ready_wait(oobj obj, oi32 opt_timeout_ms)
{
    bool ready = false;
    if (opt_timeout_ms <= 0) {
        opt_timeout_ms = oi32_MAX;
    }
    for (int i = 0; i < opt_timeout_ms; i++) {
        ready = OEvent_ready(obj);
        if (ready) {
            break;
        }
        o_sleep(1);
    }
    return ready;
}
