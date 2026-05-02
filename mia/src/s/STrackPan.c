#include "s/STrackPan.h"
#include "o/OObj_builder.h"
#include "o/OEvent.h"
#include "m/sca/flt.h"

#undef O_LOG_LIB
#define O_LOG_LIB "s"
#include "o/log.h"


//
// public
//

STrackPan *STrackPan_init(oobj obj, oobj parent, oobj opt_parent2, const struct s_spec *opt_spec)
{
    STrackPan *self = obj;
    o_clear(self, sizeof *self, 1);

    STrack_init(obj, parent, opt_parent2, STrackPan__v_pull, opt_spec);
                 
    OObj_id_set(self, STrackPan_ID);

    self->pan = 0.0f;
    self->channel_offset = 0;
    
    assert(STrack_spec(self).channels == 2);

    return self;
}

//
// virtual implementations:
//

bool STrackPan__v_pull(oobj obj, float *restrict out_data, oi64 frames)
{
    OObj_assert(obj, STrackPan);
    STrackPan *self = obj;

    int channels = self->super.spec.channels;
    
    // init silence
    o_clear(out_data, sizeof(float) * channels, frames);
    

    STrack *child = STrackPan_child(self).o;
    if(!child) {
        return true;
    }
    
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
    
    // panning:
    int child_channels = child->spec.channels;
    float pan = m_clamp(self->pan, -1.0f, +1.0f);
    int offset = o_clamp(self->channel_offset, 0, (child_channels-1));
    
    // [-1 : +1] -> [0° : 90°]
    float pan_rad = (0.5+pan*0.5) * 0.5*m_PI;
    
    for(int f=0; f<frames; f++) {
        float src = self->pulltmp[f*child_channels+offset];
        
        // L
        out_data[f*2+0] = m_cos(pan_rad);
        
        // R
        out_data[f*2+1] = m_sin(pan_rad);
    }

    return ended;
}

//
// object functions
//
