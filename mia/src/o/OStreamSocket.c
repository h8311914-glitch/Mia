#ifdef MIA_OPTION_SOCKET


#ifdef MIA_OPTION_SDL2
#  include "SDL2/SDL_net.h"
#  define NET_ReadFromStreamSocket SDLNet_TCP_Recv
#  define NET_DestroyStreamSocket SDLNet_TCP_Close
#else
#  include "SDL3_net/SDL_net.h"
#endif

#include "o/OStreamSocket.h"
#include "o/OObj_builder.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


OStreamSocket *OStreamSocket_init(oobj obj, oobj parent, void *sdl_socket_sink)
{
    OStreamSocket *self = obj;
    o_clear(self, sizeof *self, 1);

    OStream_init(obj, parent,
                 OStreamSocket__v_valid,
                 OStreamSocket__v_size,
                 OStreamSocket__v_seek,
                 OStreamSocket__v_read_try,
                 OStreamSocket__v_write,
                 OStreamSocket__v_close);
    OObj_id_set(self, OStreamSocket_ID);

    // set the sink
    self->sdl_socket = sdl_socket_sink;

#ifdef MIA_OPTION_SDL2
    self->sdl_socket_set = SDLNet_AllocSocketSet(1);
    o_assume(self->sdl_socket_set, "failed to allocate the socket set");
    SDLNet_TCP_AddSocket(self->sdl_socket_set, self->sdl_socket);
#endif

    self->timeout = -1;

    return self;
}


bool OStreamSocket__v_valid(oobj obj)
{
    OObj_assert(obj, OStreamSocket);
    OStreamSocket *self = obj;
    return self->sdl_socket != NULL;
}

osize OStreamSocket__v_size(oobj obj)
{
    // a socket stream has no size
    return -1;
}

osize OStreamSocket__v_seek(oobj obj, osize offset, enum OStream_seek_whence whence)
{
    // a socket stream cant be seeked
    return -1;
}

osize OStreamSocket__v_read_try(oobj obj, void *out_data, osize element_size, osize num)
{
    OObj_assert(obj, OStreamSocket);
    OStreamSocket *self = obj;
    if (!self->sdl_socket)
        return 0;

    osize bytes_num = osize_mul(element_size, num);
    int bytes_num_int = (int) bytes_num;
    assert(bytes_num >= 0 && bytes_num_int == bytes_num);

#ifdef MIA_OPTION_SDL2
    ou32 timeout = self->timeout>=0? self->timeout : ou32_MAX;
    int got_data = SDLNet_CheckSockets(self->sdl_socket_set, timeout);
#else
    int got_data = NET_WaitUntilInputAvailable(o_list_compound(void*, self->sdl_socket), 1, self->timeout);
#endif
    if(got_data <= 0) {
        return 0;
    }

    osize read = NET_ReadFromStreamSocket(self->sdl_socket, out_data, bytes_num_int);

    osize read_elements = read / element_size;
    int remaining = (int) (element_size - (read % element_size));

    // hopefully, we wont ever get here...
    if(remaining != element_size) {
        o_log_debug_s("OStreamSocket_read_try", "need remaining bytes of an element: %i/%i",
                    remaining, (int) element_size);

        obyte *it = out_data;
        it += read;

        // add the remaining read element
        read_elements++;

        while(remaining > 0) {
            int rem_read = NET_ReadFromStreamSocket(self->sdl_socket, it, remaining);
            if(rem_read <= 0) {
                o_log_debug_s("OStreamSocket_read_try", "failed reading the remaining bytes",
                           remaining, (int) element_size);
                read_elements = 0;
                break;
            }
            remaining -= rem_read;
            it += rem_read;
        }
    }
    return read_elements;
}

osize OStreamSocket__v_write(oobj obj, const void *data, osize element_size, osize num)
{
    OObj_assert(obj, OStreamSocket);
    OStreamSocket *self = obj;
    if (!self->sdl_socket)
        return 0;

    osize bytes_num = osize_mul(element_size, num);
    int bytes_num_int = (int) bytes_num;
    assert(bytes_num >= 0 && bytes_num_int == bytes_num);

#ifdef MIA_OPTION_SDL2
    osize written = SDLNet_TCP_Send(self->sdl_socket, data, bytes_num_int);
    osize elements_written = written / element_size;
    if (elements_written < num) {
        OStream_close(self);
    }
    return elements_written;
#else
    bool valid = NET_WriteToStreamSocket(self->sdl_socket, data, bytes_num_int);
    if (!valid) {
        OStream_close(self);
        return 0;
    }
    return num;
#endif
}

bool OStreamSocket__v_close(oobj obj)
{
    OObj_assert(obj, OStreamSocket);
    OStreamSocket *self = obj;
    if (!self->sdl_socket)
        return true;

    // cant fail?
    NET_DestroyStreamSocket(self->sdl_socket);
    self->sdl_socket = NULL;
#ifdef MIA_OPTION_SDL2
    if (self->sdl_socket_set) {
        SDLNet_FreeSocketSet(self->sdl_socket_set);
        self->sdl_socket_set = NULL;
    }
#endif
    return true;
}


#endif // MIA_OPTION_SOCKET
typedef int avoid_empty_translation_unit;

