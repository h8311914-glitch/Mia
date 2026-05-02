#ifndef S_STRACKPAN_H
#define S_STRACKPAN_H

/**
 * @file STrackPan.h
 *
 * Object (derives STrack)
 *
 * Creates a stereo (channels==2) track from the first child of type STrack from its own OObj tree.
 * From the child only a single channel is used for panning.
 * Panning is done with the equal power panning rule (3db); using L=cos and R=sib curves
 * May delete an STrack child if it had ended
 *
 * @sa STrack.h, STrackMix.h, STrackBuf.h, SBuf.h
 */

#include "s/STrack.h"


/** object id */
#define STrackPan_ID STrack_ID "Pan."


typedef struct
{
    STrack super;
    
    // clamped to [-1 : +1]
    // with -1=L 0=center +1=R and all inbetween...
    float pan;
    int channel_offset;
    
    float *pulltmp;
    osize pulltmp_cap;
    
} STrackPan;

/**
 * Initializes the object
 * @param obj STrackPan object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param opt_spec specification to be used, NULL for system spec
 * @return obj casted as STrackPan
 * @note asserts that the spec used has channels==2
 */
STrackPan *STrackPan_init(oobj obj, oobj parent, oobj opt_parent2, const struct s_spec *opt_spec);

/**
 * Creates a new STrackPan object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param opt_spec specification to be used, NULL for system spec
 * @return The new object
 * @note asserts that the spec used has channels==2
 */
o_inline STrackPan *STrackPan_new(oobj parent, oobj opt_parent2, const struct s_spec *opt_spec)
{
    OObj_DECL_IMPL_NEW(STrackPan, parent, opt_parent2, opt_spec);
}

//
// virtual implementations
//

/**
 * Pans the first STrack child
 */
bool STrackPan__v_pull(oobj obj, float *restrict out_data, oi64 frames);



//
// object functions
//


/**
 * Get the child allocated on this STrack to resample
 * @param obj STrackPan object
 * @return The first STrack child or NULL
 */
o_inline struct oobj_opt STrackPan_child(oobj obj) {
    return OObj_find(obj, STrack, NULL, 0);
}

/**
 * @param obj STrackPan object
 * @return [-1 : +1] for the pan direction
 *         -1.0: fully left
 *         0.0: centered (default)
 *         +1.0: fully right
 */
OObj_DECL_GETSET(STrackPan, float, pan)

/**
 * @param obj STrackPan object
 * @return channel offset (defaults to 0) 
 *         if the child source STrack has multiple channels.
 *         Only a single channels is used.
 */
OObj_DECL_GETSET(STrackPan, int, channel_offset)


#endif //S_STRACKPAN_H
