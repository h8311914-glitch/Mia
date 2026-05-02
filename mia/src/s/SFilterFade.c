#include "s/SFilterFade.h"
#include "o/OObj_builder.h"
#include "m/vec/vecn.h"
#include "s/STrack.h"


SFilterFade *SFilterFade_init(oobj obj, oobj parent, oobj opt_parent2)
{
    SFilterFade *self = obj;
    o_clear(self, sizeof *self, 1);

    SFilter_init(obj, parent, opt_parent2, SFilterFade__v_apply);
    OObj_id_set(self, SFilterFade_ID);

    self->frame_a = self->frame_b = 0;
    self->gain_a = self->gain_b = 0;

    return self;
}

//
// virtual implementations:
//

void SFilterFade__v_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec)
{
    OObj_assert(obj, SFilterFade);
    SFilterFade *self = obj;


    for(int i=0; i<frames; i++) {
        float *i_data = &data[i * spec.channels];
        oi64 i_frame = self->super.frame_time + i;
        float t = (float) (i_frame - self->frame_a) / (float) (self->frame_b - self->frame_a);
        t = m_clamp(t, 0.0f, 1.0f);
        float gain = m_mix(self->gain_a, self->gain_b, t);
        vecn_scale(i_data, i_data, gain, spec.channels);
    }
}

//
// object functions
//

void SFilterFade_set_ex(oobj obj, float gain_a, float gain_b, oi64 frame_a, oi64 frame_b, bool update_frame_end)
{
    OObj_assert(obj, SFilterFade);
    SFilterFade *self = obj;
    assert(frame_b>=frame_a);
    self->gain_a = gain_a;
    self->gain_b = gain_b;
    self->frame_a = frame_a;
    self->frame_b = frame_b;
    if(update_frame_end) {
        SFilter_frame_end_set(self, frame_b);
    }
}

void SFilterFade_set(oobj obj, oobj track, float gain_a, float gain_b, double time_a, double time_b, bool update_frame_end)
{
    struct s_spec spec = STrack_spec(track);
    oi64 frame_a = s_spec_frames_from_seconds(spec, time_a);
    oi64 frame_b = s_spec_frames_from_seconds(spec, time_b);
    SFilterFade_set_ex(obj, gain_a, gain_b, frame_a, frame_b, update_frame_end);
}
