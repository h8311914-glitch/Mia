#include "m/MMat_ex.h"
#include "o/OObj_builder.h"
#include "m/flt.h"
#include "m/int.h"
#include "m/byte.h"
#include "m/types/byte.h"
#include <stdlib.h> // qsort


#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"


static void clone_into(void *restrict dst_data, ivec2 dst_stride,
                       const void *restrict src_data, ivec2 src_stride, ou32 format,
                       ivec2 size)
{
    const obyte *restrict src = src_data;
    obyte *restrict dst = dst_data;
    osize format_size = m_format_size(format);
    if (ivec2_equals_v(src_stride, dst_stride)) {
        // fast copy all packed
        osize num = size.x * size.y;
        o_memcpy(dst, src, format_size, num);
    } else {
        // slow copy each value
        for (osize r = 0; r < size.y; r++) {
            for (osize c = 0; c < size.x; c++) {
                const obyte *restrict s = &src[c * src_stride.x + r * src_stride.y];
                obyte *restrict d = &dst[c * dst_stride.x + r * dst_stride.y];
                o_memcpy(d, s, format_size, 1);
            }
        }
    }
}




//
// public
//

MMat *MMat_init_ex(oobj obj, oobj parent, void *opt_data, const void *opt_data_root, bool clone_data, ou32 format,
                   m_ivec2 size, m_ivec2 opt_stride)
{
    MMat *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, MMat_ID);

    assert(size.x > 0 && size.y > 0);

    self->format = format;
    self->size = size;

    osize format_size = m_format_size(format);
    assert(format_size>0);
    ivec2 packed_stride = ivec2_(format_size, format_size * self->size.x);
    osize num = self->size.x * self->size.y;


    if (opt_stride.x == 0 || opt_stride.y == 0) {
        // stride is not set, use packed
        opt_stride = packed_stride;
    }


    if (opt_data && !clone_data) {
        // only a reference, just set values not copy em
        self->data = opt_data;
        self->data_root = o_or(opt_data_root, opt_data);
        self->owning = false;
        self->stride = opt_stride;
    } else {
        // we need to allocate the buffer
        self->data = o_alloc_try(self, format_size, num);
        self->data_root = self->data;
        self->owning = true;
        self->stride = packed_stride;
    }

    if (self->data && opt_data && clone_data) {
        // we need to clone
        clone_into(self->data, self->stride, opt_data, opt_stride, self->format, self->size);
    }

    // vfuncs
    self->super.v_op_num = MMat__v_op_num;
    self->super.v_op_at = MMat__v_op_at;

    return self;
}


struct oobj_opt MMat_new_try(oobj parent, const void *opt_data, int cols, int rows, ou32 format)
{
    // its fine to cast away const here, cause its only read internally (clone_data=true)
    MMat *self = MMat_new_ex(parent, (void *) opt_data, NULL, true, format,
                             ivec2_(cols, rows), ivec2_(0));
    if (!MMat_valid(self)) {
        o_del(self);
        return oobj_opt(NULL);
    }
    return oobj_opt(self);
}


struct oobj_opt MMat_new_0_try(oobj parent, int cols, int rows, ou32 format)
{
    MMat *self = MMat_new_try(parent, NULL, cols, rows, format).o;
    if (!self) {
        return oobj_opt(NULL);
    }
    o_clear(self->data, 1, MMat_data_size(self));
    return oobj_opt(self);
}



//
// virtual implementations:
//

osize MMat__v_op_num(oobj obj)
{
    return MMat_num(obj);
}

void *MMat__v_op_at(oobj obj, osize idx)
{
    return MMat_at_idx(obj, idx);
}

//
// object functions
//


bool MMat_format_raw_set(oobj obj, bool set)
{
    MMat_assert(obj);
    MMat *self = obj;
    ou32 prim_type = m_format_primitive_type(self->format);
    if (set && prim_type == M_FORMAT__PRIMITIVE_TYPE_U) {
        prim_type = M_FORMAT__PRIMITIVE_TYPE_UR;
    } else if (!set && prim_type == M_FORMAT__PRIMITIVE_TYPE_UR) {
        prim_type = M_FORMAT__PRIMITIVE_TYPE_U;
    } else {
        return prim_type == M_FORMAT__PRIMITIVE_TYPE_UR || prim_type == M_FORMAT__PRIMITIVE_TYPE_IR;
    }
    ou32 prim_size = m_format_primitive_size(self->format);
    ou32 channels = m_format_channels(self->format);
    self->format = M_FORMAT_X_FULL(channels, prim_type, prim_size);
    return set;
}


void *MMat_data_packed(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    obyte *restrict data = o_alloc_try(self, MMat_data_size(self), 1);
    if (!data) {
        return NULL;
    }
    if (MMat_packed(self)) {
        // fast path, just a memcpy
        o_memcpy(data, self->data, MMat_data_size(self), 1);
    } else {
        ivec2 packed_stride = MMat_packed_stride(self);
        clone_into(data, packed_stride, self->data, self->stride, self->format, self->size);
    }
    return data;
}

void MMat_data_packed_set(oobj obj, const void *restrict data)
{
    MMat_assert(obj);
    MMat *self = obj;
    if (!data) {
        return;
    }
    if (MMat_packed(self)) {
        // fast path, just a memcpy
        o_memcpy(self->data, data, MMat_data_size(self), 1);
    } else {
        ivec2 packed_stride = MMat_packed_stride(self);
        clone_into(self->data, self->stride, data, packed_stride, self->format, self->size);
    }
}

