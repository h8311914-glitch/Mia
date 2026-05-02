#include "s/STrack.h"
#include "o/OObj.h"
#include "o/OObj_builder.h"
#include "o/OPtr.h"
#include "o/OEvent.h"

#undef O_LOG_LIB
#define O_LOG_LIB "s"

#include "o/log.h"


STrack *STrack_init(oobj obj, oobj parent, oobj opt_parent2, STrack__pull_fn pull_fn, const struct s_spec *opt_spec)
{
    STrack *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, STrack_ID);

    if (opt_parent2) {
        self->opt_parent2_ptr = OPtr_new(self, opt_parent2);
    }

    self->spec = opt_spec ? *opt_spec : s_spec_default();

    self->endable = false;

    // vfuncs
    self->v_pull = pull_fn;
    
    return self;
}

//
// object functions
//

bool STrack_pull(oobj obj, float *restrict out_data, oi64 frames)
{
    OObj_assert(obj, STrack);
    STrack *self = obj;

    if (self->opt_parent2_ptr && !OPtr_available(self->opt_parent2_ptr)) {
        self->opt_parent2_ptr = NULL;
        o_log_debug_s(__func__, "deleted by parent2");
        // crucial to have self as parent, so if it gets deleted before events are handled, its not called anymore
        OEvent_new_post_del(self, self);
        // silence data
        o_clear(out_data, sizeof(float) * self->spec.channels, frames);
        return true;
    }

    if(self->quiet) {
        o_clear(out_data, sizeof(float) * self->spec.channels, frames);
        return false;
    }
    bool ended = self->v_pull(self, out_data, frames);
    ended = ended && self->endable;
    return ended && self->endable;
}
