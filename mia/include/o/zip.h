#ifdef MIA_OPTION_ZIP
#ifndef O_ZIP_H
#define O_ZIP_H

/**
 * @file zip.h
 *
 * Compress and uncompress using the zlib deflate algorithm via miniz.
 *
 * Makes use of miniz "https://github.com/richgel999/miniz"
 */

#include "common.h"


/**
 * @param uncompressed_size data size in bytes to be compressed
 * @return minimal compressed data buffer size needed.
 *         May be larger what o_zip_compress returns.
 * @note there can't be a simple o_zip_uncompress_buffer_bound function,
 *       as we cant know how well the data is compressed.
 */
osize o_zip_compress_buffer_bound(osize uncompressed_size);


/**
 * Compresses a blob of data
 * @param dst_compressed result, needs a buffer size of 
 *                       at least o_zip_compress_buffer_bound
 * @param dst_size from o_zip_compress_buffer_bound
 * @param src_uncompressed, raw data to be compressed
 * @param src_size byte size of src
 * @return byte size of the compressed data, or 0 on error
 */
osize o_zip_compress(void *dst_compressed, osize dst_size, const void *src_uncompressed, osize src_size);

/**
 * Unompresses a blob of data
 * @param dst_uncompressed result, its buffer size needs to be known ahead of time!
 * @param dst_size size before once compressed via o_zip_compress
 * @param src_compressed, compressed data to be uncompressed
 * @param src_size byte size of src
 * @return true on success
 */
bool o_zip_uncompress(void *dst_uncompressed, osize dst_size, const void *src_compressed, osize src_size);


#endif //O_ZIP_H
#endif //MIA_OPTION_ZIP
