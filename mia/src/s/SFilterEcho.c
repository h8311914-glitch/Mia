#include "s/SFilterEcho.h"
#include "o/OObj_builder.h"
#include "m/sca/flt.h"
#include "s/STrack.h"


static void filterecho_apply_filters(SFilterEcho *self, float *restrict data, oi64 frames, struct s_spec spec)
{
    SFilter **list = SFilterEcho_filter_list(self, NULL);
    for(SFilter **it=list; *it; it++) {
        SFilter_apply(*it, data, frames, spec);
        if (SFilter_ended(*it)) {
            o_del(*it);
        }
    }
    o_free(self, list);
}

//
// public
//

SFilterEcho *SFilterEcho_init(oobj obj, oobj parent, oobj opt_parent2, int ring_size)
{
    SFilterEcho *self = obj;
    o_clear(self, sizeof *self, 1);

    SFilter_init(obj, parent, opt_parent2, SFilterEcho__v_apply);
    OObj_id_set(self, SFilterEcho_ID);

    assert(ring_size>=1);

    self->ring = o_new0(self, float, ring_size);
    self->ring_size = ring_size;

    self->echo = self->ring_size-1;
    self->gain = 1.0f;

    return self;
}

//
// virtual implementations:
//

void SFilterEcho__v_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec)
{
    OObj_assert(obj, SFilterEcho);
    SFilterEcho *self = obj;

    oi64 samples = frames * spec.channels;

    if (self->mix_tmp_cap<samples) {
        self->mix_tmp = o_renew(self, self->mix_tmp, float, samples);
        self->mix_tmp_cap = samples;
    }

    for(oi64 i=0; i<samples; i++) {
        self->ring[self->ring_pos++] = data[i];
        self->ring_pos %= self->ring_size;

        int pull_pos = o_mod(self->ring_pos - self->echo, self->ring_size);
        self->mix_tmp[i] = self->ring[pull_pos];
    }

    // filters
    filterecho_apply_filters(self, self->mix_tmp, frames, spec);

    // mix
    s_mix_into(data, self->mix_tmp, self->gain, spec.channels, frames);
}

SFilterEcho *SFilterEcho_new_time(oobj parent, oobj opt_parent2, double echo_time)
{
    struct s_spec spec = STrack_spec(parent);
    int ring_size = (int) m_ceil(echo_time * spec.freq);

    // we want the ring_size % spec.channels == 0
    // so we calc whats to much and add the remaining ceiled.
    int frame_basis_miss = ring_size % spec.channels;
    ring_size += (spec.channels-frame_basis_miss) % spec.channels;

    return SFilterEcho_new(parent, opt_parent2, ring_size);
}

//
// object functions
//

void SFilterEcho_reset(oobj obj)
{
    OObj_assert(obj, SFilterEcho);
    SFilterEcho *self = obj;
    o_clear(self->ring, sizeof(float), self->ring_size);
    self->ring_pos = 0;
}
