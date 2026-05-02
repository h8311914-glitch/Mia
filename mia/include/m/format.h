#ifndef M_FORMAT_H
#define M_FORMAT_H

/**
 * @file format.h
 *
 * Pixel formats for MMat and co.
 * Does not match available vec* and mat* primitive formats of the m module.
 * Passed as ou32 cause its a bitflag, containing of:
 * - channels
 * - primitive
 *   - primtive_type: signed_raw, unsigned_raw, unsigned_normalized, float
 *   - primive_size: in bytes
 *
 * Internally an MMat can have a custom size for the pixeldata.
 * But most functions expect a valid M_FORMAT_* and not M_FORMAT_CUSTOM.
 * Like in gl and RTex, most functions just cast to a float to be format independent.
 */

#include "o/common.h"
#include "m/m_types/flt.h"

/**
 * Using constants instead of enums, because:
 * The format is using bitflags.
 * RTex from the r module also needs formats like these.
 * The MMat formats are a superset, cause not all may be available in RTex for use (double).
 * These formats should nevertheless be interchangeable, so M_FORMAT_R_8 == R_FORMAT_R_U8.
 * Also some formats here have alias names, cause the MMat is more general and not only for images.
 * If not custom, all values are always packed
 */

#define M_FORMAT_KEEP 0
#define M_FORMAT_CUSTOM -1


// byte size for each primitive, 1 for U8, 4 for IR32 and F32
#define M_FORMAT__PRIMITIVE_SIZE_MASK 0x0f

// IR (signed integer; raw)
// UR (unsigned integer; raw)
// U (unsigned integer; casts to [0.0:1.0])
// F (float)
#define M_FORMAT__PRIMITIVE_TYPE_SHIFT 4
#define M_FORMAT__PRIMITIVE_TYPE_MASK 0x0f
#define M_FORMAT__PRIMITIVE_TYPE_IR 1
#define M_FORMAT__PRIMITIVE_TYPE_UR 2
#define M_FORMAT__PRIMITIVE_TYPE_U 3
#define M_FORMAT__PRIMITIVE_TYPE_F 4


// primitive types
#define M_FORMAT__PRIMITIVE_MASK 0xff
#define M_FORMAT__PRIMITIVE_IR8     ((M_FORMAT__PRIMITIVE_TYPE_IR   <<4) | 1)
#define M_FORMAT__PRIMITIVE_UR8     ((M_FORMAT__PRIMITIVE_TYPE_UR   <<4) | 1)
#define M_FORMAT__PRIMITIVE_U8      ((M_FORMAT__PRIMITIVE_TYPE_U    <<4) | 1)
#define M_FORMAT__PRIMITIVE_IR16    ((M_FORMAT__PRIMITIVE_TYPE_IR   <<4) | 2)
#define M_FORMAT__PRIMITIVE_UR16    ((M_FORMAT__PRIMITIVE_TYPE_UR   <<4) | 2)
#define M_FORMAT__PRIMITIVE_U16     ((M_FORMAT__PRIMITIVE_TYPE_U    <<4) | 2)
#define M_FORMAT__PRIMITIVE_IR32    ((M_FORMAT__PRIMITIVE_TYPE_IR   <<4) | 4)
#define M_FORMAT__PRIMITIVE_UR32    ((M_FORMAT__PRIMITIVE_TYPE_UR   <<4) | 4)
#define M_FORMAT__PRIMITIVE_U32     ((M_FORMAT__PRIMITIVE_TYPE_U    <<4) | 4)
#define M_FORMAT__PRIMITIVE_IR64    ((M_FORMAT__PRIMITIVE_TYPE_IR   <<4) | 8)
#define M_FORMAT__PRIMITIVE_UR64    ((M_FORMAT__PRIMITIVE_TYPE_UR   <<4) | 8)
#define M_FORMAT__PRIMITIVE_U64     ((M_FORMAT__PRIMITIVE_TYPE_U    <<4) | 8)
#define M_FORMAT__PRIMITIVE_F32     ((M_FORMAT__PRIMITIVE_TYPE_F    <<4) | 4)
#define M_FORMAT__PRIMITIVE_F64     ((M_FORMAT__PRIMITIVE_TYPE_F    <<4) | 8)

// channels as integer such as 1, 2, 3, 4
#define M_FORMAT__CHANNELS_SHIFT 8
#define M_FORMAT__CHANNELS_MASK 0xff


/**
 * generate a format int
 */
#define M_FORMAT_X_FULL(channels, type, size) (((channels) << 8) | ((type) << 4) | size)
#define M_FORMAT_X(channels, primitive) (((channels) << 8) | (primitive) & 0xff)

