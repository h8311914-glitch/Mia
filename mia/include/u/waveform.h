#ifndef U_WAVEFORM_H
#define U_WAVEFORM_H

/**
 * @file gradient.h
 *
 * draws an audio waveform onto an RTex
 */

#include "o/common.h"
#include "m/types/flt.h"
#include "r/box.h"


/**
 * Creates an OArray of struct r_box's for each column
 * @param parent to allocate on
 * @param audio_data audio stream start
 * @param channel channel to use
 * @param channels_num number of channels
 * @param frames number of audio frames to render
 * @param rect xywh rectangle to create the points into (dst RTex full rect, for example)
 * @param thickness width of each rendered col, normally just "1"
 * @param init to copy the boxes from
 * @return OArray of struct r_box
 */
oobj u_waveform_boxes(oobj parent, const float *audio_data, int channel, int channels_num, oi64 frames,
                      vec4 rect, float thickness, struct r_box init);


/**
 * Renders the waveform as a colored line onto the texture
 * @param tex RTex object to render onto
 * @param audio_data audio stream start
 * @param channel channel to use
 * @param frames number of audio frames to render
 * @param ticks number of audio ticks to sample
 * @param rect xywh rectangle to create the points into (dst RTex full rect, for example)
 *             use vec4_(-1) (w||h <= 0) to use the full RTex size
 * @param thickness width of each rendered col, normally just "1"
 * @param color for the line
 * @return number of sampled points
 */
void u_waveform_render(oobj tex, const float *audio_data, int channel, int channels_num, oi64 frames,
                       vec4 rect, float thickness, vec4 color);



#endif //U_WAVEFORM_H
