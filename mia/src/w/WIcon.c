#include "w/WIcon.h"
#include "o/OObj_builder.h"
#include "w/WTheme.h"
#include "w/WStyle.h"
#include "m/vec/vec2.h"
#include "u/rect.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"

//
// public
//

WIcon *WIcon_init(oobj obj, oobj parent, enum WTheme_indices icon_idx)
{
    WObj *super = obj;
    WIcon *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WIcon_ID);

    self->icon_idx = icon_idx;

    // vfuncs
    super->v_update = WIcon__v_update;
    super->v_style_apply = WIcon__v_style_apply;

    // apply style
    WObj_style_apply(self);

    return self;
}

//
// vfuncs
//

vec2 WIcon__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WIcon);
    WIcon *self = obj;

    struct u_atlas atlas = WTheme_atlas(theme);


    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);

    vec2 icon_size = u_atlas_size(atlas, self->icon_idx);

    struct r_box *box = WObj__alloc_boxes(self, theme, 1);
    box->uv_rect = u_atlas_rect(atlas, self->icon_idx);
    box->rect = u_rect_new_lt(m_2(lt), m_2(icon_size));
    box->fx = WObj_enabled_apply_color(self, enabled, self->color);
    
    return vec2_max_v(child_size, icon_size);
}

void WIcon__v_style_apply(oobj obj)
{
    WObj__v_style_apply(obj);

    OObj_assert(obj, WIcon);
    WIcon *self = obj;
    oobj style = WObj_style(self);

    self->color = WStyle_icon_color(style);
}
