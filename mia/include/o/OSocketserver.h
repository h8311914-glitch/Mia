#ifdef MIA_OPTION_SOCKET
#ifndef O_OSOCKETSERVER_H
#define O_OSOCKETSERVER_H

/**
 * @file OSocketserver.h
 *
 * Object
 *
 * A SocketServer object starts a TCP server and accepts clients as OStreamSocket
 */


#include "OObj.h"
#include "OStream.h"

/** object id */
#define OSocketserver_ID OObj_ID "OSocketserver."


typedef struct {
    OObj super;

    oi32 timeout;
    void *sdl_server;

#ifdef MIA_OPTION_SDL2
    void *sdl_socket_set;
#endif
} OSocketserver;


/**
 * Initializes the object
 * @param obj OSocketserver object
 * @param parent to inherit from
 * @param port the port for the server
 * @return obj casted as OSocketserver
 */
OSocketserver *OSocketserver_init(oobj obj, oobj parent, ou16 port);

/**
 * Creates a new OSocketserver object
 * @param parent to inherit from
 * @param port the port for the server
 * @return The new object
 */
o_inline OSocketserver *OSocketserver_new(oobj parent, ou16 port)
{
    OObj_DECL_IMPL_NEW(OSocketserver, parent, port);
}

//
// v implementations
//

/**
 * Virtual function
 * Default deletor that closes the server.
 * @param obj OSocketserver object
 */
void OSocketserver__v_del(oobj obj);

//
// object functions:
//


/**
 * @param obj OSocketStream object
 * @return true if valid
 */
o_inline bool OSocketserver_valid(oobj obj)
{
    OObj_assert(obj, OSocketserver);
    OSocketserver *self = obj;
    return self->sdl_server != NULL;
}


/**
 * Accepts a client, if available, does not block
 * @param obj OSocketserver object
 * @return an OStream object of the client to read and write with the name, or NULL if not available
 */
struct oobj_opt OSocketserver_accept_try(oobj obj);


/**
 * Accepts a client, blocks until connected
 * @param obj OSocketserver object
 * @return an OStreamSocket object of the client to read and write with the name.
 */
OStream *OSocketserver_accept(oobj obj);


/**
 * @param obj OSocketserver object
 * @return socket accept timeout. Defaults to -1 to be infinite (large on sdl2).
 *         On timeout, accept may return 0 even if the socket is valid.
 */
OObj_DECL_GET(OSocketserver, oi32, timeout)


#endif //O_OSOCKETSERVER_H
#endif //MIA_OPTION_SOCKET
