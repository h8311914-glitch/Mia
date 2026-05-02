#include "o/OContext.h"
#include "o/OObj_builder.h"
#include "o/OMap.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

//
// public
//

OContext *OContext_init(oobj obj, oobj parent, oobj opt_clone_from)
{
    OContext *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OContext_ID);

    self->map = OMap_new_string_keys(self, sizeof(struct OContext_state), 64);

    if(opt_clone_from) {
        OContext_clone(self, opt_clone_from);
    }

    return self;
}


//
// object functions:
//

void OContext_apply(oobj obj)
{
    OObj_assert(obj, OContext);
    OContext *self = obj;
    osize num = OMap_num(self->map);
    for (osize i=0; i<num; i++) {
        struct OContext_state *state = OMap_value_at(self->map, i, struct OContext_state);
        if(state->fn) {
            state->fn(state->obj);
        }
    }
}

void OContext_set_at(oobj obj, osize idx, OObj__event_fn state_fn, oobj state_obj)
{
    OObj_assert(obj, OContext);
    OContext *self = obj;
    struct OContext_state state = {
        state_fn,
        state_obj
    };
    OMap_set_at(self->map, idx, &state);
}


osize OContext_set_key(oobj obj, const char *key, OObj__event_fn state_fn, oobj state_obj)
{
    OObj_assert(obj, OContext);
    OContext *self = obj;
    struct OContext_state state = {
        state_fn,
        state_obj
    };
    return OMap_set(self->map, &key, &state);
}

void OContext_clear(oobj obj)
{
    OObj_assert(obj, OContext);
    OContext *self = obj;
    OMap_clear(self->map);
}

void OContext_clone(oobj obj, oobj clone_from)
{
    OObj_assert(obj, OContext);
    OContext *self = obj;
    
    OObj_assert(clone_from, OContext);
    OContext *clone = clone_from;
    OMap_merge(self->map, clone->map);
}
