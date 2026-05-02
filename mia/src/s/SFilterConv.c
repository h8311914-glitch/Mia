#include "s/SFilterConv.h"
#include "o/OObj_builder.h"
#include "m/vec/vecn.h"
#include "m/utils/kernel.h"
#include "o/log.h"
#include "s/STrack.h"


SFilterConv *SFilterConv_init(oobj obj, oobj parent, oobj opt_parent2, int kernel_size)
{
    SFilterConv *self = obj;
    o_clear(self, sizeof *self, 1);

    SFilter_init(obj, parent, opt_parent2, SFilterConv__v_apply);
    OObj_id_set(self, SFilterConv_ID);

    assert(kernel_size>1);

    self->kernel = o_new0(self, float, kernel_size);
    self->kernel[kernel_size-1] = 1.0;
    self->kernel_size = kernel_size;

    self->cache = NULL;
    self->cache_channels = 0;

    return self;
}

//
// virtual implementations:
//

void SFilterConv__v_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec)
{
    OObj_assert(obj, SFilterConv);
    SFilterConv *self = obj;

    if (self->cache_channels != spec.channels) {
        if (self->cache_channels>0) {
            o_log_warn_s(__func__, "resetting cache due to channel switch");
        }
        self->cache_channels = spec.channels;
        self->cache = o_renew(self, self->cache, float, spec.channels * self->kernel_size);
        o_clear(self->cache, sizeof(float) * spec.channels, self->kernel_size);
        self->cache_pos = 0;
    }

    for (oi64 i=0; i<frames; i++) {
        float *i_data = &data[i * spec.channels];
        float *cache_pos_data = &self->cache[self->cache_pos*spec.channels];
        o_memcpy(cache_pos_data, i_data, sizeof(float) * spec.channels, 1);
        o_clear(i_data, sizeof(float) * spec.channels, 1);

        for (int k=0; k<self->kernel_size; k++) {
            // positive modulo
            int cidx = o_mod(self->cache_pos - self->kernel_size + k, self->kernel_size);
            float *c_data = &self->cache[cidx*spec.channels];
            for (int channel=0; channel<spec.channels; channel++) {
                i_data[channel] += c_data[channel] * self->kernel[k];
            }
        }
        self->cache_pos++;
        self->cache_pos %= self->kernel_size;
    }
}

//
// object functions
//

void SFilterConv_reset(oobj obj)
{
    OObj_assert(obj, SFilterConv);
    SFilterConv *self = obj;
    o_clear(self->cache, sizeof(float) * self->cache_channels, self->kernel_size);
    self->cache_pos = 0;
}
