#ifndef S_STRACKRESAMPLE_H
#define S_STRACKRESAMPLE_H

/**
 * @file STrackResample.h
 *
 * Object (derives STrack)
 *
 * Resamples the first child of type STrack from its own OObj tree.
 * Frequency can also be overriden, see STrackResample_freq.
 *      Cool to change playback speed
 *
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
#define STrackResample_ID STrack_ID "Resample."


typedef struct
{
    STrack super;

    float *pull_cache;
    osize pull_cache_cap;

    oobj lazy_resampler;

    int freq;

} STrackResample;

/**
 * Initializes the object
 * @param obj STrackResample object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param opt_spec specification to be used, NULL for system spec
 * @return obj casted as STrackResample
 */
STrackResample *STrackResample_init(oobj obj, oobj parent, oobj opt_parent2, const struct s_spec *opt_spec);

/**
 * Creates a new STrackResample object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param opt_spec specification to be used, NULL for system spec
 * @return The new object
 */
o_inline STrackResample *STrackResample_new(oobj parent, oobj opt_parent2, const struct s_spec *opt_spec)
{
    OObj_DECL_IMPL_NEW(STrackResample, parent, opt_parent2, opt_spec);
}

//
// virtual implementations
//

/**
 * Resamples the first STrack child
 */
bool STrackResample__v_pull(oobj obj, float *restrict out_data, oi64 frames);



//
// object functions
//


/**
 * Get the child allocated on this STrack to resample
 * @param obj STrackResample object
 * @return The first STrack child or NULL
 */
o_inline struct oobj_opt STrackResample_child(oobj obj) {
    return OObj_find(obj, STrack, NULL, 0);
}

/**
 * @param obj STrackBuf object
 * @return freq override. <=0 to use child freq (default), else this will be used.
 *         Cool to change playback speed.
 *         (child has say 44100 and we just say it was 22050 so its played with 0.5x speed)
 */
OObj_DECL_GETSET(STrackResample, int, freq)


#endif //S_STRACKRESAMPLE_H
