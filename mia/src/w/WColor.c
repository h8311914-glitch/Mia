#include "w/WColor.h"
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

WColor *WColor_init(oobj obj, oobj parent, vec4 color)
{
    WObj *super = obj;
    WColor *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WColor_ID);

    self->color = color;

    // vfuncs
    super->v_update = WColor__v_update;

    return self;
}

//
// vfuncs
//

vec2 WColor__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WColor);
    WColor *self = obj;


    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);
    vec2 size = vec2_max_v(child_size, min_size);

    vec2 fixed_size = WObj_fixed_size(self);
    if(fixed_size.x>=0) {
        size.x = fixed_size.x;
    }
    if(fixed_size.y>=0) {
        size.y = fixed_size.y;
    }

    struct r_box *boxes = WObj__alloc_boxes(self, theme, 1);

    boxes->uv_rect = u_atlas_rect(WTheme_atlas(theme), WTheme_WHITE);
    boxes->rect = u_rect_new_lt(m_2(lt), m_2(size));
    boxes->fx = WObj_enabled_apply_color(self, enabled, self->color);
    
    return size;
}
