#ifndef S_STRACKGEN_H
#define S_STRACKGEN_H

/**
 * @file STrackGen.h
 *
 * Object (derives STrack)
 *
 * Generates sounds, like an Oszilator
 *
 * SFilter can be applied to this STrack by using it as the SFilter's parent
 *
 * @sa STrack.h, STrackMix.h, SGen.h
 */

#include "s/STrack.h"
#include "s/SFilter.h"


/** object id */
#define STrackGen_ID STrack_ID "Gen."

enum STrackGen_mode {
    STrackGen_WAVE,
    STrackGen_SAW,
    STrackGen_RAMP,
    STrackGen_BLOCK,
    STrackGen_NOISE,
    STrackGen_ENUM_MAX
};


typedef struct
{
    STrack super;

    enum STrackGen_mode mode;
    
    // loop / second (ignored for mode NOISE)
    float freq;
    
    // current loop pos [0:1]
    float t;
    
    float gain;
    
} STrackGen;

/**
 * Initializes the object.
 * Use a mixer as parent to let it play.
 * @param obj STrackGen object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param mode to generate
 * @param freq loops / second (ignored for mode NOISE)
 * @param opt_spec specification to be used, NULL for system spec
 * @return obj casted as STrackGen
 */
STrackGen *STrackGen_init(oobj obj, oobj parent, oobj opt_parent2, enum STrackGen_mode mode, float freq, const struct s_spec *opt_spec);

/**
 * Creates a new STrackGen object
 * @param parent to inherit from
 * @param opt_parent2 another optional parent, if that got deleted, this track gets auto deleted
 * @param mode to generate
 * @param freq loops / second (ignored for mode NOISE)
 * @param opt_spec specification to be used, NULL for system spec
 * @return The new object
 */
o_inline STrackGen *STrackGen_new(oobj parent, oobj opt_parent2, enum STrackGen_mode mode, float freq, const struct s_spec *opt_spec)
{
    OObj_DECL_IMPL_NEW(STrackGen, parent, opt_parent2, mode, freq, opt_spec);
}

//
// virtual implementations
//

/**
 * Generates a sound based on mode and freq(uency)
 * Runs given SFilter's.
 * Amplifies with its gain.
 * @return false
 */
bool STrackGen__v_pull(oobj obj, float *restrict out_data, oi64 frames);


//
// object functions
//

/**
 * Get the SFilter children allocated on this Gen to be applied
 * @param obj STrackGen object
 * @param opt_out_num if not NULL, returns the list size/num
 * @return A null terminated list of the SFilter children, call o_free(obj, list) to free it manually
 * @note the list is allocated on obj.
 *       Create an SFilter **it=list to traverse through the list
 */
o_inline SFilter **STrackGen_filter_list(oobj obj, osize *opt_out_num) {
    return (SFilter **) OObj_list(obj, opt_out_num, SFilter);
}

/**
 * @param obj STrackGen object
 * @return generating mode to be used
 */
OObj_DECL_GETSET(STrackGen, enum STrackGen_mode, mode)

/**
 * @param obj STrackGen object
 * @return freq(uency) (loops/second) used for generating, ignored for NOISE
 */
OObj_DECL_GETSET(STrackGen, float, freq)

/**
 * @param obj STrackGen object
 * @return Current loop position [0:1], ignored for NOISE
 */
OObj_DECL_GETSET(STrackGen, float, t)

/**
 * @param obj STrackGen object
 * @return amplification that is applied in pull. (defaults to 1.0)
 */
OObj_DECL_GETSET(STrackGen, float, gain)




#endif //S_STRACKGEN_H
