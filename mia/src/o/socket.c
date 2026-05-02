#ifdef MIA_OPTION_SOCKET


#ifdef MIA_OPTION_SDL2
#  include "SDL2/SDL_net.h"
#else
#  include "SDL3_net/SDL_net.h"
#endif


#include "o/OStreamSocket.h"
#include "o/str.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


struct oobj_opt o_socket_open(oobj parent, const char *address, ou16 port)
{
    if(!address) {
        address = "127.0.0.1";
    }

#ifdef MIA_OPTION_SDL2
    IPaddress ip;
    if(SDLNet_ResolveHost(&ip, address, port) == -1) {
        o_log_warn_s(__func__, "failed to resolve host: %s", SDLNet_GetError());
        return oobj_opt(NULL);
    }

    TCPsocket socket = SDLNet_TCP_Open(&ip);
    if(!socket) {
        o_log_warn_s(__func__, "failed to create the connection");
        return oobj_opt(NULL);
    }

    // impl->so will be NULL on error, so _valid check would fail
#else
    NET_Address *ip = NET_ResolveHostname(address);
    if (!ip) {
        o_log_warn_s(__func__, "failed to resolve host (init): %s", SDL_GetError());
        return oobj_opt(NULL);
    }
    if (NET_WaitUntilResolved(ip, -1) <=0) {
        o_log_warn_s(__func__, "failed to resolve host: %s", SDL_GetError());
        NET_UnrefAddress(ip);
        return oobj_opt(NULL);
    }

    NET_StreamSocket *socket = NET_CreateClient(ip, port);
    NET_UnrefAddress(ip);

    if(!socket) {
        o_log_warn_s(__func__, "failed to create the connection (init): %s", SDL_GetError());
        return oobj_opt(NULL);
    }
    if (NET_WaitUntilConnected(socket, -1) <=0) {
        o_log_warn_s(__func__, "failed to create the connection: %s", SDL_GetError());
        NET_DestroyStreamSocket(socket);
        return oobj_opt(NULL);
    }

    NET_UnrefAddress(ip);
#endif


    oobj stream = OStreamSocket_new(parent, socket);

    char buf[64];
    o_strf_buf(buf, "Socket#%s:%i", address, port);
    OObj_name_set(stream, buf);

    return oobj_opt(stream);
}



#endif // MIA_OPTION_SOCKET
typedef int avoid_empty_translation_unit;

