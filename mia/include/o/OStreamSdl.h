#ifndef O_OSTREAMSDL_H
#define O_OSTREAMSDL_H

/**
 * @file OStreamSdl.h
 *
 * Object
 *
 * OStream implementation based on SDL_RWops
 * @sa name.h for functions that open files
 */

#include "OStream.h"

/** object id */
#define OStreamSdl_ID OStream_ID "Sdl."


typedef struct {
    OStream super;

    void *sdl_stream;
} OStreamSdl;


/**
 * Initializes the object
 * @param obj OStreamSdl object
 * @param parent to inherit from
 * @param sdl_stream_sink SDL_RWops | SDL_IOStream object as sink (will be destroyed | closed by this stream)
 * @return obj casted as OStreamSdl
 */
OStreamSdl *OStreamSdl_init(oobj obj, oobj parent, void *sdl_stream_sink);


/**
 * Creates a new OStreamSdl object
 * @param parent to inherit from
 * @param rwops_sink SDL_RWops | SDL_IOStream object as sink (will be destroyed | closed by this stream)
 * @return The new object
 */
o_inline OStreamSdl *OStreamSdl_new(oobj parent, void *sdl_stream_sink)
{
    OObj_DECL_IMPL_NEW(OStreamSdl, parent, sdl_stream_sink);
}

//
// virtual implementations:
//

bool OStreamSdl__v_valid(oobj obj);

osize OStreamSdl__v_size(oobj obj);

osize OStreamSdl__v_seek(oobj obj, osize offset, enum OStream_seek_whence whence);

osize OStreamSdl__v_read_try(oobj obj, void *data, osize element_size, osize num);

osize OStreamSdl__v_write(oobj obj, const void *data, osize element_size, osize num);

bool OStreamSdl__v_close(oobj obj);

#endif //O_OSTREAMSDL_H
