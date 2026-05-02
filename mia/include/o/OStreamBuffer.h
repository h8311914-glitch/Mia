#ifndef O_OSTREAMBUFFER_H
#define O_OSTREAMBUFFER_H

/**
 * @file OStreamBuffer.h
 *
 * Object
 *
 * OStream implementation which reads ahead buffered
 * and buffers writings until the buffer is full or OStreamBuffer_write_flush() is called.
 *
 * So with less real stream reads and writes, will be much faster for io stuff.
 *     If you're reading a file or smth that fits easily into ram,
 *     you should read all in once for max speed instead.
 *     If you're preparing to write smth that easily fits into ram, create an OArray or smth first
 *     and send the full array all in once for max speed instead.
 * An OStreamBuffer shines where it's not predictable how much data will be available to read or write.
 *     Like in a http request.
 *
 * Only need a read buffer (or only a write buffer)? Leave the others buffer size at 0, which will ignore buffering.
 *
 * A buffered stream has no size, nor is it seekable!
 *
 * Allocated internal buffer is 2*read_buffer_size + 1*write_buffer_size
 */

#include "OStream.h"

/** object id */
#define OStreamBuffer_ID OStream_ID "Buffer."


typedef struct {
    OStream super;

    OStream *stream;
    bool auto_close;

    // double-spaced buffer for read
    obyte *read_buffer, *write_buffer;
    osize read_buffer_size, write_buffer_size;

    // relative to buffer (for the double-spaces buffer)
    osize read_start;

    // internal valid buffer [0:buffer_size]
    osize read_size, write_size;
    
    // last write sets it
    osize flush_element_size;
} OStreamBuffer;


/**
 * Initializes the object
 * @param obj OStreamBuffer object
 * @param parent to inherit from
 * @param stream The OStream to work on
 * @param read_buffer_size The buffer size to maximal read ahead, <=0 to ignore
 * @param write_buffer_size The buffer size to buffer writings until full or flushed, <=0 to ignore
 * @param auto_close calls close on the stream if this stream is closed
 * @return obj casted as OStreamBuffer
 */
OStreamBuffer *OStreamBuffer_init(oobj obj, oobj parent, oobj stream,
                                  osize read_buffer_size, osize write_buffer_size, bool auto_close);


/**
 * Creates a new OStreamBuffer object
 * @param parent to inherit from
* @param stream The OStream to work on
 * @param read_buffer_size The buffer size to maximal read ahead, <=0 to ignore
 * @param write_buffer_size The buffer size to buffer writings until full or flushed, <=0 to ignore
 * @param auto_close calls close on the stream if this stream is closed
 * @return The new object
 */
o_inline OStreamBuffer *OStreamBuffer_new(oobj parent, oobj stream,
                                          osize read_buffer_size, osize write_buffer_size, bool auto_close)
{
    OObj_DECL_IMPL_NEW(OStreamBuffer, parent, stream, read_buffer_size, write_buffer_size, auto_close);
}

//
// virtual implementations:
//

bool OStreamBuffer__v_valid(oobj obj);

osize OStreamBuffer__v_size(oobj obj);

osize OStreamBuffer__v_seek(oobj obj, osize offset, enum OStream_seek_whence whence);

osize OStreamBuffer__v_read_try(oobj obj, void *data, osize element_size, osize num);

osize OStreamBuffer__v_write(oobj obj, const void *data, osize element_size, osize num);

/** Calls OStreamBuffer_write_flush first */
bool OStreamBuffer__v_close(oobj obj);

//
// object functions
//

/**
 * Prereads manually into the read buffer.
 * @param obj OStreamBuffer object
 * @return elements read from the socket (not bytes)
 */
osize OStreamBuffer_read_ahead(oobj obj, osize element_size);

/**
 * Flushes manually the write buffer to write the full buffer to the socket.
 * Uses element_size of the last write call.
 * @param obj OStreamBuffer object
 * @return bytes written to the socket (not elements)
 */
osize OStreamBuffer_write_flush(oobj obj);

#endif //O_OSTREAMBUFFER_H
