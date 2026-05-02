#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_rwops.h>
#  define SDL_GetIOSize SDL_RWsize
#  define SDL_CloseIO SDL_RWclose
#else
#  include <SDL3/SDL_iostream.h>
#endif


#include "o/OStreamSdl.h"
#include "o/OObj_builder.h"


OStreamSdl *OStreamSdl_init(oobj obj, oobj parent, void *sdl_stream_sink)
{
    OStreamSdl *self = obj;
    o_clear(self, sizeof *self, 1);

    OStream_init(obj, parent,
                 OStreamSdl__v_valid,
                 OStreamSdl__v_size,
                 OStreamSdl__v_seek,
                 OStreamSdl__v_read_try,
                 OStreamSdl__v_write,
                 OStreamSdl__v_close);
    OObj_id_set(self, OStreamSdl_ID);

    // set the sink
    self->sdl_stream = sdl_stream_sink;

    return self;
}


bool OStreamSdl__v_valid(oobj obj)
{
    OObj_assert(obj, OStreamSdl);
    OStreamSdl *self = obj;
    return self->sdl_stream != NULL;
}

osize OStreamSdl__v_size(oobj obj)
{
    OObj_assert(obj, OStreamSdl);
    OStreamSdl *self = obj;
    if (!self->sdl_stream)
        return -1;
    return SDL_GetIOSize(self->sdl_stream);
}

osize OStreamSdl__v_seek(oobj obj, osize offset, enum OStream_seek_whence whence)
{
    OObj_assert(obj, OStreamSdl);
    OStreamSdl *self = obj;
    if (!self->sdl_stream)
        return -1;

#ifdef MIA_OPTION_SDL2
    // should be 0, 1, 2, just in case...
    return SDL_RWseek(self->sdl_stream, offset, (int[]) {RW_SEEK_SET, RW_SEEK_CUR, RW_SEEK_END}[whence]);
#else
    // should be 0, 1, 2, just in case...
    return SDL_SeekIO(self->sdl_stream, offset, (int[]) {SDL_IO_SEEK_SET, SDL_IO_SEEK_CUR, SDL_IO_SEEK_END}[whence]);
#endif
}

osize OStreamSdl__v_read_try(oobj obj, void *out_data, osize element_size, osize num)
{
    OObj_assert(obj, OStreamSdl);
    OStreamSdl *self = obj;
    if (!self->sdl_stream)
        return 0;

#ifdef MIA_OPTION_SDL2
    osize read = (osize) SDL_RWread(self->sdl_stream, out_data, element_size, num);
#else
    osize read = (osize) SDL_ReadIO(self->sdl_stream, out_data, osize_mul(element_size, num));
#endif


    if (read <= 0) {
        OStream_close(self);
    }
    return read;
}

osize OStreamSdl__v_write(oobj obj, const void *data, osize element_size, osize num)
{
    OObj_assert(obj, OStreamSdl);
    OStreamSdl *self = obj;
    if (!self->sdl_stream)
        return 0;

#ifdef MIA_OPTION_SDL2
    osize written = (osize) SDL_RWwrite(self->sdl_stream, data, element_size, num);
#else
    osize written = (osize) SDL_WriteIO(self->sdl_stream, data, osize_mul(element_size, num));
#endif

    if (written < num) {
        OStream_close(self);
    }
    return written;
}

bool OStreamSdl__v_close(oobj obj)
{
    OObj_assert(obj, OStreamSdl);
    OStreamSdl *self = obj;
    if (!self->sdl_stream)
        return true;

    bool valid = SDL_CloseIO(self->sdl_stream) == O_SDL_OK;

    self->sdl_stream = NULL;
    return valid;
}
