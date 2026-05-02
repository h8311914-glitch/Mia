#ifndef M_MMAT_CHANNEL_H
#define M_MMAT_CHANNEL_H

/**
 * @file MMat_channel.h
 *
 * Object extension
 *
 * MMat_ namespace
 *
 * MMat channel split and merging
 *
 * @sa MMat_ex to import all MMat object extensions
 */

#include "MMat.h"


//
// Channels and merging
//


/**
 * @param obj MMat object.
 * @param channel [0-4) for r, g, b, a
 * @return MMat view on obj as a single channeled matrix
 * @note asserts r_format_channels(MMat_format(obj)) > channel.
 *       Use MMat_owning_make or MMat_clone to get a copy of that channel.
 */
MMat *MMat_channel(oobj obj, int channel);

/**
 * @param obj MMat object.
 * @param out_splits an array of m_format_channel length (oobj rgba[4] for an rgba MMat).
 *                   Each is a MMat view of that channel.
 * @note Use MMat_owning_make or MMat_clone to get a copy of that channel.
 */
o_inline void MMat_channel_split(oobj obj, oobj *out_splits)
{
    ou32 channels = m_format_channels(MMat_format(obj));
    for (int i = 0; i < channels; i++) {
        out_splits[i] = MMat_channel(obj, i);
    }
}

/**
 * @param obj MMat object.
 * @param obj_channel [0-4) to set (for r, g, b, a)
 * @param set MMat object to set the channel from
 * @param set_channel [0-4) to get from (for r, g, b, a)
 * @note asserts r_format_channels(MMat_format(obj)) > obj_channel
 *               r_format_channels(MMat_format(set)) > set_channel
 *               and that format primitive match
 * @warning like memcpy, this is using restrict to speed up, so avoid aliasing
 */
void MMat_channel_set(oobj obj, int obj_channel, oobj set, int set_channel);



void MMat_channel_merge_into(const oobj *channels, oobj into, const float *opt_add);
struct oobj_opt MMat_channel_merge_try(oobj parent, ou32 format, const oobj *channels, const float *opt_add);
/**
 * Combines given channel MMat objects to a single MMat
 * @param parent to inherit from
 * @param format for the new MMat
 * @param channels array of MMat's or NULL which is replaced with 0, length of format channels
 * @param opt_add if not NULL an array of size format channels and each with a value to be added on the full channel
 * @return The merged MMat
 * @note the opt_add can be used to get all values 1 if NULL is passed as that channel.
 *       If a channel has more then 1 internal channels, the first is used (red)
 *       Asserts at least a single channel available, not all NULL
 */
o_inline MMat *MMat_channel_merge(oobj parent, ou32 format, const oobj *channels, const float *opt_add)
{
    MMat *self = MMat_channel_merge_try(parent, format, channels, opt_add).o;
    o_assume(self, "MMat allocation failed");
    return self;
}

/**
 * Combines all 4 channels into a single matrix.
 * If a MMat has more than 1 channel, red is used
 * @param r red
 * @param g green
 * @param b blue
 * @param opt_a alpha or NULL for all 1.0
 * @return MMat allocated on r (using size of r)
 * @note uses format primtive of r
 */
MMat *MMat_channel_merge_rgba(oobj r, oobj g, oobj b, oobj opt_a);



void MMat_channel_swizzle_into(oobj obj, oobj into, const char *swizzle);
struct oobj_opt MMat_channel_swizzle_try(oobj obj, const char *swizzle, ou32 format);
/**
 * Remerges / sorts the channels of the given MMat with the swizzle string
 * @param obj MMat object
 * @param swizzle string of at max 4 chars.
 *                Rules: R for channel[0]
 *                       G for channel[1]
 *                       B for channel[2]
 *                       A for channel[3]
 *                       0 for constant 0.0
 *                       1 for constant 1.0
 *                       case is ignored, so r, g, b, a, are fine to use
 *                Examples: "RGBA" (just a clone)
 *                          "RRRA" (red channel as rgba image)
 *                          "RGB1" (ignoring alpha)
 *                          "BGR"  (like OpenCV)
 *                          "R"    (just red channel)
 *                          "0G01" (only green channel as rgba image)
 * @param format primitive for the resulting MMat, or 0 (M_FORMAT_KEEP) to use it from obj (channels ignored)
 * @return the new swizzled MMat
 */
o_inline MMat *MMat_channel_swizzle(oobj obj, const char *swizzle, ou32 format)
{
    MMat *self = MMat_channel_swizzle_try(obj, swizzle, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}


#endif //M_MMAT_CHANNEL_H
