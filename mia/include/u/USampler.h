#ifndef U_USAMPLER_H
#define U_USAMPLER_H

/**
 * @file USampler.h
 *
 * Object
 *
 * The sampler obj is an abstract base object "interface" to interpolate elements on a specific time
 */


#include "a/AView.h"

/** object id */
#define USampler_ID OObj_ID "USampler."

/**
 * Interpolation vfunc
 * @param obj USampler object
 * @param out_interpolated_element result
 * @param time timestamp in seconds
 */
typedef void (*USampler__interpolate_fn)(oobj obj, void *out_interpolated_element, float time);

/**
 * start and end time vfunc
 * @param obj USampler object
 * @param opt_out_num returned array length
 * @return a -1 terminated float array of frame times, call o_free on it
 */
typedef float *(*USampler__times_fn)(oobj obj, osize *opt_out_num);

typedef struct {
    OObj super;

    osize element_size;
    
    // vfuncs
    USampler__interpolate_fn v_interpolate;
    USampler__times_fn v_times;
} USampler;


/**
 * Initializes the object.
 * @param obj USampler object
 * @param parent to inherit from
 * @param element_size necessary size for an element to pass to USampler_interpolate
 * @param interpolate_fn element interpolation function
 * @param times_fn start and end time
 * @return obj casted as USampler
 */
USampler *USampler_init(oobj obj, oobj parent, osize element_size, 
        USampler__interpolate_fn interpolate_fn,
        USampler__times_fn times_fn);

/**
 * Creates a new USampler object
 * @param parent to inherit from
 * @param element_size necessary size for an element to pass to USampler_interpolate
 * @param interpolate_fn element interpolation function
 * @param times_fn start and end time
 * @return The new object
 */
o_inline USampler *USampler_new(oobj parent, osize element_size, 
        USampler__interpolate_fn interpolate_fn,
        USampler__times_fn times_fn)
{
    OObj_DECL_IMPL_NEW(USampler, parent, element_size, interpolate_fn, times_fn);
}


//
// object functions:
//

/**
 * @param obj USampler object
 * @return necessary size for an element to pass to USampler_interpolate
 */
OObj_DECL_GET(USampler, osize, element_size)

/**
 * @param obj USampler object
 * @param out_interpolated_element the interpolated element
 * @param time in seconds
 */
o_inline void USampler_interpolate(oobj obj, void *out_interpolated_element, float time)
{
    OObj_assert(obj, USampler);
    USampler *self = obj;
    self->v_interpolate(obj, out_interpolated_element, time);
}


/**
* @param obj USampler object
 * @param opt_out_num returned array length
 * @return a -1 terminated float array of frame times, call o_free on it
 */
o_inline float *USampler_times(oobj obj, osize *opt_out_num)
{
    OObj_assert(obj, USampler);
    USampler *self = obj;
    return self->v_times(obj, opt_out_num);
}

/**
 * @param obj USampler object
 * @param out_start_time, out_end_time first and last frame time, uses USampler_times
 */
void USampler_times_start_end(oobj obj, float *out_start_time, float *out_end_time);



#endif //U_USAMPLER_H
