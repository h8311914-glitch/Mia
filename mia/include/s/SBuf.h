#ifndef S_SBUF_H
#define S_SBUF_H

/**
 * @file SBuf.h
 *
 * Object
 *
 * Holds an OArray of streamable audio samples
 */


#include "o/OArray.h"
#include "s/common.h"

/** object id */
#define SBuf_ID OObj_ID "SBuf."



typedef struct {
    OObj super;

    // element_size = sizeof(float) * spec.channels
    OArray *array;

    struct s_spec spec;
} SBuf;


/**
 * Initializes the object
 * @param obj SBuf object
 * @param parent to inherit from
 * @param array_sink OArray with element_size is asserted to be sizeof(float) * spec.channels.
 *                   o_move'd into the new SBuf object
 * @param opt_spec specification for the buffer, NULL if its system spec
 * @return obj casted as SBuf
 */
SBuf *SBuf_init_array(oobj obj, oobj parent, oobj array_sink, const struct s_spec *opt_spec);

/**
 * Creates a new SBuf object
 * @param parent to inherit from
 * @param array_sink OArray with element_size is asserted to be sizeof(float) * spec.channels.
 *                   o_move'd into the new SBuf object
 * @param opt_spec specification for the buffer, NULL if its system spec
 * @return The new object
 */
o_inline SBuf *SBuf_new_array(oobj parent, oobj array_sink, const struct s_spec *opt_spec)
{
    OObj_DECL_IMPL_NEW_SPECIAL(SBuf, array, parent, array_sink, opt_spec);
}

/**
 * Creates a new SBuf object.
 * Internally creates an OArray and calls SBuf_new_array
 * @param parent to inherit from
 * @param data samples data
 * @param smp amount of samples (data floats = smp * spec.channels)
 * @param opt_spec specification for the buffer, NULL if its system spec
 * @return The new object
 */
SBuf *SBuf_new(oobj parent, float *data, oi64 smp, const struct s_spec *opt_spec);

//
// object functions
//


/**
 * @param obj SBuf object
 * @return audio spec for this buffer; element_size = sizeof(float) * spec.channels
 */
OObj_DECL_GET(SBuf, struct s_spec, spec)

/**
 * @param obj SBuf object
 * @return holds the audio data
 */
OObj_DECL_GET(SBuf, OArray *, array)

/**
 * @param buf SBuf object
 * @param spec Audio specification for the returned new SBuf
 * @return A new SBuf allocated on obj, resampled to the given spec
 */
SBuf *SBuf_as_spec(oobj obj, struct s_spec spec);

#endif //S_SBUF_H
