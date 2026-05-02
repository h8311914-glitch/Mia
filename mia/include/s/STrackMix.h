#ifndef S_STRACKMIX_H
#define S_STRACKMIX_H

/**
 * @file STrackMix.h
 *
 * Object (derives STrack)
 *
 * Mixes all children of type STrack from its own OObj tree.
 * May delete an STrack child if it had ended
 *
 * Resulting sound is amplified using STrackMix_gain and SFilter's are applied if present
 *
 * In module "s" context, "sample" is a single float value
 *                        "frame" count samples * channels and are typically setup to play 48000 frames each second.
 *                        "block" are frames * S_BLOCK_SIZE, some filters and systems use them internally.
 *
 * SFilter can be applied to this STrack by using it as the SFilter's parent
 *
 * @sa STrack.h, STrackBuf.h, SBuf.h
 */

#include "s/STrack.h"
#include "s/SFilter.h"


/** object id */
#define STrackMix_ID STrack_ID "Mix."


typedef struct
{
    STrack super;
    
    int arena;
    
    float *pulltmp;
    osize pulltmp_cap;

    float gain;
    
} STrackMix;

/**
 * Initializes the object
 * @param obj STrackMix object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param opt_spec specification to be used, NULL for system spec
 * @return obj casted as STrackMix
 */
STrackMix *STrackMix_init(oobj obj, oobj parent, oobj opt_parent2, const struct s_spec *opt_spec);

/**
 * Creates a new STrackMix object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param opt_spec specification to be used, NULL for system spec
 * @return The new object
 */
o_inline STrackMix *STrackMix_new(oobj parent, oobj opt_parent2, const struct s_spec *opt_spec)
{
    OObj_DECL_IMPL_NEW(STrackMix, parent, opt_parent2, opt_spec);
}

//
// virtual implementations
//

/**
 * Mixes all children STrack's.
 * Runs given SFilter's.
 * Amplifies with its gain.
 */
bool STrackMix__v_pull(oobj obj, float *restrict out_data, oi64 frames);



//
// object functions
//


/**
 * Get the children allocated on this Mixer
 * @param obj STrackMix object
 * @param opt_out_num if not NULL, returns the list size/num
 * @return A null terminated list of the STrack children, call o_free(obj, list) to free it manually
 * @note the list is allocated on obj.
 *       Create an STrack **it=list to traverse through the list
 */
o_inline STrack **STrackMix_list(oobj obj, osize *opt_out_num) {
    return (STrack **) OObj_list(obj, opt_out_num, STrack);
}

/**
 * Get the SFilter children allocated on this Mixer to be applied
 * @param obj STrackMix object
 * @param opt_out_num if not NULL, returns the list size/num
 * @return A null terminated list of the SFilter children, call o_free(obj, list) to free it manually
 * @note the list is allocated on obj.
 *       Create an SFilter **it=list to traverse through the list
 */
o_inline SFilter **STrackMix_filter_list(oobj obj, osize *opt_out_num) {
    return (SFilter **) OObj_list(obj, opt_out_num, SFilter);
}

/**
 * @param obj STrackMix object
 * @return maximal allowed children
 *         If it exceeds that limit,
 *         the first children gets deleted in pull,
 *         until limit is met.
 *         defaults to 128
 */
OObj_DECL_GETSET(STrackMix, int, arena)


/**
 * @param obj STrackMix object
 * @return amplification that is applied in pull. (defaults to 1.0)
 */
OObj_DECL_GETSET(STrackMix, float, gain)




#endif //S_STRACKMIX_H
