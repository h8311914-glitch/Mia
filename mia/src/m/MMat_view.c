#include "m/MMat_view.h"
#include "m/int.h"


#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"



//
// Windows, rotate and mirror as views
//

static const void *window_data_min(const void *data, m_ivec2 size, m_ivec2 stride)
{
    int a = 0;
    int b = (size.x-1) * stride.x;
    int c = (size.y-1) * stride.y;
    int d = b+c;
    return (const obyte*) data + o_min(a, o_min(b, o_min(c, d)));
}
static const void *window_data_max(const void *data, m_ivec2 size, m_ivec2 stride)
{
    int a = 0;
    int b = (size.x-1) * stride.x;
    int c = (size.y-1) * stride.y;
    int d = b+c;
    return (const obyte*) data + o_max(a, o_max(b, o_max(c, d)));
}

MMat *MMat_window(oobj obj, void *data, m_ivec2 size, m_ivec2 stride)
{
    MMat_assert(obj);
    MMat *self = obj;
    data = o_or(data, self->data);
    if (size.x==0) {
        size = self->size;
    }
    if (stride.x==0) {
        stride = self->stride;
    }
    assert(window_data_min(self->data, self->size, self->stride) <= window_data_min(data, size, stride));
    assert(window_data_max(self->data, self->size, self->stride) >= window_data_max(data, size, stride));
    return MMat_new_ex(obj, data, self->data_root,false, self->format, size, stride);
}

MMat *MMat_window_rect(oobj obj, m_ivec4 rect, bool clamp)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(clamp || (rect.left >= 0
        && rect.top >= 0));
    if (rect.left < 0) {
        rect.width += rect.left;
        rect.left = 0;
    }
    if (rect.top < 0) {
        rect.height += rect.top;
        rect.top = 0;
    }
    ivec2 max_size = ivec2_sub_v(self->size, rect.xy);
    assert(clamp || (rect.width >=0 && rect.width <= max_size.x
        && rect.height>0 && rect.height <= max_size.y));
    rect.zw = ivec2_clamp_v(rect.zw, ivec2_(0), max_size);
    
    if(rect.width <= 0 || rect.height <= 0) {
        rect = MMat_rect_int(self, 0, 0);
    }
    
    obyte *restrict data = MMat_at_raw(self, rect.left, rect.top);
    return MMat_window(self, data, rect.zw, ivec2_(0));
}

MMat *MMat_transpose(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    ivec2 stride = {{self->stride.y, self->stride.x}};
    return MMat_window(self, NULL, ivec2_(0), stride);
}


MMat *MMat_mirror_v(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    ivec2 stride = {{-self->stride.x, self->stride.y}};
    obyte *restrict data = MMat_at_raw(self, self->size.x-1, 0);
    return MMat_window(self, data, ivec2_(0), stride);
}


MMat *MMat_mirror_h(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    ivec2 stride = {{self->stride.x, -self->stride.y}};
    obyte *restrict data = MMat_at_raw(self, 0, self->size.y-1);
    return MMat_window(self, data, ivec2_(0), stride);
}


MMat *MMat_rotate_left(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    ivec2 size = {{self->size.y, self->size.x}};
    ivec2 stride = {{self->stride.y, -self->stride.x}};
    obyte *restrict data = MMat_at_raw(self, self->size.x-1, 0);
    return MMat_window(self, data, size, stride);
}


MMat *MMat_rotate_right(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    ivec2 size = {{self->size.y, self->size.x}};
    ivec2 stride = {{-self->stride.y, self->stride.x}};
    obyte *restrict data = MMat_at_raw(self, 0, self->size.y-1);
    return MMat_window(self, data, size, stride);
}


MMat *MMat_rotate_180(oobj obj)
{
    MMat_assert(obj);
    MMat *self = obj;
    ivec2 stride = {{-self->stride.x, -self->stride.y}};
    obyte *restrict data = MMat_at_raw(self, self->size.x-1, self->size.y-1);
    return MMat_window(self, data, ivec2_(0), stride);
}
