#ifndef S_OGG_H
#define S_OGG_H

/**
 * @file wav.h
 *
 * load .ogg files
 */

#include "common.h"

/**
 * Loads a .ogg (music) file and resamples it to the used frequency (if neccessary)
 * @param file route path
 * @return OArray of floats for the resulting loaded audio buffer or NULL if failed
 */
struct oobj_opt s_ogg_load_array(oobj parent, const char *file, const struct s_spec *opt_spec);


/**
 * Loads a .ogg (music) file and resamples it to the used frequency (if neccessary)
 * @param file route path
 * @return SBuf of the resulting loaded audio buffer or NULL if failed
 */
struct oobj_opt s_ogg_load_buf(oobj parent, const char *file, const struct s_spec *opt_spec);


#endif //S_OGG_H
