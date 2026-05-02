#ifndef S_SRESAMPLER_H
#define S_SRESAMPLER_H

/**
 * @file SResampler.h
 *
 * Object
 *
 * Can resample from a specific audio spec to another.
 * Want a one time / not perf heavy resample?
 *     Have a look at o_resample() in common.h (internally uses this)
 */


#include "o/OObj.h"
#include "s/common.h"

/** object id */
#define SResampler_ID OObj_ID "SResampler."



typedef struct {
    OObj super;

    // const, do not change em
    struct s_spec_ex input_ex;
    struct s_spec_ex output_ex;

    // impl with sdl stuff
    void *impl;
} SResampler;


/**
 * Initializes the object
 * @param obj SResampler object
 * @param parent to inherit from
 * @param input_ex, output_ex audio specs (ex, including primitive type) to resample
 * @return obj casted as SResampler
 */
SResampler *SResampler_init_ex(oobj obj, oobj parent, struct s_spec_ex input_ex, struct s_spec_ex output_ex);

/**
 * Creates a new SResampler object
 * @param parent to inherit from
 * @param input_ex, output_ex audio specs (ex, including primitive type) to resample
 * @return The new object
 */
o_inline SResampler *SResampler_new_ex(oobj parent, struct s_spec_ex input_ex, struct s_spec_ex output_ex)
{
    OObj_DECL_IMPL_NEW_SPECIAL(SResampler, ex, parent, input_ex, output_ex);
}

/**
 * Creates a new SResampler object
 * @param parent to inherit from
 * @param input, output audio specs (using S_FORMAT_F32, as mias default) to resample
 * @return The new object
 */
o_inline SResampler *SResampler_new(oobj parent, struct s_spec input, struct s_spec output)
{
    struct s_spec_ex input_ex = {0};
    input_ex.spec = input;
    input_ex.format = S_FORMAT_F32;
    struct s_spec_ex output_ex = {0};
    output_ex.spec = output;
    output_ex.format = S_FORMAT_F32;
    return SResampler_new_ex(parent, input_ex, output_ex);
}

//
// virtual implementations:
//

/**
 * Frees sdl stuff
 * @param obj SResampler object
 */
void SResampler__v_del(oobj obj);

//
// object functions
//

/**
 * @param obj SResampler object
 * @return input spec (fixed)
 */
OObj_DECL_GET(SResampler, struct s_spec_ex, input_ex)

/**
 * @param obj SResampler object
 * @return output spec (fixed)
 */
OObj_DECL_GET(SResampler, struct s_spec_ex, output_ex)

/**
 * @param obj SResampler object
 * @return smp (samples) for output if used for resampling
 */
o_inline oi64 SResampler_output_smp(oobj obj, oi64 input_smp)
{
    OObj_assert(obj, SResampler);
    SResampler *self = obj;
    return (oi64) md_ceil((double) input_smp * (double) self->output_ex.spec.freq / (double) self->input_ex.spec.freq);
}

/**
 * Resamples a track accoring to input and output spec.
 * @param obj SResampler object
 */
void SResampler_resample(oobj obj, void *restrict out_output, const void *restrict input, oi64 input_smp);




#endif //S_SRESAMPLER_H