/**
 * Maximal byte size for a field (channels * primitive_size).
 * Most functions work with abritary sizes, but some use this a maximum.
 * would be 8 channels with double precision, or 16 channels with float precision.
 */
#define M_FORMAT_MAX_SIZE 64

// integer formats
#define M_FORMAT_1_IR8 M_FORMAT_X_FULL(1, 1, 1)
#define M_FORMAT_1_UR8 M_FORMAT_X_FULL(1, 2, 1)
#define M_FORMAT_1_IR16 M_FORMAT_X_FULL(1, 1, 2)
#define M_FORMAT_1_UR16 M_FORMAT_X_FULL(1, 2, 2)
#define M_FORMAT_1_IR32 M_FORMAT_X_FULL(1, 1, 4)
#define M_FORMAT_1_UR32 M_FORMAT_X_FULL(1, 2, 4)

// image formats (mostly)
#define M_FORMAT_1_U8 M_FORMAT_X_FULL(1, 3, 1)
#define M_FORMAT_1_F32 M_FORMAT_X_FULL(1, 4, 4)
#define M_FORMAT_2_U8 M_FORMAT_X_FULL(2, 3, 1)
#define M_FORMAT_2_F32 M_FORMAT_X_FULL(2, 4, 4)
#define M_FORMAT_3_U8 M_FORMAT_X_FULL(3, 3, 1)
#define M_FORMAT_3_F32 M_FORMAT_X_FULL(3, 4, 4)
#define M_FORMAT_4_U8 M_FORMAT_X_FULL(4, 3, 1)
#define M_FORMAT_4_F32 M_FORMAT_X_FULL(4, 4, 4)

// rgba alias
#define M_FORMAT_R_U8 M_FORMAT_1_U8
#define M_FORMAT_R_F32 M_FORMAT_1_F32
#define M_FORMAT_RG_U8 M_FORMAT_2_U8
#define M_FORMAT_RG_F32 M_FORMAT_2_F32
#define M_FORMAT_RGB_U8 M_FORMAT_3_U8
#define M_FORMAT_RGB_F32 M_FORMAT_3_F32
#define M_FORMAT_RGBA_U8 M_FORMAT_4_U8
#define M_FORMAT_RGBA_F32 M_FORMAT_4_F32


/**
 * @return M_FORMAT__PRIMITIVE_* of the format
 */
#define m_format_primitive(format) ((format) & M_FORMAT__PRIMITIVE_MASK)


/**
 * @return M_FORMAT__PRIMITIVE_TYPE_* like unsigned, signed, float
 */
#define m_format_primitive_type(format) (((format) >> M_FORMAT__PRIMITIVE_TYPE_SHIFT) & M_FORMAT__PRIMITIVE_TYPE_MASK)


/**
 * @return bytesize of a pixel for the given format primitive (channels ignored, see m_format_size)
 */
#define m_format_primitive_size(format) ((format) & M_FORMAT__PRIMITIVE_SIZE_MASK)

/**
 * @return M_FORMAT__CHANNELS_* of the format, which is
 */
#define m_format_channels(format) (((format) >> M_FORMAT__CHANNELS_SHIFT) & M_FORMAT__CHANNELS_MASK)

/**
 * @return byte size for a format "value" so for M_FORMAT_4_32F its 4*4==16
 */
#define m_format_size(format) (m_format_primitive_size(format) * m_format_channels(format))


//
// cast float
//

/**
 * Casts a single (channel==1 if you will) primitive to a float.
 * Cause all are packed, just run it in a for loop to get more like all channels
 */
o_inline float m_format_primitive_cast_to_float(const void *restrict src, ou32 src_format)
{
    switch (m_format_primitive(src_format)) {
        case M_FORMAT__PRIMITIVE_IR8:
            return *(const oi8 *) src;
        case M_FORMAT__PRIMITIVE_UR8:
            return *(const ou8 *) src;
        case M_FORMAT__PRIMITIVE_U8:
            return (*(const ou8 *) src) / (float) ou8_MAX;
        case M_FORMAT__PRIMITIVE_IR16:
            return *(const oi16 *) src;
        case M_FORMAT__PRIMITIVE_UR16:
            return *(const ou16 *) src;
        case M_FORMAT__PRIMITIVE_U16:
            return (*(const ou16 *) src) / (float) ou16_MAX;
        case M_FORMAT__PRIMITIVE_IR32:
            return *(const oi32 *) src;
        case M_FORMAT__PRIMITIVE_UR32:
            return *(const ou32 *) src;
        case M_FORMAT__PRIMITIVE_U32:
            return (*(const ou32 *) src) / (float) ou32_MAX;
        case M_FORMAT__PRIMITIVE_IR64:
            return *(const oi64 *) src;
        case M_FORMAT__PRIMITIVE_UR64:
            return *(const ou64 *) src;
        case M_FORMAT__PRIMITIVE_U64:
            return (*(const ou64 *) src) / (float) ou64_MAX;
        case M_FORMAT__PRIMITIVE_F32:
            return *(const of32 *) src;
        case M_FORMAT__PRIMITIVE_F64:
            return *(const of64 *) src;
        default:
            break;
    }
    assert(0 && "invalid format!");
    return 0;
}

