#include "s/STrackGen.h"
#include "o/OObj_builder.h"
#include "m/vec/vecn.h"
#include "m/utils/random.h"
#include "s/SFilter.h"


#undef O_LOG_LIB
#define O_LOG_LIB "s"
#include "o/log.h"

static void trackgen_apply_filters(STrackGen *self, float *restrict data, oi64 frames)
{
    SFilter **list = STrackGen_filter_list(self, NULL);
    for(SFilter **it=list; *it; it++) {
        SFilter_apply(*it, data, frames, self->super.spec);
        if (SFilter_ended(*it)) {
            o_del(*it);
        }
    }
    o_free(self, list);
}

static void trackgen_wave(STrackGen *self, float *restrict data, oi64 frames)
{
    float t_smp = self->freq / self->super.spec.freq;
    int channels = self->super.spec.channels;
    float gain = self->gain;
    
    float t = self->t;
    for(int i=0;i<frames;i++) {
        float *sample = data+(i*channels);
        t = m_fract(t+t_smp);
        float val = gain * m_signal_wave(t);
        vecn_set(sample, val, channels);
    }
    self->t = t;
}

static void trackgen_saw(STrackGen *self, float *restrict data, oi64 frames)
{
    float t_smp = self->freq / self->super.spec.freq;
    int channels = self->super.spec.channels;
    float gain = self->gain;
    
    float t = self->t;
    for(int i=0;i<frames;i++) {
        float *sample = data+(i*channels);
        t = m_fract(t+t_smp);
        float val = gain * m_signal_saw(t);
        vecn_set(sample, val, channels);
    }
    self->t = t;
}

static void trackgen_ramp(STrackGen *self, float *restrict data, oi64 frames)
{
    float t_smp = self->freq / self->super.spec.freq;
    int channels = self->super.spec.channels;
    float gain = self->gain;
    
    float t = self->t;
    for(int i=0;i<frames;i++) {
        float *sample = data+(i*channels);
        t = m_fract(t+t_smp);
        float val = gain * m_signal_ramp(t);
        vecn_set(sample, val, channels);
    }
    self->t = t;
}

static void trackgen_block(STrackGen *self, float *restrict data, oi64 frames)
{
    float t_smp = self->freq / self->super.spec.freq;
    int channels = self->super.spec.channels;
    float gain = self->gain;
    
    float t = self->t;
    for(int i=0;i<frames;i++) {
        float *sample = data+(i*channels);
        t = m_fract(t+t_smp);
        float val = gain * m_signal_block(t);
        vecn_set(sample, val, channels);
    }
    self->t = t;
}

static void trackgen_noise(STrackGen *self, float *restrict data, oi64 frames)
{
    int channels = self->super.spec.channels;
    float gain = self->gain;
    for(int i=0;i<frames;i++) {
        float *sample = data+(i*channels);
        float val = gain * m_random_range(-1, +1);
        vecn_set(sample, val, channels);
    }
}


//
// public
//


STrackGen *STrackGen_init(oobj obj, oobj parent, oobj opt_parent2, enum STrackGen_mode mode, float freq, const struct s_spec *opt_spec)
{
    STrackGen *self = obj;
    o_clear(self, sizeof *self, 1);

    STrack_init(obj, parent, opt_parent2, STrackGen__v_pull, opt_spec);
    OObj_id_set(self, STrackGen_ID);

    self->mode = mode;
    self->freq = freq;
    self->gain = 1.0f;

    return self;
}

//
// virtual implementations:
//

bool STrackGen__v_pull(oobj obj, float *restrict out_data, oi64 frames)
{
    OObj_assert(obj, STrackGen);
    STrackGen *self = obj;

    switch(self->mode) {
    default:
    case STrackGen_WAVE:
        trackgen_wave(self, out_data, frames);
        break;
    case STrackGen_SAW:
        trackgen_saw(self, out_data, frames);
        break;
    case STrackGen_RAMP:
        trackgen_ramp(self, out_data, frames);
        break;
    case STrackGen_BLOCK:
        trackgen_block(self, out_data, frames);
        break;
    case STrackGen_NOISE:
        trackgen_noise(self, out_data, frames);
        break;
    }
    
    // filters
    trackgen_apply_filters(self, out_data, frames);
    
    // gain is applied while generating

    return false;
}


//
// object functions
//

