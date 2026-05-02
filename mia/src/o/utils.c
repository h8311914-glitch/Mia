#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL.h>
#else
#  include <SDL3/SDL_misc.h>
#  include <SDL3/SDL_clipboard.h>
#endif

#include "o/utils.h"
#include "o/str.h"


bool o_utils_open_url(const char *url)
{
#ifdef MIA_OPTION_SDL2
    return false;
#else
    return SDL_OpenURL(url) == 0;
#endif
}


bool o_utils_clipboard_system_available(void)
{
#ifdef MIA_PLATFORM_EMSCRIPTEN
    return false;
#endif
    return true;
}

bool o_utils_clipboard_available(void)
{
    return SDL_HasClipboardText();
}

char *o_utils_clipboard_get(oobj parent)
{
    if(!o_utils_clipboard_available()) {
        return NULL;
    }
    char *sdl_clipboard = SDL_GetClipboardText();
    if(!sdl_clipboard) {
        return NULL;
    }
    char *paste = o_str_clone(parent, sdl_clipboard);
    SDL_free(sdl_clipboard);
    return paste;
}

void o_utils_clipboard_set(const char *set)
{
    // better not setting NULL
    set = o_or(set, "");
    SDL_SetClipboardText(set);
}
