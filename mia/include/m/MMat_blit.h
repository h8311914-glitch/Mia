#ifndef M_MMAT_BLIT_H
#define M_MMAT_BLIT_H

/**
 * @file MMat_blit.h
 *
 * Object extension
 *
 * MMat_ namespace
 *
 * Blitting, blending, resampling, resizing
 *
 * @sa MMat_ex to import all MMat object extensions
 */

#include "MMat.h"


//
// Blit and Blend
//


/**
 * Blits (copies) the MMat img into obj, with the given offset from obj:lt to img:lt.
 * Only uses a subview of img defined by the img_rect
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to copy onto obj
 * @param x, y offsets from left top
 * @param img_rect ltwh rect atlas for img. w|h<=0 for full img
 * @note just copies the values, no blending internally.
 *       format missmatch -> only up to channel 4 (rgba).
 * @warning img and obj must not share the same data root!
 */
void MMat_blit_rect(oobj obj, oobj img, int x, int y, m_ivec4 img_rect);


/**
 * Blits (copies) the MMat img into obj, with the given offset from obj:lt to img:lt.
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to copy onto obj
 * @param x, y offsets from left top
 * @note just copies the values, no blending internally.
 *       format missmatch -> only up to channel 4 (rgba).
 * @warning img and obj must not share the same data root!
 */
o_inline void MMat_blit(oobj obj, oobj img, int x, int y)
{
    MMat_blit_rect(obj, img, x, y, (m_ivec4) {0});
}

/**
 * Blends (using alpha blending) the MMat img into obj, with the given offset from obj:lt to img:lt.
 * Only uses a subview of img defined by the img_rect
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to blend onto obj
 * @param x, y offsets from left top
 * @param img_rect ltwh rect atlas for img. w|h<=0 for full img
 * @note if alpha channel is missing, 0 is used!, as with m_format_cast_to_vec4
 *       Like with RTex, the blend functions resulting alpha value is obj.a + img.a
 *       In contrast to MMat_blit, blending is roughly 2-5x more expensive (for formats vec4 and bvec4)
 * @warning img and obj must not share the same data root!
 */
void MMat_blend_rect(oobj obj, oobj img, int x, int y, m_ivec4 img_rect);


/**
 * Blends (using alpha blending) the MMat img into obj, with the given offset from obj:lt to img:lt.
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to blend onto obj
 * @param x, y offsets from left top
 * @note if alpha channel is missing, 0 is used!, as with m_format_cast_to_vec4
 *       Like with RTex, the blend functions resulting alpha value is obj.a + img.a#
 *       In contrast to MMat_blit, blending is roughly 2-5x more expensive (for formats vec4 and bvec4)
 * @warning img and obj must not share the same data root!
 */
o_inline void MMat_blend(oobj obj, oobj img, int x, int y)
{
    MMat_blend_rect(obj, img, x, y, (m_ivec4) {0});
}



//
// Resample
//

/**
 * Blits (copies) the MMat img into obj.
 * Resamples img, which allows for resizing / scaling.
 * Only uses a subview of img defined by the img_rect (float)
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to copy onto obj
 * @param rect resulting rect for obj. w|h<=0 for full obj
 * @param img_rect ltwh rect atlas for img. w|h<=0 for full img
 * @param linear true: linear sampling
 *               false: nearest
 * @note just copies the values, no blending internally.
 *       format missmatch -> only up to channel 4 (rgba).
 * @warning img and obj must not share the same data root!
 */
void MMat_resample_blit_rect(oobj obj, oobj img, m_ivec4 rect, m_vec4 img_rect, bool linear);

/**
 * Blits (copies) the MMat img into obj.
 * Resamples img, which allows for resizing / scaling.
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to copy onto obj
 * @param rect resulting rect for obj. w|h<=0 for full obj
 * @param linear true: linear sampling
 *               false: nearest
 * @note just copies the values, no blending internally.
 *       format missmatch -> only up to channel 4 (rgba).
 * @warning img and obj must not share the same data root!
 */
o_inline void MMat_resample_blit(oobj obj, oobj img, m_ivec4 rect, bool linear)
{
    MMat_resample_blit_rect(obj, img, rect, (m_vec4) {0}, linear);
}

/**
 * Blends (using alpha blending) the MMat img into.
 * Resamples img, which allows for resizing / scaling.
 * Only uses a subview of img defined by the img_rect (float)
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to copy onto obj
 * @param rect resulting rect for obj. w|h<=0 for full obj
 * @param img_rect ltwh rect atlas for img. w|h<=0 for full img
 * @param linear true: linear sampling (fastest path ig img format is RGBA_F32 and obj either RGBA_U8 or _F32)
 *               false: nearest
 * @note Like with RTex, the blend functions resulting alpha value is obj.a + img.a#
 *       In contrast to MMat_resample_blit, blending is roughly 2-5x more expensive (for formats vec4 and bvec4).
 *       format missmatch -> only up to channel 4 (rgba).
 * @warning img and obj must not share the same data root!
 */
void MMat_resample_blend_rect(oobj obj, oobj img, m_ivec4 rect, m_vec4 img_rect, bool linear);

/**
 * Blends (using alpha blending) the MMat img into.
 * Resamples img, which allows for resizing / scaling.
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to copy onto obj
 * @param rect resulting rect for obj. w|h<=0 for full obj
 * @param linear true: linear sampling
 *               false: nearest
 * @note Like with RTex, the blend functions resulting alpha value is obj.a + img.a#
 *       In contrast to MMat_resample_blit, blending is roughly 2-5x more expensive (for formats vec4 and bvec4)
 *       format missmatch -> only up to channel 4 (rgba).
 * @warning img and obj must not share the same data root!
 */
