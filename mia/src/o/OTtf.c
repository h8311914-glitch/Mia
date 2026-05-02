#ifdef MIA_OPTION_TTF

#include "o/OTtf.h"
#include "o/OObj_builder.h"
#include "o/file.h"

#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_ttf.h>
#  define SDL_DestroySurface SDL_FreeSurface
#else
#  include <SDL3_ttf/SDL_ttf.h>
#endif


#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"

//
// public
//

OTtf *OTtf_init(oobj obj, oobj parent, const char *file, float size)
{
    char routed[O_FILE_PATH_MAX];
    o_file_route_resolve(routed, file);

    OTtf *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OTtf_ID);

#ifdef MIA_OPTION_SDL2
    // pt is usually in 1/72 inches, but seems to be 1/96 here, so we get size==ptsize
    self->sdl_ttf = TTF_OpenFontDPI(routed, size, 96, 96);
#else
    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetStringProperty(props, TTF_PROP_FONT_CREATE_FILENAME_STRING, routed);
    SDL_SetFloatProperty(props, TTF_PROP_FONT_CREATE_SIZE_FLOAT, size);
    SDL_SetNumberProperty(props, TTF_PROP_FONT_CREATE_HORIZONTAL_DPI_NUMBER, 96);
    SDL_SetNumberProperty(props, TTF_PROP_FONT_CREATE_VERTICAL_DPI_NUMBER, 96);
    self->sdl_ttf = TTF_OpenFontWithProperties(props);
    SDL_DestroyProperties(props);
#endif

    if (!self->sdl_ttf) {
        o_log_error_s(__func__, "TTF_OpenFontDPI failed: %s", SDL_GetError());
    }

    // v del override
    self->super.v_del = OTtf__v_del;

    return self;
}

//
// virtual implementations:
//

void OTtf__v_del(oobj obj)
{
    OObj_assert(obj, OTtf);
    OTtf *self = obj;
    if (self->sdl_ttf) {
        TTF_CloseFont(self->sdl_ttf);
    }
    OObj__v_del(self);
}



//
// object functions:
//

struct o_img OTtf_draw(oobj obj, const char *text)
{
    OObj_assert(obj, OTtf);
    OTtf *self = obj;
    struct o_img img = {0};
    if (!self->sdl_ttf) {
        return img;
    }
    SDL_Color white = {255, 255, 255, 255};

    SDL_Surface *surf;
#ifdef MIA_OPTION_SDL2
    surf = TTF_RenderUTF8_Solid(self->sdl_ttf, text, white);
#else
    surf = TTF_RenderText_Solid(self->sdl_ttf, text, 0, white);
#endif

    img = o_img_new_sdl_surface(self, surf, o_img_KEEP);
    SDL_DestroySurface(surf);
    return img;
}


#endif // MIA_OPTION_TTF
typedef int avoid_empty_translation_unit;
