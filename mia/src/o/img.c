#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_image.h>
#  define SDL_DestroySurface SDL_FreeSurface
#else
#  include <SDL3_image/SDL_image.h>
#endif

#include "o/img.h"
#include "o/OObj.h"
#include "o/OObjRoot.h"
#include "o/file.h"


#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


struct o_img o_img_new(oobj parent, int cols, int rows, enum o_img_channels channels)
{
    assert(channels>0 && channels<o_img_ENUM_MAX);
    struct o_img self = {
        NULL,
        cols, rows,
        channels,
        parent
    };
    self.data = o_new_try(parent, obyte, o_img_data_size(self));
    return self;
}

struct o_img o_img_new0(oobj parent, int cols, int rows, enum o_img_channels channels)
{
    struct o_img self = o_img_new(parent, cols, rows, channels);
    o_clear(self.data, 1, o_img_data_size(self));
    return self;
}

struct o_img o_img_new_clone(oobj parent, struct o_img to_clone)
{
    assert(to_clone.channels>0 && to_clone.channels<o_img_ENUM_MAX);
    struct o_img self = o_img_new(parent, to_clone.cols, to_clone.rows, to_clone.channels);
    if (self.data) {
        o_memcpy(self.data, to_clone.data, 1, o_img_data_size(self));
    }
    return self;
}

struct o_img o_img_new_sdl_surface(oobj parent, void *sdl_surface, enum o_img_channels channels)
{
    struct o_img self = {0};
    SDL_Surface *img = (SDL_Surface *) sdl_surface;

#ifdef MIA_OPTION_SDL2
    SDL_Surface *tmp = NULL;
    SDL_PixelFormat *tmp_f = NULL;

    SDL_PixelFormat *sf = img->format;

    // in case of keep, check if sf is either RGB or RGBA like.
    // R or RG are not valid in SDL_Surface from simply loading.
    // "SDL_PIXELFORMAT_INDEX8" may misbehave as its designed for indexed palette lookups and not handled here
    if (channels <= 0 || channels >= o_img_ENUM_MAX) {
        if (sf->BitsPerPixel == 24) {
            channels = o_img_RGB;
        } else if (sf->BitsPerPixel == 32) {
            channels = o_img_RGBA;
        } else {
            o_log_info("got an invalid format, using o_img_RGBA instead");
            channels = o_img_RGBA;
        }
    }

    SDL_PixelFormat *df;
    int stride;
    if (channels == o_img_RGBA) {
        tmp_f = df = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
        stride = 4;
    } else {
        tmp_f = df = SDL_AllocFormat(SDL_PIXELFORMAT_RGB24);
        stride = 3;
    }

    // check if the format equals the needed
    //    and convert it otherwise
    if (sf->BitsPerPixel != df->BitsPerPixel
        || sf->Rmask != df->Rmask
        || sf->Gmask != df->Gmask
        || sf->Bmask != df->Bmask
        || sf->Amask != df->Amask) {
        tmp = img = SDL_ConvertSurface(img, df, 0);
    }

    if (img->pitch != img->w * stride) {
        o_log_warn_s(__func__, "failed: pitch must be == cols*stride");
        goto CLEAN_UP;
    }

    SDL_PixelFormat *f = img->format;

    if (f->BitsPerPixel != stride * 8) {
        o_log_warn_s(__func__, "failed to load and cast the image, stride*8bpp needed");
        goto CLEAN_UP;
    }
    if (channels == o_img_RGBA && f->Amask == 0) {
        o_log_warn_s(__func__, "failed to load and cast the image, alpha needed");
        goto CLEAN_UP;
    }

