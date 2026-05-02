#ifndef S_SFILTERECHO_H
#define S_SFILTERECHO_H

/**
 * @file SFilterEcho.h
 *
 * Object (derives SFilter)
 *
 * Filter that caches the samples in a ring buffer.
 * Mixes in the previous played samples.
 * Simply ignores channels and just works on samples.
 * Before mixing in, SFilter children are applied on the pulled samples.
 *     As well as gain is applied on that pull after filtering (and before mixing)
 *
 * To use a SFilter, simply use that STrack(Buf;Mix) ad its parent
 */


#include "SFilter.h"

/** object id */
#define SFilterEcho_ID SFilter_ID "Echo."


typedef struct {
    SFilter super;

    float *ring;
    int ring_size;
    int ring_pos;

    float *mix_tmp;
    osize mix_tmp_cap;

    // clamped between [0:ring_size)
    int echo;

    float gain;

} SFilterEcho;


/**
 * Initializes the object.
 * @param obj SFilterEcho object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @param ring_size the samples to save in the ring, echo -> ring_size-1
 * @return obj casted as SFilter
 */
SFilterEcho *SFilterEcho_init(oobj obj, oobj parent, oobj opt_parent2, int ring_size);

/**
 * Creates a new SFilterEcho object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @param ring_size the samples to save in the ring, echo -> ring_size-1
 * @return The new object
 */
o_inline SFilterEcho *SFilterEcho_new(oobj parent, oobj opt_parent2, int ring_size)
{
    OObj_DECL_IMPL_NEW(SFilterEcho, parent, opt_parent2, ring_size);
}

/**
 * Creates a new SFilterEcho object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @param echo_time in [s], calcs the ring_size from it with whte parents spec
 * @return The new object
 */
SFilterEcho *SFilterEcho_new_time(oobj parent, oobj opt_parent2, double echo_time);


//
// virtual implementations:
//

/**
 * Creates an echo effect with mixing in previous samples, before mixing SFilter children are applies as well as a gain
 * @param obj SFilterEcho object
 * @param data frames (=samples*channels) to fill in data pulled by the track
 * @patam frames number of frames to filter
 * @param spec audio spec passed in data
 */
void SFilterEcho__v_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec);

//
// object functions
//

/**
 * Get the SFilter children allocated on this filter to be applied
 * @param obj SFilterEcho object
 * @param opt_out_num if not NULL, returns the list size/num
 * @return A null terminated list of the SFilter children, call o_free(obj, list) to free it manually
 * @note the list is allocated on obj.
 *       Create an SFilter **it=list to traverse through the list
 */
o_inline SFilter **SFilterEcho_filter_list(oobj obj, osize *opt_out_num) {
    return (SFilter **) OObj_list(obj, opt_out_num, SFilter);
}

/**
 * Clears the ring to silence
 * @param obj SFilterEcho object
 */
void SFilterEcho_reset(oobj obj);


/**
 * @param obj SFilterEcho object
 * @return echo distance in samples, clamped to [0:ring_size) with ring_size-1 as default
 */
OObj_DECL_GETSET(SFilterEcho, int, echo)


/**
 * @param obj SFilterEcho object
 * @return amplification that is applied before mixing in. (defaults to 1.0)
 */
OObj_DECL_GETSET(SFilterEcho, float, gain)



#endif //S_SFILTERECHO_H
