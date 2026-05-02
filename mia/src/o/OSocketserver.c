#ifdef MIA_OPTION_SOCKET

#ifdef MIA_OPTION_SDL2
#  define NET_DestroyServer SDLNet_TCP_Close
#  include "SDL2/SDL_net.h"
#else
#  include "SDL3_net/SDL_net.h"
#endif

#include "o/OSocketserver.h"
#include "o/OObj_builder.h"
#include "o/OStreamSocket.h"
#include "o/str.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


#define RETRY_DELAY_MS 100


OSocketserver *OSocketserver_init(oobj obj, oobj parent, ou16 port)
{
    OSocketserver *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OSocketserver_ID);

    self->timeout = -1;

#ifdef MIA_OPTION_SDL2
    IPaddress ip;
    if(SDLNet_ResolveHost(&ip, INADDR_ANY, port) == -1) {
        o_log_warn_s(__func__, "failed to resolve host: %s", SDLNet_GetError());
        return self;
    }
    self->sdl_server = SDLNet_TCP_Open(&ip);
    if(!self->sdl_server) {
        o_log_warn_s(__func__, "failed to create the server socket");
        return self;
    }
    self->sdl_socket_set = SDLNet_AllocSocketSet(1);
    o_assume(self->sdl_socket_set, "failed to allocate the socket set");
    SDLNet_AddSocket(self->sdl_socket_set, self->sdl_server);

#else
    self->sdl_server = NET_CreateServer(NULL, port);
    if(!self->sdl_server) {
        o_log_warn_s(__func__, "failed to create the server socket: %s", SDL_GetError());
        return self;
    }

#endif

    char buf[64];
    o_strf_buf(buf, "Server#:%i", port);
    OObj_name_set(self, buf);

    // vfuncs
    self->super.v_del = OSocketserver__v_del;

    return self;
}

//
// virtual functions
//


void OSocketserver__v_del(oobj obj)
{
    OObj_assert(obj, OSocketserver);
    OSocketserver *self = obj;
    if(self->sdl_server) {
        NET_DestroyServer(self->sdl_server);
    }
#ifdef MIA_OPTION_SDL2
    if (self->sdl_socket_set) {
        SDLNet_FreeSocketSet(self->sdl_socket_set);
    }
#endif
    OObj__v_del(self);
}


//
// object functions
//


struct oobj_opt OSocketserver_accept_try(oobj obj)
{
    OObj_assert(obj, OSocketserver);
    OSocketserver *self = obj;
    if(!self->sdl_server) {
        return oobj_opt(NULL);
    }


#ifdef MIA_OPTION_SDL2
    ou32 timeout = self->timeout>=0? self->timeout : ou32_MAX;
    int got_data = SDLNet_CheckSockets(self->sdl_socket_set, timeout);
#else
    int got_data = NET_WaitUntilInputAvailable(o_list_compound(void*, self->sdl_server), 1, self->timeout);
#endif
    if(got_data <= 0) {
        return oobj_opt(NULL);
    }

#ifdef MIA_OPTION_SDL2
    TCPsocket socket = SDLNet_TCP_Accept(self->sdl_server);
    if(!socket) {
        return oobj_opt(NULL);
    }
    IPaddress *client_ip = SDLNet_TCP_GetPeerAddress(socket);
    if(!client_ip) {
        o_log_debug_s(__func__, "failed to get client ip address");
    } else {
        o_log_s(__func__, "connected with: %s", SDLNet_ResolveIP(client_ip));
    }
    return oobj_opt(OStreamSocket_new(obj, socket));

#else
    NET_StreamSocket *socket;
    bool valid = NET_AcceptClient(self->sdl_server, &socket);
    if (!valid) {
        o_log_warn_s(__func__, "accepting from the server socket failed: %s", SDL_GetError());
        return oobj_opt(NULL);
    }
    if (!socket) {
        return oobj_opt(NULL);
    }
    NET_Address *client_ip = NET_GetStreamSocketAddress(socket);
    if(!client_ip) {
        o_log_debug_s(__func__, "failed to get client ip address");
    } else {
        o_log_s(__func__, "connected with: %s", NET_GetAddressString(client_ip));
    }
    NET_UnrefAddress(client_ip);
    return oobj_opt(OStreamSocket_new(obj, socket));



#endif
}


OStream *OSocketserver_accept(oobj obj)
{
    for(;;) {
        struct oobj_opt client = OSocketserver_accept_try(obj);
        if(client.o) {
            return client.o;
        }
        o_sleep(RETRY_DELAY_MS);
    }
}

#endif // MIA_OPTION_SOCKET
typedef int avoid_empty_translation_unit;

