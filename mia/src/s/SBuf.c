#include "s/SBuf.h"
#include "o/OObj_builder.h"
#include "o/OArray.h"


SBuf *SBuf_init_array(oobj obj, oobj parent, oobj array_sink, const struct s_spec *opt_spec)
{
    SBuf *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, SBuf_ID);


    self->spec = opt_spec ? *opt_spec : s_spec_default();

    OObj_assert(array_sink, OArray);
    assert(OArray_element_size(array_sink) == sizeof(float) * self->spec.channels);
    self->array = array_sink;
    o_move(array_sink, self);

    return self;
}

SBuf *SBuf_new(oobj parent, float *data, oi64 smp, const struct s_spec *opt_spec)
{
    struct s_spec spec = opt_spec ? *opt_spec : s_spec_default();
    return SBuf_new_array(parent, OArray_new(parent, data, sizeof(float) * spec.channels, smp), &spec);
}

//
// object functions
//

SBuf *SBuf_as_spec(oobj obj, struct s_spec spec)
{
    oobj self_array = SBuf_array(obj);
    struct s_spec self_spec = SBuf_spec(obj);
    
    oi64 self_smp = OArray_num(self_array);
    assert(OArray_element_size(self_array) == sizeof(float) * self_spec.channels);
    oi64 smp = s_resample_output_frames(spec, self_spec, self_smp);
    oobj array = OArray_new(obj, NULL, sizeof(float) * spec.channels, smp);
    s_resample(OArray_data_void(array), spec, OArray_data_void(self_array), self_spec, self_smp);
    return SBuf_new_array(obj, array, &spec);
}
