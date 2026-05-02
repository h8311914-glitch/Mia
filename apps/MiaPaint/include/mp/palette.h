#ifndef MP_PALETTE_H
#define MP_PALETTE_H

/**
 * @file palette.h
 *
 * Implements an AView to be used as color picker palette
 */

#include "o/OObj.h"
#include "m/types/flt.h"

#include "surface.h"

#define MP_PALETTE_DROP_SIZE 16.0f

/**
 * @param parent to inherit from
 * @param surface surface for color changes
 * @return AView that renders and handles the selected color
 */
oobj mp_palette_new(oobj parent, struct mp_surface *surface, const vec4 *colors, int num);

/**
 * @param view AView created from mp_palette_new
 * @return the needed size to setup AView rendering
 */
vec2 mp_palette_needed_size(oobj view);

/**
 * Hides the selected color, cause it may be set from other sources like the 2nd color swap
 * @param view AView created from mp_palette_new
 */
void mp_palette_current_hide(oobj view);

/**
 * @param view AView created from mp_palette_new
 * @param colors array of colors to show in the palette
 * @param num array size
 */
void mp_palette_set(oobj view, const vec4 *colors, int num);

#endif //MP_PALETTE_H