    if (channels == o_img_RGB || channels == o_img_RGBA) {
        // just clone the data...
        struct o_img clone = {
            img->pixels,
            img->w, img->h,
            channels,
            parent
        };
        self = o_img_new_clone(parent, clone);
    } else {
        assert(channels == o_img_R || channels == o_img_RG);
        // copy red (green) channel
        self = o_img_new(parent, img->w, img->h, channels);
        for (int r = 0; r < self.rows; r++) {
            for (int c = 0; c < self.cols; c++) {
                obyte *restrict src = (obyte *) img->pixels + r * img->pitch + c * stride;
                obyte *restrict dst = o_img_at_raw(self, c, r);
                o_memcpy(dst, src, channels, 1);
            }
        }
    }

CLEAN_UP:
    if (tmp) {
        SDL_DestroySurface(tmp);
    }
    if (tmp_f) {
        SDL_FreeFormat(tmp_f);
    }
    return self;

#else
    // SDL3
    SDL_Surface *tmp = NULL;

    SDL_PixelFormat sf = img->format;


    // in case of keep, check if sf is either RGB or RGBA like.
    // R or RG are not valid in SDL_Surface from simply loading.
    // "SDL_PIXELFORMAT_INDEX8" may misbehave as its designed for indexed palette lookups and not handled here
    if (channels <= 0 || channels >= o_img_ENUM_MAX) {
        if(SDL_ISPIXELFORMAT_ALPHA(sf)) {
            channels = o_img_RGBA;
        } else {
            channels = o_img_RGB;
        }
    }


    SDL_PixelFormat df;
    int stride;
    if (channels == o_img_RGBA) {
        df = SDL_PIXELFORMAT_RGBA32;
        stride = 4;
    } else {
        df = SDL_PIXELFORMAT_RGB24;
        stride = 3;
    }

    // check if the format equals the needed
    //    and convert it otherwise
    if (sf != df) {
        tmp = img = SDL_ConvertSurface(img, df);
    }

    if (img->pitch != img->w * stride) {
        o_log_warn_s(__func__, "failed: pitch must be == cols*stride");
        goto CLEAN_UP;
    }

    SDL_PixelFormat f = img->format;

    if (f != df) {
        o_log_warn_s(__func__, "failed to load and cast the image, stride*8bpp and alpha needed");
        goto CLEAN_UP;
    }

    if (channels == o_img_RGB || channels == o_img_RGBA){
        struct o_img clone = {
            img->pixels,
            img->w, img->h,
            channels,
            parent
        };
        self = o_img_new_clone(parent, clone);
    } else {
        assert(channels == o_img_R || channels == o_img_RG);
        // copy red (green) channel
        self = o_img_new(parent, img->w, img->h, channels);
        for (int r = 0; r < self.rows; r++) {
            for (int c = 0; c < self.cols; c++) {
                obyte *restrict src = (obyte *) img->pixels + r * img->pitch + c * stride;
                obyte *restrict dst = o_img_at_raw(self, c, r);
                o_memcpy(dst, src, channels, 1);
            }
        }
    }

    CLEAN_UP:
    if (tmp) {
        SDL_DestroySurface(tmp);
    }
    return self;

#endif
}

struct o_img o_img_indexed_new_sdl_surface(oobj parent, struct o_img *out_palette, bool fixed_transparency, void *sdl_surface)
{
    struct o_img self = {0};
    SDL_Surface *img = (SDL_Surface *) sdl_surface;

#ifdef MIA_OPTION_SDL2
    SDL_Surface *tmp = NULL;
    SDL_PixelFormat *sf = img->format;
    SDL_PixelFormat *df = SDL_AllocFormat(SDL_PIXELFORMAT_INDEX8);

    // check if the format equals the needed
    //    and convert it otherwise
    if (sf->BitsPerPixel != df->BitsPerPixel
        || sf->Rmask != df->Rmask
        || sf->Gmask != df->Gmask
        || sf->Bmask != df->Bmask
        || sf->Amask != df->Amask) {
        tmp = img = SDL_ConvertSurface(img, df, 0);
        }

    if (img->pitch != img->w) {
        o_log_warn_s(__func__, "failed: pitch must be == cols*1");
        goto CLEAN_UP;
    }