void MMat_owning_make_try(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    if (self->owning) {
        return;
    }
    self->owning = true;
    self->data = MMat_data_packed(self);
    osize format_size = m_format_size(self->format);
    self->stride = ivec2_(format_size, format_size * self->size.x);
}


bool MMat_equals(oobj obj, oobj mat)
{
    MMat_assert(obj);
    MMat *a = obj;
    MMat_assert(mat);
    MMat *b = mat;
    if(a->format != b->format) {
        return false;
    }
    if(!ivec2_equals_v(a->size, b->size)) {
        return false;
    }
    int format_size = m_format_size(a->format);
    ivec2 size = a->size;
    
    // fast path, equals all packed
    if (MMat_packed(a) && ivec2_equals_v(a->stride, b->stride)) {
        osize num = size.x * size.y;
        return o_equals(a->data, b->data, format_size, num);
    }
    
    // fallback check each
    {
        for (osize r = 0; r < size.x; r++) {
            for (osize c = 0; c < size.y; c++) {
                const obyte *restrict s = MMat_at_raw(a, c, r);
                const obyte *restrict d = MMat_at_raw(b, c, r);
                if(!o_equals(s, d, format_size, 1)) {
                    return false;
                }
            }
        }
        
        return true;
    }
}

static _Thread_local o__compare_fn mmat__L_sort_cmp_fn;

static int mmat_sort_incides_cmp(void *mat, const void *a_void, const void *b_void)
{
    osize a_idx = *(osize*) a_void;
    osize b_idx = *(osize*) b_void;
    const void *a = MMat_at_idx_raw(mat, a_idx);
    const void *b = MMat_at_idx_raw(mat, b_idx);
    return mmat__L_sort_cmp_fn(mat, a, b);
}

bool MMat_sort_try(oobj obj, o__compare_fn comperator)
{
    MMat_assert(obj);
    MMat *self = obj;
    osize format_size = m_format_size(self->format);
    osize num = MMat_num(self);
    if (num <= 1) {
        return true;
    }
    if (MMat_packed(self)) {
        // fast path, works directly on data
        o_qsort(self->data, format_size, num, self, comperator);
        return true;
    }
    // data is not packed, so we create a packed clone, sort that and copy the data back into obj
    MMat *clone = MMat_clone_try(self).o;
    if (!clone) {
        return false;
    }
    assert(MMat_packed(clone));
    o_qsort(clone->data, format_size, num, self, comperator);
    for (osize i=0; i<num; i++) {
        const void *src = MMat_at_idx_raw(clone, i);
        void *dst = MMat_at_idx_raw(self, i);
        o_memcpy(dst, src, format_size, 1);
    }
    o_del(clone);
    return true;
}

osize MMat_search(oobj obj, const void *search_element, o__compare_fn comperator)
{
    MMat_assert(obj);
    MMat *self = obj;
    osize num = MMat_num(self);
    for (osize i = 0; i < num; i++) {
        if (comperator(self, search_element, MMat_at_idx_raw(self, i)) == 0) {
            return i;
        }
    }
    return -1;
}


//
// Copies
//


void MMat_clone_into(oobj obj, oobj into)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat_assert(into);
    MMat *clone = into;
    assert(self->format == clone->format);
    ivec2 size = ivec2_min_v(self->size, clone->size);
    clone_into(clone->data, clone->stride, self->data, self->stride, self->format, size);
}


struct oobj_opt MMat_clone_try(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat *clone = MMat_new_ex(obj, self->data, NULL, true, self->format, self->size, self->stride);
    if (!MMat_valid(clone)) {
        o_del(clone);
        return oobj_opt(NULL);
    }
    return oobj_opt(clone);
}

void MMat_cast_into(oobj obj, oobj into)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat_assert(into);
    MMat *cast = into;
    osize num = o_min(MMat_num(self), MMat_num(cast));
    ou32 self_format = self->format;
    ou32 cast_format = cast->format;
    int channels = o_min(m_format_channels(self_format), m_format_channels(cast_format));
    channels = o_min(channels, M_FORMAT_MAX_SIZE);
    double tmp[M_FORMAT_MAX_SIZE];
    for (osize i = 0; i < num; i++) {
        const void *restrict src_data = MMat_at_idx_raw(self, i);
        void *restrict dst_data = MMat_at_idx_raw(cast, i);
        m_format_primitive_cast_to_double_n(tmp, src_data, self_format, channels);
        m_format_primitive_cast_from_double_n(dst_data, cast_format, tmp, channels);
    }
}

struct oobj_opt MMat_cast_try(oobj obj, ou32 format)
{
    MMat_assert(obj);
    MMat *self = obj;
    if (self->format == format || format == M_FORMAT_KEEP) {
        return MMat_clone_try(obj);
    }
    ou32 channels = m_format_channels(format);
    assert(channels == m_format_channels(self->format) && channels <= M_FORMAT_MAX_SIZE);
    MMat *cast = MMat_new_ex(obj, NULL, NULL, true, format, self->size, ivec2_(0));
    if (!MMat_valid(cast)) {
        o_del(cast);
        return oobj_opt(NULL);
    }
    MMat_cast_into(self, cast);
    return oobj_opt(cast);
}