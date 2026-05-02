#include "m/MMat_channel.h"
#include "m/int.h"


#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"


//
// Channels and merging
//

MMat *MMat_channel(oobj obj, int channel)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(m_format_channels(self->format) > channel);
    osize channel_byte_offset = m_format_primitive_size(self->format) * channel;
    ou32 single_channel_format = M_FORMAT_X(1, (self->format & M_FORMAT__PRIMITIVE_MASK));
    obyte *restrict data = self->data;
    data += channel_byte_offset;
    return MMat_new_ex(obj, data, self->data_root,false, single_channel_format,
        self->size, self->stride);
}


void MMat_channel_set(oobj obj, int obj_channel, oobj set, int set_channel)
{
    MMat_assert(obj);
    MMat *dst = obj;
    MMat_assert(set);
    MMat *src = obj;
    assert(m_format_channels(dst->format) > obj_channel);
    assert(m_format_channels(src->format) > set_channel);
    assert(m_format_primitive(dst->format) == m_format_primitive(src->format));
    osize num = MMat_num(dst);
    assert(num == MMat_num(src) && "needs same size");

    // both asserted to be the same primitive
    osize prim_size = m_format_primitive_size(dst->format);
    osize dst_offset = prim_size * obj_channel;
    osize src_offset = prim_size * set_channel;

    for (osize i = 0; i < num; i++) {
        obyte *restrict dst_data = MMat_at_idx(dst, i);
        dst_data += dst_offset;
        const obyte *src_data = MMat_at_idx(src, i);
        src_data += src_offset;
        o_memcpy(dst_data, src_data, prim_size, 1);
    }
}

void MMat_channel_merge_into(const oobj *channels, oobj into, const float *opt_add)
{
    MMat_assert(into);
    MMat *into_mat = into;
    ou32 into_format = into_mat->format;
    osize prim_size = m_format_primitive_size(into_format);
    ou32 n = m_format_channels(into_format);

    osize num = into_mat->size.x * into_mat->size.y;

    oobj container = OObj_new(into);
    const float *add;
    if (opt_add) {
        add = opt_add;
    } else {
        add = o_new0(container, float, n);
    }

    for (int c = 0; c < n; c++) {
        if (channels[c]) {
            ou32 src_format = MMat_format(channels[c]);
            assert(MMat_num(channels[c]) == num);
            for (osize i = 0; i < num; i++) {
                obyte *restrict dst_data = MMat_at_idx(into_mat, i);
                dst_data += c * prim_size;
                const obyte *src_data = MMat_at_idx(channels[c], i);
                float tmp = m_format_primitive_cast_to_float(src_data, src_format);
                tmp += add[c];
                m_format_primitive_cast_from_float(dst_data, into_format, tmp);
            }
        } else {
            // channels[c] is NULL
            for (osize i = 0; i < num; i++) {
                obyte *restrict dst_data = MMat_at_idx(into_mat, i);
                dst_data += c * prim_size;
                m_format_primitive_cast_from_float(dst_data, into_format, add[c]);
            }
        }
    }

    o_del(container);
}

struct oobj_opt MMat_channel_merge_try(oobj parent, ou32 format, const oobj *channels, const float *opt_add)
{
    osize prim_size = m_format_primitive_size(format);
    ou32 n = m_format_channels(format);
    ivec2 size = ivec2_(-1);
    for (int c = 0; c < n; c++) {
        if (channels[c]) {
            size = MMat_size_int(channels[c]);
            break;
        }
    }
    assert(size.x > 0 && size.y > 0);
    MMat *self = MMat_new_ex(parent, NULL, NULL, false, format, size, ivec2_(0));
    if (!MMat_valid(self)) {
        return oobj_opt(NULL);
    }
    MMat_channel_merge_into(channels, self, opt_add);
    return oobj_opt(self);
}

MMat *MMat_channel_merge_rgba(oobj r, oobj g, oobj b, oobj opt_a)
{
    ou32 format_prim = m_format_primitive(MMat_format(r));
    ou32 format = M_FORMAT_X(4, format_prim);
    return MMat_channel_merge(r, format, (oobj[]){r, g, b, opt_a}, (float[]){0, 0, 0, opt_a ? 0 : 1});
}


void MMat_channel_swizzle_into(oobj obj, oobj into, const char *swizzle)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat_assert(into);
    MMat *into_mat = into;
    
    int channels = o_strlen(swizzle);
    assert(channels > 0 && channels <= 4);
    assert(channels <= m_format_channels(into_mat->format));

    oobj merge_channels[4] = {0};
    float merge_add[4] = {0};

    for (int i = 0; i < channels; i++) {
        char c = swizzle[i];
        switch (c) {
            case 'R':
            case 'r':
                merge_channels[i] = MMat_channel(self, 0);
                break;
            case 'G':
            case 'g':
                merge_channels[i] = MMat_channel(self, 1);
                break;
            case 'B':
            case 'b':
                merge_channels[i] = MMat_channel(self, 2);
                break;
            case 'A':
            case 'a':
                merge_channels[i] = MMat_channel(self, 3);
                break;
            case '0':
                // nothing to do
                break;
            case '1':
                merge_add[i] = 1.0;
                break;
            default:
                assert(0 && "invalid swizzle character");
                break;
        }
    }

    MMat_channel_merge_into(merge_channels, into_mat, merge_add);
}

struct oobj_opt MMat_channel_swizzle_try(oobj obj, const char *swizzle, ou32 format)
{
    MMat_assert(obj);
    MMat *self = obj;
    int channels = o_strlen(swizzle);
    assert(channels > 0 && channels <= 4);
    
    if (format == M_FORMAT_KEEP) {
        format = self->format;
    }
    format = M_FORMAT_X(channels, format);

    struct oobj_opt res = MMat_new_try(obj, NULL, m_2(self->size), format);
    if(!res.o) {
        return res;
    }
    MMat_channel_swizzle_into(self, res.o, swizzle);
    return res;
}

