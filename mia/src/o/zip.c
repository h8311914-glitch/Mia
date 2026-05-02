#ifdef MIA_OPTION_ZIP

#include "o/zip.h"

#include "OArchive_miniz.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"


//
// public
//

osize o_zip_compress_buffer_bound(osize uncompressed_size)
{
    return mz_compressBound(uncompressed_size);
}


osize o_zip_compress(void *dst_compressed, osize dst_size, const void *src_uncompressed, osize src_size)
{
    assert(dst_compressed && src_uncompressed && src_size>0);
    mz_ulong dst_len = dst_size;
    int res = mz_compress(dst_compressed, &dst_len, src_uncompressed, src_size);
    if (res != MZ_OK || dst_len == 0) {
        o_log_error_s(__func__, "compression failed!");
        return 0;
    }
    return dst_len;
}

bool o_zip_uncompress(void *dst_uncompressed, osize dst_size, const void *src_compressed, osize src_size)
{
    assert(dst_uncompressed && src_compressed && src_size>0);
    mz_ulong dst_len = dst_size;
    int res = mz_uncompress(dst_uncompressed, &dst_len, src_compressed, src_size);
    if (res != MZ_OK || dst_len == 0) {
        o_log_error_s(__func__, "uncompression failed!");
        return false;
    }
    if(dst_len != dst_size) {
        o_log_warn_s(__func__, "invalid uncompressed size?: %i / %i", dst_len, dst_size);
    }
    return true;
}

#endif //MIA_OPTION_ZIP
typedef int avoid_empty_translation_unit;
