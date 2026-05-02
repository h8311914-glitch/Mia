#ifndef S_WAV_H
#define S_WAV_H

/**
 * @file wav.h
 *
 * load and write .wav files
 */

#include "common.h"

/**
 * Loads a .wav (sound effect) file and resamples it to the used frequency (if neccessary)
 * @param file route path
 * @return OArray of floats for the resulting loaded audio buffer or NULL if failed
 */
struct oobj_opt s_wav_load_array(oobj parent, const char *file, const struct s_spec *opt_spec);

/**
 * Loads a .wav (sound effect) file and resamples it to the used frequency (if neccessary)
 * @param file route path
 * @return SBuf of the resulting loaded audio buffer or NULL if failed
 */
struct oobj_opt s_wav_load_buf(oobj parent, const char *file, const struct s_spec *opt_spec);

/**
 * Writes pcm data into a .wav file
 * @param file to create (.wav ending) (route path)
 * @param data pcm data
 * @param smp ticks (number of floats / channels)
 * @param opt_spec pcm data specification, if NULL: system spec is expected
 * @return true if succeeded
 */
bool s_wav_write(const char *file, const float *data, oi64 smp, const struct s_spec *opt_spec);

#endif //S_WAV_H