    SDL_PixelFormat *f = img->format;
    if (f->BitsPerPixel != 8) {
        o_log_warn_s(__func__, "failed to load and cast the image, 8bpp needed");
        goto CLEAN_UP;
    }

    SDL_Palette *palette = f->palette;
    if (!palette || palette->ncolors <= 0) {
        o_log_warn_s(__func__, "failed to get the image palette");
        goto CLEAN_UP;
    }

    struct o_img clone = {
        img->pixels,
        img->w, img->h,
        o_img_R,
        parent
    };
    self = o_img_new_clone(parent, clone);

    if (out_palette) {
        int num = o_min(palette->ncolors, 255);
        *out_palette = o_img_new(parent, num, 1, o_img_RGBA);
        for (int i=0; i<num; i++) {
            obyte *restrict dst = o_img_at_raw(*out_palette, i, 0);
            SDL_Color *restrict src = &palette->colors[i];
            dst[0] = src->r;
            dst[1] = src->g;
            dst[2] = src->b;
            dst[3] = 255;
        }

        if (fixed_transparency) {
            obyte *restrict dst = o_img_at_raw(*out_palette, 0, 0);
            dst[3] = 0;
        }
    }

    CLEAN_UP:
    if (tmp) {
        SDL_DestroySurface(tmp);
    }
    SDL_FreeFormat(df);
    return self;

#else
    // SDL3
    SDL_Surface *tmp = NULL;

    SDL_PixelFormat sf = img->format;
    SDL_PixelFormat df = SDL_PIXELFORMAT_INDEX8;

    // check if the format equals the needed
    //    and convert it otherwise
    if (sf != df) {
        tmp = img = SDL_ConvertSurface(img, df);
    }

    if (img->pitch != img->w) {
        o_log_warn_s(__func__, "failed: pitch must be == cols*1");
        goto CLEAN_UP;
    }

    SDL_PixelFormat f = img->format;
    if (f != df) {
        o_log_warn_s(__func__, "failed to load and cast the image, indexed 8bpp needed");
        goto CLEAN_UP;
    }

    SDL_Palette *palette = SDL_GetSurfacePalette(img);
    if (!palette || palette->ncolors <= 0) {
        o_log_warn_s(__func__, "failed to get the image palette");
        goto CLEAN_UP;
    }

    struct o_img clone = {
        img->pixels,
        img->w, img->h,
        o_img_R,
        parent
    };
    self = o_img_new_clone(parent, clone);

    if (out_palette) {
        int num = o_min(palette->ncolors, 255);
        *out_palette = o_img_new(parent, num, 1, o_img_RGBA);
        for (int i=0; i<num; i++) {
            obyte *restrict dst = o_img_at_raw(*out_palette, i, 0);
            SDL_Color *restrict src = &palette->colors[i];
            dst[0] = src->r;
            dst[1] = src->g;
            dst[2] = src->b;
            dst[3] = 255;
        }

        if (fixed_transparency) {
            obyte *restrict dst = o_img_at_raw(*out_palette, 0, 0);
            dst[3] = 0;
        }
    }

    CLEAN_UP:
    if (tmp) {
        SDL_DestroySurface(tmp);
    }
    return self;

#endif
}

struct o_img o_img_new_file(oobj parent, const char *file, enum o_img_channels channels)
{
    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    struct o_img self = {0};
    SDL_Surface *img = IMG_Load(routed);
    if (!img || img->w <= 0 || img->h <= 0) {
        o_log_warn_s(__func__, "failed to load the image: %s (%s)", file, SDL_GetError());
        return self;
    }

    self = o_img_new_sdl_surface(parent, img, channels);
    if (!self.data) {
        o_log_warn_s(__func__, "image casting failed for file: %s", file);
    }
    SDL_DestroySurface(img);
    return self;
}

