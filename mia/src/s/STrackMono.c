#include "s/STrackMono.h"
#include "o/OObj_builder.h"
#include "o/OEvent.h"
#include "m/vec/vecn.h"

#undef O_LOG_LIB
#define O_LOG_LIB "s"
#include "o/log.h"


//
// public
//

STrackMono *STrackMono_init(oobj obj, oobj parent, oobj opt_parent2, const struct s_spec *opt_spec)
{
    STrackMono *self = obj;
    o_clear(self, sizeof *self, 1);

    STrack_init(obj, parent, opt_parent2, STrackMono__v_pull, opt_spec);
                 
    OObj_id_set(self, STrackMono_ID);

    self->gain = 1.0f;
    
    assert(STrack_spec(self).channels == 1);

    return self;
}

//
// virtual implementations:
//

bool STrackMono__v_pull(oobj obj, float *restrict out_data, oi64 frames)
{
    OObj_assert(obj, STrackMono);
    STrackMono *self = obj;

    int channels = self->super.spec.channels;
    
    // init silence
    o_clear(out_data, sizeof(float) * channels, frames);
    

    STrack *child = STrackMono_child(self).o;
    if(!child) {
        return true;
    }

    int child_channels = child->spec.channels;
    
    assert(self->super.spec.freq == child->spec.freq
            && "see STrackResample to fix this");
    
    osize array_size = s_spec_array_size(child->spec, frames);
    if(self->pulltmp_cap < array_size) {
        self->pulltmp = o_renew(self, self->pulltmp, float, array_size);
        self->pulltmp_cap = array_size;
    }
    
    bool ended = STrack_pull(child, self->pulltmp, frames);
    if (ended) {
        o_log_debug_s(__func__, "deleting by ended");
        o_del(child);
    }
    
    // monoing
    float gain = self->gain;
    
    for(oi64 f=0; f<frames; f++) {
        const float *src = &self->pulltmp[f*child_channels];

        // mono output        
        out_data[f] = gain * vecn_norm(src, child_channels);
    }

    return ended;
}

//
// object functions
//
