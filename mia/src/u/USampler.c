#include "u/USampler.h"
#include "o/OObj_builder.h"

//
// public
//


USampler *USampler_init(oobj obj, oobj parent, osize element_size, 
        USampler__interpolate_fn interpolate_fn,
        USampler__times_fn times_fn)
{
    USampler *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, USampler_ID);

    self->element_size = element_size;
    
    // vfuncs
    self->v_interpolate = interpolate_fn;
    self->v_times = times_fn;

    return self;
}

void USampler_times_start_end(oobj obj, float *out_start_time, float *out_end_time)
{
    osize num;
    float *times = USampler_times(obj, &num);
    assert(num>0);
    *out_start_time = times[0];
    *out_end_time = times[num-1];
    o_free(obj, times);
}
