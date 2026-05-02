#ifndef M_MMAT_PALETTE_H
#define M_MMAT_PALETTE_H

/**
 * @file MMat_palette.h
 *
 * Object extension
 * 
 * MMat_ namespace
 *
 * Palette and indexed MMat's
 *
 * @sa MMat_ex to import all MMat object extensions
 */

#include "MMat.h"

//
// Indexed palettes
//

void MMat_palette_apply_into(oobj obj, oobj into, oobj palette, int palette_row);
struct oobj_opt MMat_palette_apply_try(oobj obj, oobj palette, int palette_row);
/**
 * Applies a palette to an indexed MMat.
 * for each pixel of obj, the index is used to replace it with the color value of that column in the palette.
 * @param obj MMat object (indexed, needs M_FORMAT__PRIMITIVE_U8 or M_FORMAT__PRIMITIVE_UR8)
 * @param palette MMat object. Can have any possible format in contrast to obj
 * @param palette_row used row to get the data from, mostly 0
 * @return MMat allocated on obj, size of obj, format of palette
 * @warning asserts obj to have either M_FORMAT__PRIMITIVE_U8 or M_FORMAT__PRIMITIVE_UR8
 * @note format M_FORMAT__PRIMITIVE_U8 is silently casted internally to M_FORMAT__PRIMITIVE_UR8.
 *       Uses modulo on the index with the palette cols to avoid overflows.
 */
o_inline MMat *MMat_palette_apply(oobj obj, oobj palette, int palette_row)
{
    MMat *self = MMat_palette_apply_try(obj, palette, palette_row).o;
    o_assume(self, "MMat allocation failed");
    return self;
}

void MMat_palette_nearest_into(oobj obj, oobj into, oobj palette, int palette_row);
struct oobj_opt MMat_palette_nearest_try(oobj obj, oobj palette, int palette_row);
/**
 * Searches for each pixel of MMat obj the associated palette color via euclidean distance.
 * Generates and returns it as an indexed image in M_FORMAT__PRIMITIVE_UR8
 * @param obj MMat object (dense)
 * @param palette MMat object. (Needs the same format as obj)
 * @param palette_row used row to get the data from, mostly 0
 * @return MMat allocated on obj, size of obj, as indexed image in M_FORMAT__PRIMITIVE_UR8 format
 * @sa MMat_palette_quantize to generate the palette from obj
 */
o_inline MMat *MMat_palette_nearest(oobj obj, oobj palette, int palette_row)
{
    MMat *self = MMat_palette_nearest_try(obj, palette, palette_row).o;
    o_assume(self, "MMat allocation failed");
    return self;
}


/**
 * Searches for the most transparent palette color and swaps front.
 * That first color will be set to full transparent, all remaining to full opaque.
 * @param obj MMat object of the palette
 * @param palette_row row to edit, mostly 0
 * @note asserts for at least 4 channels
 */
void MMat_palette_as_fixed_transparency(oobj obj, int palette_row);

/**
 * Sorts the colors int the palette according to alpha, transparent front, opaque last
 * @param obj MMat object of the palette
 * @note asserts for at least 4 channels
 */
void MMat_palette_sort_alpha(oobj obj);

/**
 * Sorts the colors int the palette according to value, low front, high last
 * @param obj MMat object of the palette
 * @note asserts for at least 4 channels
 */
void MMat_palette_sort_hue(oobj obj);

/**
 * Sorts the colors int the palette according to saturation, low front, high last
 * @param obj MMat object of the palette
 * @note asserts for at least 4 channels
 */
void MMat_palette_sort_sat(oobj obj);
/**
 * Sorts the colors int the palette according to value, low front, high last
 * @param obj MMat object of the palette
 * @note asserts for at least 4 channels
 */
void MMat_palette_sort_val(oobj obj);

/**
 * Sorts the colors int the palette.
 * First sat, then value, last hue.
 * @param obj MMat object of the palette
 * @param palette_row row to edit, mostly 0
 * @param splits_vh val, hue, <=1 for default
 * @note asserts for at least 3 channels
 */
void MMat_palette_sort_hsv(oobj obj, int palette_row, m_ivec2 splits_vh);

/**
 * Sorts the colors int the palette.
 * First alpha, then sat, then value, last hue.
 * @param obj MMat object of the palette
 * @param palette_row row to edit, mostly 0
 * @param splits_svh sat, val, hue, <=1 for default
 * @note asserts for at least 4 channels
 */
void MMat_palette_sort_hsva(oobj obj, int palette_row, m_ivec3 splits_svh);




// dont forget to o_del
struct oobj_opt MMat_palette_quantize_into_task(oobj obj, oobj into, double econd_avg_error_change, int econd_max_runs);
struct oobj_opt MMat_palette_quantize_try(oobj obj, int num);
/**
 * Quantizes the rgb colorspace of the image obj via nearest neighbor euclidean distance.
 * So produces num main colors representing the image
 * @param obj MMat object
 * @param num number of colors to quantize (cols)
 * @return MMat allocated on obj, size=(num, 1)
 */
o_inline MMat *MMat_palette_quantize(oobj obj, int num)
{
    MMat *self = MMat_palette_quantize_try(obj, num).o;
    o_assume(self, "MMat allocation failed");
    return self;
}


#endif //M_MMAT_PALETTE_H