struct o_img o_img_indexed_new_file(oobj parent, struct o_img *out_palette, bool fixed_transparency, const char *file)
{
    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    struct o_img self = {0};
    SDL_Surface *img = IMG_Load(routed);
    if (!img || img->w <= 0 || img->h <= 0) {
        o_log_warn_s(__func__, "failed to load the image: %s (%s)", file, SDL_GetError());
        return self;
    }
    self = o_img_indexed_new_sdl_surface(parent, out_palette, fixed_transparency,img);
    if (!self.data) {
        o_log_warn_s(__func__, "getting indexed image failed for: %s", file);
    }
    SDL_DestroySurface(img);
    return self;
}

void o_img_move(struct o_img *self, oobj into)
{
    o_data_move(self->parent, into, self->data);
    self->parent = into;
}

void o_img_free(struct o_img *self)
{
    if (!self->parent) {
        return;
    }
    o_free(self->parent, self->data);
    *self = (struct o_img){0};
}

//
// struct functions
//

void *o_img_as_sdl_surface(struct o_img self)
{
    if (o_img_data_size(self) <= 0) {
        o_log_error_s(__func__, "failed, data_size <= 0");
        return NULL;
    }

    SDL_Surface *surface;

#ifdef MIA_OPTION_SDL2

    int width = self.cols;
    int height = self.rows;
    int depth, pitch, format;
    if (self.channels == o_img_RGBA) {
        depth = 32;
        pitch = self.cols * 4;
        format = SDL_PIXELFORMAT_RGBA32;
    } else {
        depth = 24;
        pitch = self.cols * 3;
        format = SDL_PIXELFORMAT_RGB24;
    }

    if (self.channels == o_img_RGB || self.channels == o_img_RGBA) {
        surface = SDL_CreateRGBSurfaceWithFormatFrom(self.data, width, height, depth, pitch, format);
    } else {
        int stride = 3;
        surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, depth, format);
        pitch = surface->pitch;
        obyte *pixels = surface->pixels;
        // R is saved as RGB with RRR
        // RG is saves as RGG
        if (self.channels == o_img_R) {
            for (int r = 0; r < self.rows; r++) {
                for (int c = 0; c < self.cols; c++) {
                    obyte *restrict src = o_img_at_raw(self, c, r);
                    obyte *restrict dst = &pixels[r*pitch + c*stride];
                    dst[0] = src[0];
                    dst[1] = dst[2] = 0;
                }
            }
        }
        if (self.channels == o_img_RG) {
            for (int r = 0; r < self.rows; r++) {
                for (int c = 0; c < self.cols; c++) {
                    obyte *restrict src = o_img_at_raw(self, c, r);
                    obyte *restrict dst = &pixels[r*pitch + c*stride];
                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = 0;
                }
            }
        }
    }


#else
    // SDL3
    int width = self.cols;
    int height = self.rows;
    int pitch;
    SDL_PixelFormat format;
    if (self.channels == o_img_RGBA) {
        pitch = self.cols * 4;
        format = SDL_PIXELFORMAT_RGBA32;
    } else {
        pitch = self.cols * 3;
        format = SDL_PIXELFORMAT_RGB24;
    }

    if (self.channels == o_img_RGB || self.channels == o_img_RGBA) {
        surface = SDL_CreateSurfaceFrom(width, height, format, (void *) self.data, pitch);
    } else {
        int stride = 3;
        surface = SDL_CreateSurface(width, height, format);
        pitch = surface->pitch;
        obyte *pixels = surface->pixels;
        // R is saved as RGB with RRR
        // RG is saves as RGG
        if (self.channels == o_img_R) {
            for (int r = 0; r < self.rows; r++) {
                for (int c = 0; c < self.cols; c++) {
                    obyte *restrict src = o_img_at_raw(self, c, r);
                    obyte *restrict dst = &pixels[r*pitch + c*stride];
                    dst[0] = src[0];
                    dst[1] = dst[2] = 0;
                }
            }
        }
        if (self.channels == o_img_RG) {
            for (int r = 0; r < self.rows; r++) {
                for (int c = 0; c < self.cols; c++) {
                    obyte *restrict src = o_img_at_raw(self, c, r);
                    obyte *restrict dst = &pixels[r*pitch + c*stride];
                    dst[0] = src[0];
                    dst[1] = src[1];
                    dst[2] = 0;
                }
            }
        }
    }

