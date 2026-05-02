#ifndef S_SFILTERNORMALIZE_H
#define S_SFILTERNORMALIZE_H

/**
 * @file SFilterNormalize.h
 *
 * Object (derives SFilter)
 *
 * Filter that normalizes using root mean square (rms)
 * Also smoothes the resulting gain.
 * So like a double smooth.
 *
 * Just ignores runs that avg on all samples of all channels for simplicity
 *
 * To use a SFilter, simply use that STrack(Buf;Mix) as its parent
 */


#include "SFilter.h"

/** object id */
#define SFilterNormalize_ID SFilter_ID "Normalize."


typedef struct {
    SFilter super;

    float rms;
    float rms_alpha;
    float norm;
    float norm_alpha;
    float norm_max;
    
    // user gain
    float gain;

} SFilterNormalize;


/**
 * Initializes the object.
 * @param obj SFilterNormalize object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @return obj casted as SFilter
 */
SFilterNormalize *SFilterNormalize_init(oobj obj, oobj parent, oobj opt_parent2);

/**
 * Creates a new SFilterNormalize object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @return The new object
 */
o_inline SFilterNormalize *SFilterNormalize_new(oobj parent, oobj opt_parent2)
{
    OObj_DECL_IMPL_NEW(SFilterNormalize, parent, opt_parent2);
}


//
// virtual implementations:
//

/**
 * Virtual function to filter from the track.
 * Normalizes the audio data using rms (like a running avg)
 * @param obj SFilterNormalize object
 * @param data frames (=samples*channels) to fill in data pulled by the track
 * @patam frames number of frames to filter
 * @param spec audio spec passed in data
 * @return true if end was reached
 */
void SFilterNormalize__v_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec);

//
// object functions
//

/**
 * Clears the blk ring to 0
 * @param obj SFilterNormalize object
 */
void SFilterNormalize_reset(oobj obj);


/**
 * @param obj SFilterNormalize object
 * @return alpha for rms, defaults to 0.001
 */
OObj_DECL_GETSET(SFilterNormalize, float, rms_alpha)

/**
 * @param obj SFilterNormalize object
 * @return alpha for norm, defaults to 0.01
 */
OObj_DECL_GETSET(SFilterNormalize, float, norm_alpha)

/**
 * @param obj SFilterNormalize object
 * @return alpha for norm, defaults to 20.0
 */
OObj_DECL_GETSET(SFilterNormalize, float, norm_max)

/**
 * @param obj SFilterNormalize object
 * @return applied after normalizing (defaults to 1.0)
 */
OObj_DECL_GETSET(SFilterNormalize, float, gain)

#endif //S_SFILTERNORMALIZE_H
