#ifndef U_USAMPLERFLOAT_H
#define U_USAMPLERFLOAT_H

/**
 * @file USamplerVec.h
 *
 * Object (inherits USampler)
 *
 * Samples key frame floats
 */


#include "USampler.h"
#include "o/OArray.h"

/** object id */
#define USamplerVec_ID USampler_ID "Vec."

enum USamplerVec_mode {
    USamplerVec_LINEAR,
    USamplerVec_STEP,
    USamplerVec_SPLINE,
    USamplerVec_ENUM_MAX,
};

typedef struct {
    USampler super;

    enum USamplerVec_mode mode;

    // OArray of frames.
    // A frame always contains a USamplerVec_key with a list of floats as values.
    // The values are always [value1, value2, ..., tangent_in1, tangent_in2, ..., tangent_out1, tangent_out2, ...].
    oobj frames;
} USamplerVec;


/**
 * Initializes the object.
 * @param obj USamplerVec object
 * @param parent to inherit from
 * @param vec_size size of the float array to be interpolated
 * @param frames_array OArray of keyframes.
 *                     A frame always contains a USamplerVec_key with a list of floats as values.
 *                     The first entry is the float time.
 *                     The values are always [value1, value2, ..., tangent_in1, tangent_in2, ..., tangent_out1, tangent_out2, ...].
 * @param move_frames_array if true, frames is o_move'd into self
 * @param mode Interpolation mode
 * @return obj casted as USamplerVec
 */
USamplerVec *USamplerVec_init(oobj obj, oobj parent, osize vec_size,
                              oobj frames_array, bool move_frames_array, enum USamplerVec_mode mode);

/**
 * Creates a new USamplerVec object
 * @param parent to inherit from
 * @param vec_size size of the float array to be interpolated
 * @param frames_array OArray of keyframes.
 *                     A frame always contains a USamplerVec_key with a list of floats as values.
 *                     The first entry is the float time.
 *                     The values are always [value1, value2, ..., tangent_in1, tangent_in2, ..., tangent_out1, tangent_out2, ...].
 * @param move_frames_array if true, frames is o_move'd into self
 * @param mode Interpolation mode
 * @return The new object
 */
o_inline USamplerVec *USamplerVec_new(oobj parent, osize vec_size,
                                      oobj frames_array, bool move_frames_array, enum USamplerVec_mode mode)
{
    OObj_DECL_IMPL_NEW(USamplerVec, parent, vec_size, frames_array, move_frames_array, mode);
}

/**
 * Creates a new USamplerVec object
 * @param parent to inherit from
 * @param vec_size size of the float array to be interpolated
 * @param frames array keyframes like struct USampler_vec3[3].
 *               A frame always contains a USamplerVec_key with a list of floats as values.
 *               The first entry is the float time.
 *               The values are always [value1, value2, ..., tangent_in1, tangent_in2, ..., tangent_out1, tangent_out2, ...].
 * @param num is the number of frames passed
 * @param mode Interpolation mode
 * @return The new object
 */
USamplerVec *USamplerVec_new_frames(oobj parent, osize vec_size,
                                    const void *frames, int num, enum USamplerVec_mode mode);

//
// virtual implementations:
//

void USampleVec__v_interpolate(oobj obj, void *out_interpolated_element, float time);

float *USamplerVec__v_times(oobj obj, osize *opt_out_num);

//
// object functions:
//

/**
 * @param obj USamplerVec object
 * @return interpolation mode of this sampler, defaults to USamplerVec_LINEAR
 */
OObj_DECL_GETSET(USamplerVec, enum USamplerVec_mode, mode)

/**
 * @param obj USamplerVec object
 * @return OArray of keyframes
 */
OObj_DECL_GETSET(USamplerVec, oobj, frames)

/**
 * @param obj USamplerVec object
 * @return vec size of the sampler (==float array size passed to USampleVec__v_interpolate)
 */
o_inline osize USamplerVec_vec_size(oobj obj)
{
    return USampler_element_size(obj) / sizeof(float);
}


//
// typical frames:
//

struct USamplerVec_float {
    float time;
    float value;
    float tangent_in;
    float tangent_out;
};

struct USamplerVec_vec2 {
    float time;
    vec2 value;
    vec2 tangent_in;
    vec2 tangent_out;
};

struct USamplerVec_vec3 {
    float time;
    vec3 value;
    vec3 tangent_in;
    vec3 tangent_out;
};

struct USamplerVec_vec4 {
    float time;
    vec4 value;
    vec4 tangent_in;
    vec4 tangent_out;
};

#endif //U_USAMPLERFLOAT_H
