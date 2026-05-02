#ifndef R_RTEX_CHANNEL_H
#define R_RTEX_CHANNEL_H

/**
 * @file RTex_channel.h
 *
 * Object extension
 *
 * RTex_ namespace
 *
 * RTex channel split and mix and channel_merge
 *
 * @sa RTex_ex.h to import all RTex object extensions
 */

#include "RTex.h"



/**
 * @param obj RTex object
 * @param rgba_matrix channel mixer, col[0] -> dst red
 * @param into RTex object to render into
 * @return RTex allocated on obj, with channels remixed
 * @note for example:
 *       oobj red = RTex_channels(obj, vec4_(1, 0), vec4_(0), vec4_(0), vec4_(0), R_FORMAT_R_U8);
 *       oobj inv = RTex_channels(obj, vec4_(-1, 0), vec4_(0, -1), vec4_(0, 0, -1), vec4_(0, 0, 0, 1), R_FORMAT_RGBA_U8);
 *       oobj grey = RTex_channels(obj, vec4_(0.33, 0.33, 0.33, 0), ... , vec4_(0, 0, 0, 1), R_FORMAT_RGBA_U8);
 */
void RTex_mixer_into(oobj obj, mat4 rgba_matrix, oobj into);


/**
 * @param obj RTex object
 * @param rgba_matrix channel mixer, col[0] -> dst red
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj, with channels remixed
 * @note for example:
 *       oobj red = RTex_channels(obj, vec4_(1, 0), vec4_(0), vec4_(0), vec4_(0), R_FORMAT_R_U8);
 *       oobj inv = RTex_channels(obj, vec4_(-1, 0), vec4_(0, -1), vec4_(0, 0, -1), vec4_(0, 0, 0, 1), R_FORMAT_RGBA_U8);
 *       oobj grey = RTex_channels(obj, vec4_(0.33, 0.33, 0.33, 0), ... , vec4_(0, 0, 0, 1), R_FORMAT_RGBA_U8);
 */
o_inline RTex *RTex_mixer(oobj obj, mat4 rgba_matrix, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_mixer_into(obj, rgba_matrix, res);
    return res;
}


/**
 * @param obj RTex object
 * @param channels vec4_(0, 1, 2, 3) for a copy, vec4_(2, 2, 2, 3) to copy .b to .rgb but keep .a
 * @param into RTex object to render into
 * @note for example to get .r as a single channel image:
 *       oobj red = RTex_channels(obj, vec4_(0), vec4_(0), R_FORMAT_R_U8);
 *       just calls RTex_mixer
 */
void RTex_channels_into(oobj obj, ivec4 channel, oobj into);


/**
 * @param obj RTex object
 * @param channels vec4_(0, 1, 2, 3) for a copy, vec4_(2, 2, 2, 3) to copy .b to .rgb but keep .a
 * @param format for the resulting RTex, or 0 to keep
 * @return RTex allocated on obj, with channels remixed / colors reorderd
 * @note for example to get .r as a single channel image:
 *       oobj red = RTex_channels(obj, vec4_(0), vec4_(0), R_FORMAT_R_U8);
 *       just calls RTex_mixer
 */
o_inline RTex *RTex_channels(oobj obj, ivec4 channel, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, m_2(RTex_size(obj)), format, format);
    RTex_channels_into(obj, channel, res);
    return res;
}

/**
 * @param obj RTex object
 * @param out_rgba an array of (at least) 4 oobj to set r g b a into
 * @note calls RTex_channels 4 times in a row...
 */
o_inline void RTex_channels_split_into(oobj obj, oobj *out_rgba_into)
{
    for (int i = 0; i < 4; i++) {
        RTex_channels_into(obj, ivec4_(i), out_rgba_into[i]);
    }
}

/**
 * @param obj RTex object
 * @param out_rgba an array of (at least) 4 oobj to set r g b a into
 * @param format for the resulting RTex's, or 0 to keep
 * @note calls RTex_channels 4 times in a row...
 */
o_inline void RTex_channels_split(oobj obj, oobj *out_rgba, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    for (int i = 0; i < 4; i++) {
        out_rgba[i] = RTex_channels(obj, ivec4_(i), format);
    }
}

/**
 * @param r, g, b, a RTex object
 * @param into RTex object to render into
 * @return RTex allocated on r, with channels merged together
 * @note assumes all RTex's channels share the same size. uses .r on each channel to merge
 */
void RTex_channels_merge_into(oobj r, oobj g, oobj b, oobj a, oobj into);

/**
 * @param r, g, b, a RTex object
 * @param format for the resulting RTex, 0 to use from r
 * @return RTex allocated on r, with channels merged together
 * @note assumes all RTex's channels share the same size. uses .r on each channel to merge
 */
o_inline RTex *RTex_channels_merge(oobj r, oobj g, oobj b, oobj a, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(r);
    }
    RTex *res = RTex_new_ex(r, NULL, m_2(RTex_size(r)), format, format);
    RTex_channels_merge_into(r, g, b, a, res);
    return res;
}


#endif //R_RTEX_CHANNEL_H
