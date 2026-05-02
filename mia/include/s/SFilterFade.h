#ifndef S_SFILTERFADE_H
#define S_SFILTERFADE_H

/**
 * @file SFilterFade.h
 *
 * Object (derives SFilter)
 *
 * Filter that fades the amplitude of the track data in or out
 *
 * To use a SFilter, simply use that STrack(Buf;Mix) as its parent
 */


#include "SFilter.h"

/** object id */
#define SFilterFade_ID SFilter_ID "Fade."


typedef struct {
    SFilter super;

    float gain_a, gain_b;
    oi64 frame_a, frame_b;

} SFilterFade;


/**
 * Initializes the object
 * @param obj SFilterFade object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @return obj casted as SFilter
 */
SFilterFade *SFilterFade_init(oobj obj, oobj opt_parent2, oobj parent);

/**
 * Creates a new SFilterFade object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @return The new object
 */
o_inline SFilterFade *SFilterFade_new(oobj parent, oobj opt_parent2)
{
    OObj_DECL_IMPL_NEW(SFilterFade, parent, opt_parent2);
}


//
// virtual implementations:
//

/**
 * Virtual function to filter from the track.
 * Fades the amplitude in or out.
 * @param obj SFilter object
 * @param data frames (=samples*channels) to fill in data pulled by the track
 * @patam frames number of frames to filter
 * @param spec audio spec passed in data
 */
void SFilterFade__v_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec);

//
// object functions
//


/**
 * Just sets SFilter_frame_time_set to 0 to auto set it again
 * @param obj SFilterFade object
 */
o_inline void SFilterFade_reset(oobj obj)
{
    SFilter_frame_time_set(obj, 0);
}

/**
 * Sets up the fading
 * @param obj SFilterFade object
 * @param gain_a, gain_b: amplitudes for position a and b
 * @param frame_a, frame_b: relative track sample position for a and b (start_frame)
 * @param update_frame_end SFilter_frame_end will be set to frame_b to auto remove this filter from the STrack
 */
void SFilterFade_set_ex(oobj obj, float gain_a, float gain_b, oi64 frame_a, oi64 frame_b, bool update_frame_end);

/**
 * Sets up the fading using seconds as time not samples
 * @param obj SFilterFade object
 * @param track STrack object to gat the spec from
 * @param gain_a, gain_b: amplitudes for position a and b
 * @param time_a, time_b: relative track time in seconds for a and b (start_frame)
 * @param update_frame_end SFilter_frame_end will be set to frame_b to auto remove this filter from the STrack
 */
void SFilterFade_set(oobj obj, oobj track, float gain_a, float gain_b, double time_a, double time_b, bool update_frame_end);

#endif //S_SFILTERFADE_H