#endif
    if (!surface) {
        o_log_error_s(__func__, "failed to create sdl buffer: %s", SDL_GetError());
    }
    return surface;
}


void *o_img_indexed_as_sdl_surface(struct o_img self, struct o_img palette)
{
    if (o_img_data_size(self) <= 0 || o_img_data_size(palette) <= 0) {
        o_log_error_s(__func__, "failed, data_size <= 0");
        return NULL;
    }
    if (self.channels != o_img_R || palette.channels != o_img_RGBA) {
        o_log_error_s(__func__, "failed, channels mismatch");
        return NULL;
    }

    SDL_Surface *surface;

#ifdef MIA_OPTION_SDL2

    surface = SDL_CreateRGBSurfaceWithFormatFrom(self.data, self.cols, self.rows,
            8, self.cols, SDL_PIXELFORMAT_INDEX8);

    if (surface) {
        SDL_Palette *p = SDL_AllocPalette(palette.cols);
        if (!p) {
            SDL_DestroySurface(surface);
            surface = NULL;
        } else {
            SDL_Color *colors = SDL_malloc(sizeof *colors * palette.cols);
            for (int i=0; i<palette.cols; i++) {
                obyte *restrict src = o_img_at(palette, i, 0);
                colors[i].r = src[0];
                colors[i].g = src[1];
                colors[i].b = src[2];
                colors[i].a = src[3];
            }
            SDL_SetPaletteColors(p, colors, 0, palette.cols);
            SDL_free(colors);
            SDL_SetSurfacePalette(surface, p);
            SDL_FreePalette(p);
        }
    }


#else
    // SDL3

    surface = SDL_CreateSurfaceFrom(self.cols, self.rows,
        SDL_PIXELFORMAT_INDEX8, self.data, self.cols);

    if (surface) {
        SDL_Palette *p = SDL_CreatePalette(palette.cols);
        if (!p) {
            SDL_DestroySurface(surface);
            surface = NULL;
        } else {
            SDL_Color *colors = SDL_malloc(sizeof *colors * palette.cols);
            for (int i=0; i<palette.cols; i++) {
                obyte *restrict src = o_img_at(palette, i, 0);
                colors[i].r = src[0];
                colors[i].g = src[1];
                colors[i].b = src[2];
                colors[i].a = src[3];
            }
            SDL_SetPaletteColors(p, colors, 0, palette.cols);
            SDL_free(colors);
            SDL_SetSurfacePalette(surface, p);
            SDL_DestroyPalette(p);
        }
    }

#endif
    if (!surface) {
        o_log_error_s(__func__, "failed to create sdl buffer: %s", SDL_GetError());
    }
    return surface;
}

bool o_img_write_file(struct o_img self, const char *file)
{
    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    bool valid = false;
    SDL_Surface *surface = o_img_as_sdl_surface(self);
    if (!surface) {
        goto CLEAN_UP;
    }

    valid = IMG_SavePNG(surface, routed) == O_SDL_OK;
    o_file_route_save(file);

CLEAN_UP:

    SDL_DestroySurface(surface);

    if (!valid) {
        o_log_error_s(__func__, "failed to write image %s: %s", file, SDL_GetError());
    }
    return valid;
}

bool o_img_indexed_write_file(struct o_img self, struct o_img palette, const char *file)
{
    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    bool valid = false;
    SDL_Surface *surface = o_img_indexed_as_sdl_surface(self, palette);
    if (!surface) {
        goto CLEAN_UP;
    }

    valid = IMG_SavePNG(surface, routed) == O_SDL_OK;
    o_file_route_save(file);

    CLEAN_UP:

        SDL_DestroySurface(surface);

    if (!valid) {
        o_log_error_s(__func__, "failed to write indexed image %s: %s", file, SDL_GetError());
    }
    return valid;
}
