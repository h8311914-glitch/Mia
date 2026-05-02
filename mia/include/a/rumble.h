#ifndef A_RUMBLE_H
#define A_RUMBLE_H

/**
 * @file rumble.h
 *
 * Rumble / vibrate on:
 * - Gamepad
 * - Android device
 * - Web if device supports it
 *
 */

#include "o/common.h"

/**
 * @return true if rumbling may be available, false if not for sure
 *         MIA_PLATFORM_EMSCRIPTEN: true (but only may rumble on mobile phones, etc.)
 *         MIA_PLATFORM_ANDROID: true
 *         else: only true if a gamepad is currently connected
 */
bool a_rumble_available(void);

/**
 * @return true if app rumble is currently mute'd (false is default)
 */
bool a_rumble_mute(void);

/**
 * @return true if app rumble is currently mute'd (false is default)
 */
bool a_rumble_mute_set(bool set);

/**
 * @return global intensity [0:inf], applied on a_rumble_ex internally
 * @note may also be >1 to increase intensity if a_rumble_ex was called <1
 */
float a_rumble_intensity(void);

/**
 * @return global intensity [0:inf], applied on a_rumble_ex internally
 * @note may also be >1 to increase intensity if a_rumble_ex was called <1
 */
float a_rumble_intensity_set(float intensity);

/**
 * Rumbles if possible and !a_rumble_mute() (default is un'mute'd):
 * - Gamepad
 * - Android
 * - Web
 * The _ex version splits between left and right motor
 * @param intensity_low_freq, intensity_high_freq scaled by a_rumble_intensity
 *                                                clamped to [0:1]
 *                                                0 to turn off
 * @param time in seconds to rumble
 */
void a_rumble_ex(float intensity_low_freq, float intensity_high_freq, float time);

/**
 * Rumbles if possible and !a_rumble_mute() (default is un'mute'd):
 * - Gamepad
 * - Android
 * - Web
 * @param intensity scaled by a_rumble_intensity, clamped to [0:1], 0 to turn off
 * @param time in seconds to rumble
 * @note just calls a_rumble_ex(intensity, intensity, time);
 */
o_inline void a_rumble(float intensity, float time)
{
    a_rumble_ex(intensity, intensity, time);
}


#endif //A_RUMBLE_H
