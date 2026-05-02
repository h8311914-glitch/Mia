#include "o/OStreamCache.h"
#include "o/OObj_builder.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


OStreamCache *OStreamCache_init(oobj obj, oobj parent, oobj stream, osize buffer_size, bool auto_close)
{
    OStreamCache *self = obj;
    o_clear(self, sizeof *self, 1);

    OStream_init(obj, parent,
                 OStreamCache__v_valid,
                 OStreamCache__v_size,
                 OStreamCache__v_seek,
                 OStreamCache__v_read_try,
                 OStreamCache__v_write,
                 OStreamCache__v_close);
    OObj_id_set(self, OStreamCache_ID);

    OObj_assert(stream, OStream);
    self->stream = stream;
    self->auto_close = auto_close;

    assert(buffer_size > 0);
    self->buffer = o_new0(self, obyte, buffer_size);
    self->buffer_size = buffer_size;

    self->size = self->start = self->pos = 0;

    return self;
}


bool OStreamCache__v_valid(oobj obj)
{
    OObj_assert(obj, OStreamCache);
    OStreamCache *self = obj;
    return OStream_valid(self->stream);
}

osize OStreamCache__v_size(oobj obj)
{
    OObj_assert(obj, OStreamCache);
    OStreamCache *self = obj;
    return self->size;
}

osize OStreamCache__v_seek(oobj obj, osize offset, enum OStream_seek_whence whence)
{
    OObj_assert(obj, OStreamCache);
    OStreamCache *self = obj;
    if(whence == OStream_SET) {
        self->pos = offset;
    } else if (whence == OStream_CUR) {
        self->pos += offset;
    } else if (whence == OStream_END) {
        self->pos = self->size + offset;
    }
    self->pos = o_clamp(self->pos, 0, self->size - 1);
    return self->pos;
}

osize OStreamCache__v_read_try(oobj obj, void *out_data, osize element_size, osize num)
{
    OObj_assert(obj, OStreamCache);
    OStreamCache *self = obj;

    osize bytes_num = osize_mul(element_size, num);

    // remaining bytes in the valid buffer
    osize buffer_rem = self->size - self->pos;
    osize buffer_read = o_min(buffer_rem, bytes_num);
    osize stream_read = bytes_num - buffer_read;

    obyte *data_it = out_data;
    for (osize i = 0; i < buffer_read; i++) {
        // pos is relative to start in the ring buffer
        osize buffer_pos = o_mod(self->start + self->pos, self->buffer_size);
        *data_it++ = self->buffer[buffer_pos];
        self->pos++;
        assert(self->pos <= self->size);
    }

    osize read = 0;
    if (stream_read > 0) {
        read = OStream_read_try(self->stream, data_it, 1, stream_read);

        // let the compiler optimize that...
        for (osize i = 0; i < read; i++) {
            // pos is relative to start in the ring buffer
            osize buffer_pos = o_mod(self->start + self->pos, self->buffer_size);
            self->buffer[buffer_pos] = data_it[i];
            if(self->size < self->buffer_size) {
                // grow internal valid buffer
                self->size++;
                self->pos++;
                assert(self->pos <= self->size);
            } else {
                // shift start and loose bytes in the buffer
                self->start = o_mod(self->start + 1, self->buffer_size);
            }
        }
    }

    return buffer_read + read;
}

osize OStreamCache__v_write(oobj obj, const void *data, osize element_size, osize num)
{
    OObj_assert(obj, OStreamCache);
    OStreamCache *self = obj;
    // not buffered, just calls streams write
    return OStream_write(self->stream, data, element_size, num);
}

bool OStreamCache__v_close(oobj obj)
{
    OObj_assert(obj, OStreamCache);
    OStreamCache *self = obj;
    if(self->auto_close) {
        return OStream_close(self->stream);
    }
    return true;
}

