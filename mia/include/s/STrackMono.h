#ifndef S_STRACKMONO_H
#define S_STRACKMONO_H

/**
 * @file STrackMono.h
 *
 * Object (derives STrack)
 *
 * Creates a mono track (channels==1) from the first child of type STrack from its own OObj tree.
 * From the child only a single channel is used for panning.
 * Mono'ing is done with the equal power rule (3db); using vecn_norm of the source
 * May delete an STrack child if it had ended
 *
 * In module "s" context, "sample" is a single float value
 *                        "frame" count samples * channels and are typically setup to play 48000 frames each second.
 *                        "block" are frames * S_BLOCK_SIZE, some filters and systems use them internally.
 *
 * @sa STrack.h, STrackMix.h, STrackBuf.h, SBuf.h
 */

#include "s/STrack.h"


/** object id */
#define STrackMono_ID STrack_ID "Mono."


typedef struct
{
    STrack super;
    
    float gain;
    
    float *pulltmp;
    osize pulltmp_cap;
    
} STrackMono;

/**
 * Initializes the object
 * @param obj STrackMono object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param opt_spec specification to be used, NULL for system spec
 * @return obj casted as STrackMono
 * @note asserts that the spec used has channels==1
 */
STrackMono *STrackMono_init(oobj obj, oobj parent, oobj opt_parent2, const struct s_spec *opt_spec);

/**
 * Creates a new STrackMono object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param opt_spec specification to be used, NULL for system spec
 * @return The new object
 * @note asserts that the spec used has channels==1
 */
o_inline STrackMono *STrackMono_new(oobj parent, oobj opt_parent2, const struct s_spec *opt_spec)
{
    OObj_DECL_IMPL_NEW(STrackMono, parent, opt_parent2, opt_spec);
}

//
// virtual implementations
//

/**
 * Monos the first STrack child
 */
bool STrackMono__v_pull(oobj obj, float *restrict out_data, oi64 frames);



//
// object functions
//


/**
 * Get the child allocated on this STrack to resample
 * @param obj STrackMono object
 * @return The first STrack child or NULL
 */
o_inline struct oobj_opt STrackMono_child(oobj obj) {
    return OObj_find(obj, STrack, NULL, 0);
}


/**
 * @param obj STrackMono object
 * @return amplification that is applied in pull. (defaults to 1.0)
 */
OObj_DECL_GETSET(STrackMono, float, gain)

#endif //S_STRACKMONO_H
