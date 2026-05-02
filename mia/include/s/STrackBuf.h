#ifndef S_STRACKBUF_H
#define S_STRACKBUF_H

/**
 * @file STrackBuf.h
 *
 * Object (derives STrack)
 *
 * Holds a reference (OPtr) to an SBuf, which itself contains audio data.
 * That audio stream is played in that track on pull.
 * So works fine for music and sound fx.
 *
 * In module "s" context, "sample" is a single float value
 *                        "frame" count samples * channels and are typically setup to play 48000 frames each second.
 *                        "block" are frames * S_BLOCK_SIZE, some filters and systems use them internally.
 *
 * SFilter can be applied to this STrack by using it as the SFilter's parent
 *
 * @sa STrack.h, STrackMix.h, SBuf.h
 */

#include "s/STrack.h"
#include "s/SFilter.h"


/** object id */
#define STrackBuf_ID STrack_ID "Buf."


typedef struct
{
    STrack super;

    // OPtr to the SBuf
    oobj buf_ptr;

    oi64 frame_time;
    oi64 frame_delay;

    // 0: (default) simply play once
    // 1+: additional loops
    // -1 (<0): inf loops
    int loops;

    // if looped, this is the next start frame relative to SBuf, not audio time (if >0)
    oi64 loop_start_frame_offset;
    // if looping, that is the time to loop back to start; else (last run) runs ahead (if >0)
    oi64 loop_end_frame_offset;
    
    float gain;
    
} STrackBuf;

/**
 * Initializes the object.
 * Use a mixer as parent to let it play.
 * Remember that this object may be deleted by an STrackMix parent on next pull.
 * @param obj STrackBuf object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param buf SBuf object to play (will create an OPtr to it)
 * @param frame_delay relative frame time as sound delay.
 * @return obj casted as STrackBuf
 */
STrackBuf *STrackBuf_init(oobj obj, oobj parent, oobj opt_parent2, oobj buf, oi64 frame_delay);

/**
 * Creates a new STrackBuf object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param buf SBuf object to play (will create an OPtr to it)
 * @param frame_delay relative frame time as sound delay.
 * @return The new object
 */
o_inline STrackBuf *STrackBuf_new(oobj parent, oobj opt_parent2, oobj buf, oi64 frame_delay)
{
    OObj_DECL_IMPL_NEW(STrackBuf, parent, opt_parent2, buf, frame_delay);
}

//
// virtual implementations
//

/**
 * Uses the SBuf (if available) and streams its data.
 * Runs given SFilter's.
 * Amplifies with its gain.
 */
bool STrackBuf__v_pull(oobj obj, float *restrict out_data, oi64 frames);


//
// object functions
//

/**
 * Get the SFilter children allocated on this Track to be applied
 * @param obj STrackBuf object
 * @param opt_out_num if not NULL, returns the list size/num
 * @return A null terminated list of the SFilter children, call o_free(obj, list) to free it manually
 * @note the list is allocated on obj.
 *       Create an SFilter **it=list to traverse through the list
 */
o_inline SFilter **STrackBuf_filter_list(oobj obj, osize *opt_out_num) {
    return (SFilter **) OObj_list(obj, opt_out_num, SFilter);
}

/**
 * @param obj STrackBuf object
 * @return Current counted pulled frames (time)
 */
OObj_DECL_GETSET(STrackBuf, oi64, frame_time)

/**
 * @param obj STrackBuf object
 * @return delay time that is applied on pull with frame_time
 */
OObj_DECL_GETSET(STrackBuf, oi64, frame_delay)

/**
 * @param obj STrackBuf object
 * @return optional looping of the audio stream.
 *         0: (default) simply play once
 *         1+: additional loops
 *         -1 (<0): inf loops
 *         If looping is active, the sound is streamed until it reaches "loop_end_frame_offset" (if>0)
 *             and then jumps to "loop_start_frame_offset", both as sample time relative to SBuf (not audio time)
 *         This loops counter is also used internally to count and counts down.
 *         If looped the last run will ignore "loop_end_frame_offset" and just run to the end
 *              (If you want to have a clipped sound, just edit the SBuf...)
 */
OObj_DECL_GETSET(STrackBuf, int, loops)

/**
 * @param obj STrackBuf object
 * @return optional looping start time in SBuf relative frame time
 * @sa STrackBuf_loops
 */
OObj_DECL_GETSET(STrackBuf, oi64, loop_start_frame_offset)

/**
 * @param obj STrackBuf object
 * @return optional looping end time in SBuf relative frame time
 * @sa STrackBuf_loops
 */
OObj_DECL_GETSET(STrackBuf, oi64, loop_end_frame_offset)


/**
 * @param obj STrackBuf object
 * @return amplification that is applied in pull. (defaults to 1.0)
 */
OObj_DECL_GETSET(STrackBuf, float, gain)




#endif //S_STRACKBUF_H