o_inline void MMat_resample_blend(oobj obj, oobj img, m_ivec4 rect, bool linear)
{
    MMat_resample_blend_rect(obj, img, rect, (m_vec4) {0}, linear);
}


void MMat_resample_down_into(oobj obj, oobj into);
struct oobj_opt MMat_resample_down_try(oobj obj, ou32 format);
/**
 * Resamples linear down to size half'ed.
 * Each 2x2 block is averaged to a single 1x1 block.
 * The resulting size is ceiled so an MMat of size 10x11 will get 5x6
 * @param obj MMat object to read from
 * @param format for the new generated MMat, if 0 (keep) its set to the first not NULL sources MMat
 */
o_inline MMat *MMat_resample_down(oobj obj, ou32 format)
{
    MMat *self = MMat_resample_down_try(obj, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}


void MMat_resample_resize_rect_into(oobj obj, oobj into, m_vec4 rect, bool linear);
struct oobj_opt MMat_resample_resize_rect_try(oobj obj, m_vec4 rect, m_ivec2 size, bool linear, ou32 format);
/**
 * Resamples to resize the given MMat image.
 * Only uses a subview of img defined by the img_rect (float)
 * @param obj MMat object to read from
 * @param rect ltwh rect atlas for obj. w|h<=0 for full img
 * @param size resulting size
 * @param linear true: linear sampling
 *               false: nearest
 * @param format for the new generated MMat, if 0 (keep) its set to the first not NULL sources MMat.
 *               To speed things up, if format is either M_FORMAT_RGBA_U8 or _F32, obj may be cast'ed internally.
 */
o_inline MMat *MMat_resample_resize_rect(oobj obj, m_vec4 rect, m_ivec2 size, bool linear, ou32 format)
{
    MMat *self = MMat_resample_resize_rect_try(obj, rect, size, linear, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}

o_inline struct oobj_opt MMat_resample_resize_try(oobj obj, m_ivec2 size, bool linear, ou32 format)
{
    return MMat_resample_resize_rect_try(obj, (m_vec4) {0}, size, linear, format);
}
/**
 * Resamples to resize the given MMat image.
 * @param obj MMat object to read from
 * @param size resulting size
 * @param linear true: linear sampling
 *              false: nearest
 * @param format for the new generated MMat, if 0 (keep) its set to the first not NULL sources MMat.
 *               To speed things up, if format is either M_FORMAT_RGBA_U8 or _F32, obj may be cast'ed internally.
 */
o_inline MMat *MMat_resample_resize(oobj obj, m_ivec2 size, bool linear, ou32 format)
{
    return MMat_resample_resize_rect(obj, (m_vec4) {0}, size, linear, format);
}



// dont forget to o_del
oobj MMat_resample_resize_smooth_rect_into_task(oobj obj, oobj into, m_vec4 rect);
struct oobj_opt MMat_resample_resize_smooth_rect_try(oobj obj, m_vec4 rect, m_ivec2 size, ou32 format);
/**
 * Resamples to resize the given MMat image.
 * Always resmaples linear.
 * If resampling down and scale is > 2.0, inbetween resample_half mats are created to keep a smooth filtering
 * Only uses a subview of img defined by the img_rect (float).
 * @param obj MMat object to read from
 * @param rect ltwh rect atlas for obj. w|h<=0 for full img
 * @param size resulting size
 * @param format for the new generated MMat, if 0 (keep) its set to the first not NULL sources MMat
 */
o_inline MMat *MMat_resample_resize_smooth_rect(oobj obj, m_vec4 rect, m_ivec2 size, ou32 format)
{
    MMat *self = MMat_resample_resize_smooth_rect_try(obj, rect, size, format).o;
    o_assume(self, "MMat allocation failed");
    return self;
}

o_inline struct oobj_opt MMat_resample_resize_smooth_try(oobj obj, m_ivec2 size, ou32 format)
{
    return MMat_resample_resize_smooth_rect_try(obj, (m_vec4) {0}, size, format);
}
/**
 * Resamples to resize the given MMat image.
 * Always resmaples linear.
 * If resampling down and scale is > 2.0, inbetween resample_half mats are created to keep a smooth filtering
 * @param obj MMat object to read from
 * @param size resulting size
 * @param format for the new generated MMat, if 0 (keep) its set to the first not NULL sources MMat
 */
o_inline MMat *MMat_resample_resize_smooth(oobj obj, m_ivec2 size, ou32 format)
{
    return MMat_resample_resize_smooth_rect(obj, (m_vec4) {0}, size, format);
}


//
// Blend dab (like for a brush stroke)
//

/**
 * Blends a dab mask or image together with the given color to the MMat obj.
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to blend onto obj. If channels<4, its considered as mask using channel R (0)
 * @param x, y offsets from left top
 * @param img_rect ltwh rect atlas for img. w|h<=0 for full img
 * @param color paint color
 * @note A dab is considered as a single point in a paint stroke.
 * @warning img and obj must not share the same data root!
 */
void MMat_blend_dab_rect(oobj obj, oobj img, int x, int y, m_ivec4 img_rect, m_vec4 color);


/**
 * Blends (using alpha blending) the MMat img into obj, with the given offset from obj:lt to img:lt.
 * Clamps out of bounds
 * @param obj MMat object
 * @param img MMat object to blend onto obj. If channels<4, its considered as mask using channel R (0)
 * @param x, y offsets from left top
 * @param color paint color
 * @note A dab is considered as a single point in a paint stroke.
 * @warning img and obj must not share the same data root!
 */
o_inline void MMat_blend_dab(oobj obj, oobj img, int x, int y, m_vec4 color)
{
    MMat_blend_dab_rect(obj, img, x, y, (m_ivec4) {0}, color);
}



#endif //M_MMAT_BLIT_H
