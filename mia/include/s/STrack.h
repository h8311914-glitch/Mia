#ifndef S_STRACK_H
#define S_STRACK_H

/**
 * @file STrack.h
 *
 * Object
 *
 * An abstraction for an audio track source.
 * 
 * STrackMix -> mixes multiple STrack children
 * STrackBuf -> plays an SBuf like a sound or music
 *
 * With STrack's we can build up a tree like:
 *
 * STrackMix (root)
 *   - STrackBuf (music)
 *   - STrackMix (boss mixer)
 *     - STrackBuf (boss effect 1)
 *     - STrackBuf (boss effect 2)
 *   - STrackMix (character)
 *     - STrackBuf (jmp sound)
 *     - STrackBuf (heal music)
 *
 * Each STrack may be filtered with SFilter or amplified.
 * So we can apply filters on whole tree branches.
 *
 * STrackMix may delete ended STracks
 * 
 * Using the s_trackmix() root tree is only valid on the main thread.
 *
 * In module "s" context, "sample" is a single float value
 *                        "frame" count samples * channels and are typically setup to play 48000 frames each second.
 *                        "block" are frames * S_BLOCK_SIZE, some filters and systems use them internally.
 *
 * @sa STrackMix.h, STrackBuf.h, SBuf.h
 */

#include "s/common.h"
#include "o/OObj.h"

/** object id */
#define STrack_ID OObj_ID "STrack."


/**
 * Virtual function to pull a block of samples from the track.
 * @param obj STrack object
 * @param out_data frames*STrack_spec(obj).channels to fill in data retrieved by this track
 * @param frames number of samples*channels
 * @return true if end was reached
 */
typedef bool (*STrack__pull_fn)(oobj obj, float *restrict out_data, oi64 frames);


typedef struct
{
    OObj super;

    // optional OPtr to another parent to autodelete
    oobj opt_parent2_ptr;

    struct s_spec spec;
    bool endable;
    bool quiet;

    // vfuncs
    STrack__pull_fn v_pull;
    
} STrack;

/**
 * Initializes the object
 * @param obj STrack object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param pull_fn virtual pull function to pull audio
 * @param opt_spec specification to be used, NULL for system spec
 * @return obj casted as STrack
 */
STrack *STrack_init(oobj obj, oobj parent, oobj opt_parent2, STrack__pull_fn pull_fn, const struct s_spec *opt_spec);


//
// object functions
//


/**
 * @param obj STrack object
 * @return track data specifications like channels
 */
OObj_DECL_GET(STrack, struct s_spec, spec)


/**
 * Calls the vfunc to pull audio data from the track.
 * Also checks for quiet and for parent2.
 * @param obj STrack object
 * @param out_data frames*STrack_spec(obj).channels to fill in data retrieved by this track
 * @param frames number of samples*channels
 * @return true if track has ended and is endable
 */
bool STrack_pull(oobj obj, float *restrict out_data, oi64 frames);


/**
 * @param obj STrack object
 * @return if true, the track may be deleted on end.
 *         On false, STrack_pull always returns false (default)
 */
OObj_DECL_GETSET(STrack, bool, endable)


/**
 * @param obj STrack object
 * @return if true, the track returns silence 
 *         on STrack_pull without calling the vfunc.
 *         (default is false)
 */
OObj_DECL_GETSET(STrack, bool, quiet)



#endif //S_STRACK_H
