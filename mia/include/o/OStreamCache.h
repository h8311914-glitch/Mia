#ifndef O_OSTREAMCACHE_H
#define O_OSTREAMCACHE_H

/**
 * @file OStreamCache.h
 *
 * Object
 *
 * OStream implementation which adds a readed cache buffer to an existing stream
 * so always seekable within the buffer
 */

#include "OStream.h"

/** object id */
#define OStreamCache_ID OStream_ID "Cache."


typedef struct {
    OStream super;

    OStream *stream;
    bool auto_close;

    // ring buffer
    obyte *buffer;
    osize buffer_size;

    // relative to buffer
    osize start;

    // internal valid buffer [0:buffer_size]
    osize size;

    // position relative to start
    //      so the current byte read is buffer[mod(start + pos, buffer_size)]
    osize pos;

} OStreamCache;


/**
 * Initializes the object
 * @param obj OStreamCache object
 * @param parent to inherit from
 * @param stream The OStream to work on
 * @param buffer_size The buffer size in which seeks work
 * @param auto_close calls close on the stream if this stream is closed
 * @return obj casted as OStreamCache
 */
OStreamCache *OStreamCache_init(oobj obj, oobj parent, oobj stream, osize buffer_size, bool auto_close);


/**
 * Creates a new OStreamCache object
 * @param parent to inherit from
 * @param stream The OStream to work on
 * @param buffer_size The buffer size in which seeks work
 * @param auto_close calls close on the stream if this stream is closed
 * @return The new object
 */
o_inline OStreamCache *OStreamCache_new(oobj parent, oobj stream, osize buffer_size, bool auto_close)
{
    OObj_DECL_IMPL_NEW(OStreamCache, parent, stream, buffer_size, auto_close);
}

//
// virtual implementations:
//

bool OStreamCache__v_valid(oobj obj);

osize OStreamCache__v_size(oobj obj);

osize OStreamCache__v_seek(oobj obj, osize offset, enum OStream_seek_whence whence);

osize OStreamCache__v_read_try(oobj obj, void *data, osize element_size, osize num);

osize OStreamCache__v_write(oobj obj, const void *data, osize element_size, osize num);

bool OStreamCache__v_close(oobj obj);



#endif //O_OSTREAMCACHE_H
