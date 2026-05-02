#ifndef O_IMG_H
#define O_IMG_H

/**
 * @file img.h
 *
 * Load and write images in ou8 [r, rg, rgb, rgba] format (png and jpg).
 * Based on SDL_image
 */

#include "o/common.h"

enum o_img_channels {
    o_img_KEEP = 0,
    o_img_R = 1,
    o_img_RG = 2,
    o_img_RGB = 3,
    o_img_RGBA = 4,
    o_img_ENUM_MAX = 5
};

struct o_img {
    // row major order, beginning at bottom left
    obyte *data;
    int cols, rows;
    enum o_img_channels channels;
    
    // may be NULL
    oobj parent;
};

/**
 * Creates empty memory for an image
 * @param parent OObj to allocate the resource
 * @param cols columns to allocate
 * @param rows to allocate
 * @param channels to allocate (asserts > 0)
 * @return a new o_img struct, .data owned by obj, or {0} if failed
 * @note data is row major and begins left top 
 */
struct o_img o_img_new(oobj parent, int cols, int rows, enum o_img_channels channels);

/**
 * Creates cleared to zero memory for an image
 * @param parent OObj to allocate the resource
 * @param cols columns to allocate
 * @param rows to allocate
 * @param channels to allocate (asserts > 0)
 * @return a new o_img struct, .data owned by obj, or {0} if failed
 * @note data is row major and begins left top 
 */
struct o_img o_img_new0(oobj parent, int cols, int rows, enum o_img_channels channels);

/**
 * Clones an existing image
 * @param parent OObj to allocate the resource
 * @param to_clone image to clone
 * @return a new o_img struct, .data owned by obj, or {0} if failed
 * @note data is row major and begins left top 
 */
struct o_img o_img_new_clone(oobj parent, struct o_img to_clone);

/**
 * Creates an image from an sdl surface
 * @param parent OObj to allocate the resource
 * @param sdl_surface SDL_Surface* to clone from
 * @param channels 0 / o_img_KEEP try to keep from the sdl surface.
 *                 1 / o_img_R    loads as RGB and returns just R.
 *                 2 / o_img_RG   loads as RGB and returns just RG.
 *                 3 / o_img_RGB  tries to load as RGB directly.
 *                 4 / o_img_RGBA tries to load as RGBA directly. Alpha will be 255 if not present.                
 * @return a new o_img struct, .data owned by obj, or {0} if failed
 * @note data is row major and begins left top
 */
struct o_img o_img_new_sdl_surface(oobj parent, void *sdl_surface, enum o_img_channels channels);

/**
 * Creates an indexed image from an sdl surface.
 * The returned image is always of type o_img_R and contains index data, working on the returned by reference palette.
 * @param parent OObj to allocate the resources
 * @param out_palette image of indexed colors, rows==1, cols<=255, o_img_RGBA ({0} if failed) (safe to pass NULL).
 *                    Does not support loading transparency, .a == 255 (except for index 0, see below)
 * @param fixed_transparency if true, palette index 0 will be set to transparent
 * @param sdl_surface SDL_Surface* to clone from
 * @return a new o_img struct, .data owned by obj, or {0} if failed
 */
struct o_img o_img_indexed_new_sdl_surface(oobj parent, struct o_img *out_palette, bool fixed_transparency, void *sdl_surface);

/**
 * Loads an image file as rgba 8bit into memory.
 * The first pixel is at bottom left in row_major.
 * Supports at least .jpg and .png
 * @param parent OObj to allocate the resource
 * @param file image file (png, jpg) (route path)
 * @param channels 0 / o_img_KEEP try to keep from the sdl surface.
 *                 1 / o_img_R    loads as RGB and returns just R.
 *                 2 / o_img_RG   loads as RGB and returns just RG.
 *                 3 / o_img_RGB  tries to load as RGB directly.
 *                 4 / o_img_RGBA tries to load as RGBA directly. Alpha will be 255 if not present. 
 * @return a new o_img struct, .data owned by obj, or {0} if failed
 * @note data is row major and begins left top 
 */
struct o_img o_img_new_file(oobj parent, const char *file, enum o_img_channels channels);

