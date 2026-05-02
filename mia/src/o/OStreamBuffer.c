#include "o/OStreamBuffer.h"
#include "o/OObj_builder.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


OStreamBuffer *OStreamBuffer_init(oobj obj, oobj parent, oobj stream,
                                  osize read_buffer_size, osize write_buffer_size, bool auto_close)
{
    OStreamBuffer *self = obj;
    o_clear(self, sizeof *self, 1);

    OStream_init(obj, parent,
                 OStreamBuffer__v_valid,
                 OStreamBuffer__v_size,
                 OStreamBuffer__v_seek,
                 OStreamBuffer__v_read_try,
                 OStreamBuffer__v_write,
                 OStreamBuffer__v_close);
    OObj_id_set(self, OStreamBuffer_ID);

    OObj_assert(stream, OStream);
    self->stream = stream;
    self->auto_close = auto_close;

    self->read_buffer_size = o_max(0, read_buffer_size);
    self->write_buffer_size = o_max(0, write_buffer_size);

    osize buffer_size = 2*self->read_buffer_size + 1*self->write_buffer_size;
    if (buffer_size > 0) {
        obyte *buffer = o_new0(self, obyte, buffer_size);
        if (self->read_buffer_size > 0) {
            self->read_buffer = buffer;
        }
        if (self->write_buffer_size > 0) {
            self->write_buffer = buffer + 2*self->read_buffer_size;
        }
    }

    self->read_size = self->write_size = 0;
    self->read_start = 0;

    return self;
}


bool OStreamBuffer__v_valid(oobj obj)
{
    OObj_assert(obj, OStreamBuffer);
    OStreamBuffer *self = obj;
    return OStream_valid(self->stream);
}

osize OStreamBuffer__v_size(oobj obj)
{
    // a buffered stream has no size
    return -1;
}

osize OStreamBuffer__v_seek(oobj obj, osize offset, enum OStream_seek_whence whence)
{
    // a buffered stream cant be seeked
    return -1;
}

osize OStreamBuffer__v_read_try(oobj obj, void *out_data, osize element_size, osize num)
{
    OObj_assert(obj, OStreamBuffer);
    OStreamBuffer *self = obj;
    
    assert(element_size>0);

    // no buffer -> just delegate
    if(self->read_buffer_size<=0) {
        return OStream_read_try(self->stream, out_data, element_size, num);
    }

    osize elements_read = 0;
    obyte *data_it = out_data;
    while (elements_read < num) {
        if(self->read_size<element_size) {
            // move remaining to the front to avoid an overflow situation of the double buffer
            if (self->read_size>0 && self->read_start>0) {
                o_memmove(self->read_buffer, self->read_buffer+self->read_start, 1, self->read_size);
                self->read_start = 0;
            }
            osize read_ahead = OStreamBuffer_read_ahead(self, element_size);
            if(read_ahead==0) {
                break;
            }
        }
        osize buffer_elements = self->read_size / element_size;
        if(buffer_elements>0) {
            osize remaining = (num-elements_read);
            osize buffered = o_min(remaining, buffer_elements);
            osize buffered_bytes = buffered*element_size;
            o_memcpy(data_it, self->read_buffer + self->read_start, 1, buffered_bytes);
            data_it += buffered_bytes;
            self->read_start += buffered_bytes;
            self->read_size -= buffered_bytes;
            elements_read += buffered;
        }
        
    }
    return elements_read;
}

osize OStreamBuffer__v_write(oobj obj, const void *data, osize element_size, osize num)
{
    OObj_assert(obj, OStreamBuffer);
    OStreamBuffer *self = obj;

    // no buffer -> just delegate
    if(self->write_buffer_size<=0) {
        return OStream_write(self->stream, data, element_size, num);
    }
    
    self->flush_element_size = element_size;

    osize bytes_num = osize_mul(element_size, num);
    const obyte *data_it = data;
    while(bytes_num>0) {
        osize space = self->write_buffer_size-self->write_size;
        osize buffered = o_min(bytes_num, space);
        if(buffered>0) {
            o_memcpy(self->write_buffer+self->write_size,
                     data_it, 1, buffered);
            data_it += buffered;
            self->write_size += buffered;
            bytes_num -= buffered;
        }
        if(self->write_size>=self->write_buffer_size) {
            osize flushed = OStreamBuffer_write_flush(self);
            if(self->write_size>0) {
                // failed to flush
                return 0;
            }
        }
    }
    return num;
}

bool OStreamBuffer__v_close(oobj obj)
{
    OObj_assert(obj, OStreamBuffer);
    OStreamBuffer *self = obj;
    OStreamBuffer_write_flush(self);
    if (self->auto_close) {
        return OStream_close(self->stream);
    }
    return true;
}


osize OStreamBuffer_read_ahead(oobj obj, osize element_size)
{
    OObj_assert(obj, OStreamBuffer);
    OStreamBuffer *self = obj;

    assert(element_size>0);
    
    // no buffer,  can't read ahead
    if(self->read_buffer_size<=0) {
        return 0;
    }
    
    osize read_ahead_bytes = self->read_buffer_size - self->read_size;
    osize read_ahead_elements = read_ahead_bytes / element_size;
    osize read_elements = 0;
    if (read_ahead_elements > 0) {
        if(self->read_size==0) {
            self->read_start = 0;
        }
        read_elements = OStream_read_try(self->stream, self->read_buffer + self->read_start + self->read_size, element_size, read_ahead_elements);
        osize read_bytes = read_elements * element_size;
        self->read_size += read_bytes;
        o_log_trace_s(__func__, "read_ahead: %i", read_bytes);
    }
    return read_elements;
}

osize OStreamBuffer_write_flush(oobj obj)
{
    OObj_assert(obj, OStreamBuffer);
    OStreamBuffer *self = obj;
    if (self->write_buffer_size <= 0 || self->flush_element_size <= 0) {
        return 0;
    }
    osize written_bytes = 0;
    osize num = self->write_size / self->flush_element_size;
    if(num>0) {
        osize written_elements = OStream_write(self->stream, self->write_buffer, self->flush_element_size, num);
        written_bytes = written_elements * self->flush_element_size;
        self->write_size = o_max(0, self->write_size-written_bytes);
        o_log_trace_s(__func__, "write_flushed: %i", written_bytes);
    }
    return written_bytes;
}
