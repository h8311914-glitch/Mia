#include "s/SFilter.h"
#include "o/OObj_builder.h"
#include "o/OPtr.h"
#include "o/OEvent.h"

#undef O_LOG_LIB
#define O_LOG_LIB "s"

#include "o/log.h"


SFilter *SFilter_init(oobj obj, oobj parent, oobj opt_parent2, SFilter__apply_fn apply_fn)
{
    SFilter *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, SFilter_ID);

    if (opt_parent2) {
        self->opt_parent2_ptr = OPtr_new(self, opt_parent2);
    }

    self->active = true;
    self->frame_time = 0;
    self->frame_end = oi64_MAX;

    // vfunc
    self->v_apply = apply_fn;

    return self;
}

void SFilter_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec)
{
    OObj_assert(obj, SFilter);
    SFilter *self = obj;

    if (self->opt_parent2_ptr && !OPtr_available(self->opt_parent2_ptr)) {
        self->opt_parent2_ptr = NULL;
        o_log_debug_s(__func__, "deleted by parent2");
        // crucial to have self as parent, so if it gets deleted before events are handled, its not called anymore
        OEvent_new_post_del(self, self);
        return;
    }

    if (self->active && self->frame_time < self->frame_end) {
        self->v_apply(self, data, frames, spec);
        self->frame_time += frames;
    }
}