/**
 * Loads an image file as rgba 8bit into memory.
 * The first pixel is at bottom left in row_major.
 * The returned image is always of type o_img_R and contains index data, working on the returned by reference palette.
 * Needs an indexed png (or smth like that)
 * @param parent OObj to allocate the resource
 * @param out_palette image of indexed colors, rows==1, cols<=255, o_img_RGBA ({0} if failed) (safe to pass NULL).
 *                   Does not support loading transparency, .a == 255 (except for index 0, see below)
 * @param fixed_transparency if true, palette index 0 will be set to transparent
 * @param file image file (indexed png) (route path)
 * @return a new o_img struct, .data owned by obj, or {0} if failed
 * @note data is row major and begins left top.
 */
struct o_img o_img_indexed_new_file(oobj parent, struct o_img *out_palette, bool fixed_transparency, const char *file);

/**
 * moves the image.data into another object
 * @param self to be moved
 * @param into the new object to move the list into
 * @note just calls o_move_res(obj, into, self.data)
 */
void o_img_move(struct o_img *self, oobj into);

/**
 * frees the image.data
 * @param self to free, will be cleared to 0
 * @note just calls o_free(obj, self.data) if parent is != NULL.
 */
void o_img_free(struct o_img *self);


//
// struct functions
//


/**
 * @return cols * rows
 */
o_inline osize o_img_num(struct o_img self)
{
    return (osize) self.cols * (osize) self.rows;
}

/**
 * @return size in bytes needed to hold the image data
 */
o_inline osize o_img_data_size(struct o_img self)
{
    return o_img_num(self) * self.channels;
}


/**
 * Creates an SDL surface from the image.
 * o_img_R     will be an RGB surface with R00.
 * o_img_RG    will be an RGB surface with RG0.
 * o_img_RGB   will be an RGB surface.
 * o_img_RGBA  will be an RGBA surface.
 * @param self to create the surface from
 * @note call SDL_FreeSurface on the returned surface.
 * @warning The returned surface may use self.data as reference, not full copy! (in case for RGB and RGBA)
 */
void *o_img_as_sdl_surface(struct o_img self);


/**
 * Creates an SDL surface from the indexed image + palette.
 * @param self index image of type o_img_R
 * @param palette colors image of type o_img_RGBA. May only support writing RGB values.
 * @note call SDL_FreeSurface on the returned surface.
 *       only supports a single full transparent color which is fixed to palette index 0.
 * @warning The returned surface may use self.data as reference, not full copy!
 */
void *o_img_indexed_as_sdl_surface(struct o_img self, struct o_img palette);



/**
 * Writes an image into a jpg or png file
 * @param self the image to write (rgba 8 bit)
 * @param file to write into (route path)
 * @return false if failed
 * @note data is row major and begins left top 
 */
bool o_img_write_file(struct o_img self, const char *file);


/**
 * Writes an indexed image into a png file
 * @param self index image of type o_img_R
 * @param palette colors image of type o_img_RGBA. May only support writing RGB values.
 * @param file to write into (route path)
 * @return false if failed
 * @note data is row major and begins left top.
 */
bool o_img_indexed_write_file(struct o_img self, struct o_img palette, const char *file);

/**
 * @param self the image to get a pixel from
 * @param idx of a pixel
 * @return the pointer to the pixel at the given pos (starting at r (gba))
 * @note asserts in bounds
 *       data is row major and begins left top 
 */
o_inline obyte *o_img_at_idx(struct o_img self, int idx)
{
    assert(self.data && 0<=idx && idx<self.cols * self.rows && "idx out of o_img bounds");
    return &self.data[idx*self.channels];
}

/**
 * @param self the image to get a pixel from
 * @param c the column, 0 is left
 * @param r the row, 0 is bottom
 * @return the pointer to the pixel at the given pos (starting at r (gba))
 * @note asserts in bounds
 *       data is row major and begins left top 
 */
o_inline obyte *o_img_at(struct o_img self, int c, int r)
{
    assert(self.data && 0<=c && c<self.cols && 0<=r && r < self.rows && "idx out of o_img bounds");
    return o_img_at_idx(self, r * self.cols + c);
}

/**
 * UNCHECKED RAW MACRO MODE
 * @param self struct o_img
 * @param idx of a pixel
 * @return the pointer to the pixel at the given pos (starting at r (gba))
 */
#define o_img_at_idx_raw(self, idx) (&(self).data[(idx) * (self).channels])

/**
 * UNCHECKED RAW MACRO MODE
 * @param self struct o_img
 * @param c the column, 0 is left
 * @param r the row, 0 is top
 * @return the pointer to the pixel at the given pos (starting at r (gba))
 */
#define o_img_at_raw(self, c, r) (&(self).data[((r)*(self).cols + (c)) * (self).channels])


#endif //O_IMG_H
