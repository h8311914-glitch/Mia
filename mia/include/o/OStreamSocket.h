#ifdef MIA_OPTION_SOCKET
#ifndef O_OSTREAMSOCKET_H
#define O_OSTREAMSOCKET_H

/**
 * @file OStreamSocket.h
 *
 * Object
 *
 * OStream implementation based ob SDL_net TCPsocket
 * @sa OSocketserver.h (server) and socket.h (client)
 */

#include "OStream.h"

/** object id */
#define OStreamSocket_ID OStream_ID "Socket."


typedef struct {
    OStream super;

    oi32 timeout;
    void *sdl_socket;

#ifdef MIA_OPTION_SDL2
    void *sdl_socket_set;
#endif
} OStreamSocket;


/**
 * Initializes the object
 * @param obj OStreamSocket object
 * @param parent to inherit from
 * @param sdl_socket_sink SDL_net SDLNet_StreamSocket object as sink (will be destroyed | closed by this stream)
 * @return obj casted as OStreamSocket
 */
OStreamSocket *OStreamSocket_init(oobj obj, oobj parent, void *sdl_socket_sink);


/**
 * Creates a new OStreamSocket object
 * @param parent to inherit from
 * @param sdl_socket_sink SDL_net SDLNet_StreamSocket object as sink (will be destroyed | closed by this stream)
 * @return The new object
 */
o_inline OStreamSocket *OStreamSocket_new(oobj parent, void *sdl_socket_sink)
{
    OObj_DECL_IMPL_NEW(OStreamSocket, parent, sdl_socket_sink);
}

//
// virtual implementations:
//

bool OStreamSocket__v_valid(oobj obj);

osize OStreamSocket__v_size(oobj obj);

osize OStreamSocket__v_seek(oobj obj, osize offset, enum OStream_seek_whence whence);

osize OStreamSocket__v_read_try(oobj obj, void *data, osize element_size, osize num);

osize OStreamSocket__v_write(oobj obj, const void *data, osize element_size, osize num);

bool OStreamSocket__v_close(oobj obj);

//
// object functions
//


/**
 * @param obj OStreamSocket object
 * @return socket read timeout. Defaults to -1 to be infinite (large on sdl2).
 *         On timeout, read may return 0 even if the socket is valid.
 */
OObj_DECL_GET(OStreamSocket, oi32, timeout)

#endif //O_OSTREAMSOCKET_H
#endif //MIA_OPTION_SOCKET