/**
 * Casts a single (channel==1 if you will) primitive from a float.
 * Cause all are packed, just run it in a for loop to get more like all channels
 */
o_inline void m_format_primitive_cast_from_float(void *restrict out_dst, ou32 dst_format, float src)
{
    switch (m_format_primitive(dst_format)) {
        case M_FORMAT__PRIMITIVE_IR8:
            *(oi8 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_UR8:
            *(ou8 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_U8:
            *(ou8 *) out_dst = src*ou8_MAX;
            return;
        case M_FORMAT__PRIMITIVE_IR16:
            *(oi16 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_UR16:
            *(ou16 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_U16:
            *(ou16 *) out_dst = src*ou16_MAX;
            return;
        case M_FORMAT__PRIMITIVE_IR32:
            *(oi32 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_UR32:
            *(ou32 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_U32:
            *(ou32 *) out_dst = (float) ((double) src * (double)ou32_MAX);
            return;
        case M_FORMAT__PRIMITIVE_IR64:
            *(oi64 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_UR64:
            *(ou64 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_U64:
            *(ou64 *) out_dst = (float) ((double) src * (double)ou64_MAX);
            return;
        case M_FORMAT__PRIMITIVE_F32:
            *(of32 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_F64:
            *(of64 *) out_dst = src;
            return;
        default:
            break;
    }
    assert(0 && "invalid format!");
}

/**
 * Calls m_format_primitive_cast_to_float n times for the arrays out_dst and src
 */
o_inline void m_format_primitive_cast_to_float_n(float *restrict out_dst, const void *restrict src, ou32 src_format, int n)
{
    ou32 primitive_size = m_format_primitive_size(src_format);
    const ou8 *restrict src_bytes = src;
    for (int i=0; i<n; i++) {
        out_dst[i] = m_format_primitive_cast_to_float(&src_bytes[i*primitive_size], src_format);
    }
}

/**
 * Calls m_format_primitive_cast_from_float n times for the arrays out_dst and src
 */
o_inline void m_format_primitive_cast_from_float_n(void *restrict out_dst, ou32 dst_format, const float *restrict src, int n)
{
    ou32 primitive_size = m_format_primitive_size(dst_format);
    ou8 *restrict dst_bytes = out_dst;
    for (int i=0; i<n; i++) {
        m_format_primitive_cast_from_float(&dst_bytes[i*primitive_size], dst_format, src[i]);
    }
}

/**
 * Casts the full format value (all channels) to float
 */
o_inline void m_format_cast_to_float(float *restrict out_dst, const void *restrict src, ou32 src_format)
{
    m_format_primitive_cast_to_float_n(out_dst, src, src_format, m_format_channels(src_format));
}
/**
 * Casts the full format value (all channels) from float
 */
o_inline void m_format_cast_from_float(void *restrict out_dst, ou32 dst_format, const float *restrict src)
{
    m_format_primitive_cast_from_float_n(out_dst, dst_format, src, m_format_channels(dst_format));
}


/**
 * Casts up to 4 channels to a vec4, if channels < 4, remaining is set to 0
 */
o_inline m_vec4 m_format_cast_to_vec4(const void *restrict src, ou32 src_format)
{
    if(src_format == M_FORMAT_4_F32) {
        return *(const m_vec4 *) src;
    }
    ou32 channels = o_min(4, m_format_channels(src_format));
    m_vec4 res = {{0}};
    m_format_primitive_cast_to_float_n(res.v, src, src_format, channels);
    return res;
}

/**
 * Casts a vec4 to up to 4 channels vec4, channels > 4 are untouched / ignored
 */
o_inline void m_format_cast_from_vec4(void *restrict out_dst, ou32 dst_format, m_vec4 src)
{
    if(dst_format == M_FORMAT_4_F32) {
        *(m_vec4 *) out_dst = src;
        return;
    }
    ou32 channels = o_min(4, m_format_channels(dst_format));
    m_format_primitive_cast_from_float_n(out_dst, dst_format, src.v, channels);
}


//
// cast double
//

/**
 * Casts a single (channel==1 if you will) primitive to a double.
 * Cause all are packed, just run it in a for loop to get more like all channels
 */
o_inline double m_format_primitive_cast_to_double(const void *restrict src, ou32 src_format)
{
    switch (m_format_primitive(src_format)) {
        case M_FORMAT__PRIMITIVE_IR8:
            return *(const oi8 *) src;
        case M_FORMAT__PRIMITIVE_UR8:
            return *(const ou8 *) src;
        case M_FORMAT__PRIMITIVE_U8:
            return (*(const ou8 *) src) / (double) ou8_MAX;
        case M_FORMAT__PRIMITIVE_IR16:
            return *(const oi16 *) src;
        case M_FORMAT__PRIMITIVE_UR16:
            return *(const ou16 *) src;
        case M_FORMAT__PRIMITIVE_U16:
            return (*(const ou16 *) src) / (double) ou16_MAX;
        case M_FORMAT__PRIMITIVE_IR32:
            return *(const oi32 *) src;
        case M_FORMAT__PRIMITIVE_UR32:
            return *(const ou32 *) src;
        case M_FORMAT__PRIMITIVE_U32:
            return (*(const ou32 *) src) / (double) ou32_MAX;
        case M_FORMAT__PRIMITIVE_IR64:
            return *(const oi64 *) src;
        case M_FORMAT__PRIMITIVE_UR64:
            return *(const ou64 *) src;
        case M_FORMAT__PRIMITIVE_U64:
            return (*(const ou64 *) src) / (double) ou64_MAX;
        case M_FORMAT__PRIMITIVE_F32:
            return *(const of32 *) src;
        case M_FORMAT__PRIMITIVE_F64:
            return *(const of64 *) src;
        default:
            break;
    }
    assert(0 && "invalid format!");
    return 0;
}

/**
 * Casts a single (channel==1 if you will) primitive from a double.
 * Cause all are packed, just run it in a for loop to get more like all channels
 */
o_inline void m_format_primitive_cast_from_double(void *restrict out_dst, ou32 dst_format, double src)
{
    switch (m_format_primitive(dst_format)) {
        case M_FORMAT__PRIMITIVE_IR8:
            *(oi8 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_UR8:
            *(ou8 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_U8:
            *(ou8 *) out_dst = src*ou8_MAX;
            return;
        case M_FORMAT__PRIMITIVE_IR16:
            *(oi16 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_UR16:
            *(ou16 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_U16:
            *(ou16 *) out_dst = src*ou16_MAX;
            return;
        case M_FORMAT__PRIMITIVE_IR32:
            *(oi32 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_UR32:
            *(ou32 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_U32:
            *(ou32 *) out_dst = (double) src * (double)ou32_MAX;
            return;
        case M_FORMAT__PRIMITIVE_IR64:
            *(oi64 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_UR64:
            *(ou64 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_U64:
            *(ou64 *) out_dst = (double) src * (double)ou64_MAX;
            return;
        case M_FORMAT__PRIMITIVE_F32:
            *(of32 *) out_dst = src;
            return;
        case M_FORMAT__PRIMITIVE_F64:
            *(of64 *) out_dst = src;
            return;
        default:
            break;
    }
    assert(0 && "invalid format!");
}

/**
 * Calls m_format_primitive_cast_to_double n times for the arrays out_dst and src
 */
o_inline void m_format_primitive_cast_to_double_n(double *restrict out_dst, const void *restrict src, ou32 src_format, int n)
{
    ou32 primitive_size = m_format_primitive_size(src_format);
    const ou8 *restrict src_bytes = src;
    for (int i=0; i<n; i++) {
        out_dst[i] = m_format_primitive_cast_to_double(&src_bytes[i*primitive_size], src_format);
    }
}

/**
 * Calls m_format_primitive_cast_from_double n times for the arrays out_dst and src
 */
o_inline void m_format_primitive_cast_from_double_n(void *restrict out_dst, ou32 dst_format, const double *restrict src, int n)
{
    ou32 primitive_size = m_format_primitive_size(dst_format);
    ou8 *restrict dst_bytes = out_dst;
    for (int i=0; i<n; i++) {
        m_format_primitive_cast_from_double(&dst_bytes[i*primitive_size], dst_format, src[i]);
    }
}

/**
 * Casts the full format value (all channels) to double
 */
o_inline void m_format_cast_to_double(double *restrict out_dst, const void *restrict src, ou32 src_format)
{
    m_format_primitive_cast_to_double_n(out_dst, src, src_format, m_format_channels(src_format));
}
/**
 * Casts the full format value (all channels) from double
 */
o_inline void m_format_cast_from_double(void *restrict out_dst, ou32 dst_format, const double *restrict src)
{
    m_format_primitive_cast_from_double_n(out_dst, dst_format, src, m_format_channels(dst_format));
}


#endif //M_FORMAT_H
