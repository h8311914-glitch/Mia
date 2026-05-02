#ifndef S_SFILTER_H
#define S_SFILTER_H

/**
 * @file SFilter.h
 *
 * Object
 *
 * Abstract filter to be used on a STrackMix or STrackBuf.
 
 * To use an SFilter, simply use that STrack ad its parent
 */


#include "o/OObj.h"
#include "s/common.h"

/** object id */
#define SFilter_ID OObj_ID "SFilter."


/**
 * Virtual function to filter from the track. The default is a noop, returns true if end frame time was reached
 * @param obj SFilter object
 * @param data frames (=samples*channels) to fill in data pulled by the track
 * @patam frames number of frames to filter
 * @param spec audio spec passed in data
 */
typedef void (*SFilter__apply_fn)(oobj obj, float *restrict data, oi64 frames, struct s_spec spec);


typedef struct {
    OObj super;

    // optional OPtr to another parent to autodelete
    oobj opt_parent2_ptr;

    bool active;
    oi64 frame_time;
    oi64 frame_end;

    // vfuncs
    SFilter__apply_fn v_apply;
} SFilter;


/**
 * Initializes the object
 * @param obj SFilter object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @param apply_fn vfunc which is called to apply the filter
 * @return obj casted as SFilter
 */
SFilter *SFilter_init(oobj obj, oobj parent, oobj opt_parent2, SFilter__apply_fn apply_fn);

/**
 * Creates a new SFilter object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this filter gets auto deleted
 * @param apply_fn vfunc which is called to apply the filter
 * @return The new object
 */
o_inline SFilter *SFilter_new(oobj parent, oobj opt_parent2, SFilter__apply_fn apply_fn)
{
    OObj_DECL_IMPL_NEW(SFilter, parent, opt_parent2, apply_fn);
}

//
// object functions
//

/**
 * Calls the vfunc to pull audio data from the track.
 * Checks for parent2 and for active + ended (and may not call the vfunc in such cases)
 * @param obj SFilter object
 * @param data frames (=samples*channels) to fill in data pulled by the track
 * @patam frames number of frames to filter
 * @param spec audio spec passed in data
 */
void SFilter_apply(oobj obj, float *restrict data, oi64 frames, struct s_spec spec);

/**
 * @param obj SFilter object
 * @return only runs the vfunc apply if active (default)
 */
OObj_DECL_GETSET(SFilter, bool, active)

/**
 * @param obj SFilter object
 * @return current filtered frames counted as time and used with frame_end
 */
OObj_DECL_GETSET(SFilter, oi64, frame_time)

/**
 * @param obj SFilter object
 * @return frame where that filter ended, defaults to oi64_MAX
 *         STrackMixer or STrackBuf can use these to delete the tracks if really out of scope (not within cache)
 */
OObj_DECL_GETSET(SFilter, oi64, frame_end)

/**
 * @param obj SFilter object
 * @return true if that filter had ended (self->frame_time >= self->frame_end)
 */
o_inline bool SFilter_ended(oobj obj)
{
    OObj_assert(obj, SFilter);
    SFilter *self = obj;
    return self->frame_time >= self->frame_end;
}

#endif //S_SFILTER_H
