#include "s/STrackMix.h"
#include "o/OObj_builder.h"
#include "m/vec/vecn.h"
#include "s/SFilter.h"


#undef O_LOG_LIB
#define O_LOG_LIB "s"
#include "o/log.h"

static void trackmix_apply_filters(STrackMix *self, float *restrict data, oi64 frames)
{
    SFilter **list = STrackMix_filter_list(self, NULL);
    for(SFilter **it=list; *it; it++) {
        SFilter_apply(*it, data, frames, self->super.spec);
        if (SFilter_ended(*it)) {
            o_del(*it);
        }
    }
    o_free(self, list);
}


//
// public
//

STrackMix *STrackMix_init(oobj obj, oobj parent, oobj opt_parent2, const struct s_spec *opt_spec)
{
    STrackMix *self = obj;
    o_clear(self, sizeof *self, 1);

    STrack_init(obj, parent, opt_parent2, STrackMix__v_pull, opt_spec);
                 
    OObj_id_set(self, STrackMix_ID);

    self->arena = 128;
    
    int channels = self->super.spec.channels;

    self->gain = 1.0f;

    return self;
}

//
// virtual implementations:
//

bool STrackMix__v_pull(oobj obj, float *restrict out_data, oi64 frames)
{
    OObj_assert(obj, STrackMix);
    STrackMix *self = obj;

    int channels = self->super.spec.channels;
    
    // init silence
    o_clear(out_data, sizeof(float) * channels, frames);

    osize array_size = s_spec_array_size(self->super.spec, frames);
    if(self->pulltmp_cap < array_size) {
        self->pulltmp = o_renew(self, self->pulltmp, float, array_size);
        self->pulltmp_cap = array_size;
    }
    
    bool ended = true;
    
    osize list_len;
    STrack **list = STrackMix_list(self, &list_len);
    osize arena_pops = list_len - self->arena;
    
    for(STrack **it=list; *it; it++) {
        assert(s_spec_equals(self->super.spec, (*it)->spec)
            && "see STrackResample, STrackPan or STrackMono to fix this");

        // check max arena size, pops front
        if((*it)->endable && arena_pops>0) {
            o_log_debug_s(__func__, "deleting by arena");
            o_del(*it);
            arena_pops--;
            continue;
        }

        
        bool child_ended = STrack_pull(*it, self->pulltmp, frames);
        ended |= child_ended;

        if (child_ended) {
            o_log_debug_s(__func__, "deleting by ended");
            o_del(*it);
        }


        // mix
        s_mix_into(out_data, self->pulltmp, 1.0f, channels, frames);

    }
    
    o_free(self, list);

    // filters and gain
    trackmix_apply_filters(self, out_data, frames);
    
    vecn_scale(out_data, out_data, self->gain, channels * frames);


    return ended;
}

//
// object functions
//

