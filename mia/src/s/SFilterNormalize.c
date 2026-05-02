#include "s/SFilterNormalize.h"
#include "o/OObj_builder.h"
#include "m/sca/flt.h"


SFilterNormalize *SFilterNormalize_init(oobj obj, oobj parent, oobj opt_parent2)
{
    SFilterNormalize *self = obj;
    o_clear(self, sizeof *self, 1);

    SFilter_init(obj, parent, opt_parent2, SFilterNormalize__v_apply);
    OObj_id_set(self, SFilterNormalize_ID);

    self->rms = 1.0f;
    self->rms_alpha = 0.0001f;
    self->norm = 0.5f;
    self->norm_alpha = 0.001f;
    self->norm_max = 20.0f;

    self->gain = 1.0f;

    return self;
}

//
// virtual implementations:
//

void SFilterNormalize__v_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec)
{
    OObj_assert(obj, SFilterNormalize);
    SFilterNormalize *self = obj;

    oi64 samples = frames * spec.channels;

    for(oi64 i=0; i<samples; i++) {
        float *sample = &data[i];
     
        float rms = *sample * *sample;
        self->rms = m_mix(self->rms, rms, self->rms_alpha);

        rms = m_sqrt(self->rms);
        float norm = m_abs(*sample) / m_max(rms, 1e-6);
        norm = m_min(norm, self->norm_max);
        self->norm = m_mix(self->norm, norm, self->norm_alpha);

        *sample *= self->norm * self->gain;
    }
}

//
// object functions
//
