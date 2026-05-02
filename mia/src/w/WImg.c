#include "w/WImg.h"
#include "o/OObj_builder.h"
#include "w/WTheme.h"
#include "m/vec/vec2.h"
#include "u/rect.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"

//
// public
//

WImg *WImg_init(oobj obj, oobj parent, vec4 uv_rect)
{
    WObj *super = obj;
    WImg *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WImg_ID);

    self->uv_rect = uv_rect;
    self->color = R_WHITE;

    // vfuncs
    super->v_update = WImg__v_update;

    return self;
}

//
// vfuncs
//

vec2 WImg__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WImg);
    WImg *self = obj;

    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);

    vec2 size = WImg_size(self);

    struct r_box *box = WObj__alloc_boxes(self, theme, 1);
    box->uv_rect = self->uv_rect;
    box->rect = u_rect_new_lt(m_2(lt), m_2(size));
    box->fx = WObj_enabled_apply_color(self, enabled, self->color);
    
    return vec2_max_v(child_size, size);
}

vec2 WImg_size(oobj obj)
{
    OObj_assert(obj, WImg);
    WImg *self = obj;
    vec2 size = self->uv_rect.zw;
    vec2 fixed = WObj_fixed_size(self);
    if(fixed.x>=0) {
        size.x = fixed.x;
    }
    if(fixed.y>=0) {
        size.y = fixed.y;
    }
    return size;
}

