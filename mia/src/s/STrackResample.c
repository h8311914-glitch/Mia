#include "s/STrackResample.h"
#include "o/OObj_builder.h"
#include "s/SResampler.h"

#undef O_LOG_LIB
#define O_LOG_LIB "s"
#include "o/log.h"


//
// public
//

STrackResample *STrackResample_init(oobj obj, oobj parent, oobj opt_parent2, const struct s_spec *opt_spec)
{
    STrackResample *self = obj;
    o_clear(self, sizeof *self, 1);

    STrack_init(obj, parent, opt_parent2, STrackResample__v_pull, opt_spec);
                 
    OObj_id_set(self, STrackResample_ID);

    self->freq = 0;

    return self;
}

//
// virtual implementations:
//

bool STrackResample__v_pull(oobj obj, float *restrict out_data, oi64 frames)
{
    OObj_assert(obj, STrackResample);
    STrackResample *self = obj;

    int channels = self->super.spec.channels;

    STrack *child = STrackResample_child(self).o;
    if(!child) {
        // silence
        o_clear(out_data, sizeof(float) * channels, frames);
        return true;
    }

    struct s_spec child_spec = STrack_spec(child);
    if (self->freq>0) {
        child_spec.freq = self->freq;
    }

    // reversed
    osize src_frames = s_resample_output_frames(child_spec, self->super.spec, frames);

    // be sure to pull enough frames
    osize dst_frames;
    while ((dst_frames = s_resample_output_frames(self->super.spec, child_spec, src_frames)) < frames) {
        src_frames++;
    }

    osize pull_array_size = s_spec_array_size(child_spec, src_frames);
    if(self->pull_cache_cap < pull_array_size) {
        self->pull_cache = o_renew(self, self->pull_cache, float, pull_array_size);
        self->pull_cache_cap = pull_array_size;
    }

    bool ended = STrack_pull(child, self->pull_cache, src_frames);

    if (ended) {
        o_log_debug_s(__func__, "deleting by ended");
        o_del(child);
    }

    bool resampler_valid = self->lazy_resampler
        && s_spec_equals(SResampler_output_ex(self->lazy_resampler).spec, self->super.spec)
        && s_spec_equals(SResampler_output_ex(self->lazy_resampler).spec, self->super.spec);
    if (!resampler_valid) {
        o_del(self->lazy_resampler);
        self->lazy_resampler = SResampler_new(self, child_spec, self->super.spec);
    }
    SResampler_resample(self->lazy_resampler, out_data, self->pull_cache, src_frames);

    return ended;
}

//
// object functions
//
