#include "w/WTextShadow.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "w/WTheme.h"
#include "w/WStyle.h"
#include "m/vec/vec2.h"
#include "u/rect.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"

#include <ctype.h>




struct WTextShadow__layout_user {
    oobj text_self;
    oobj theme;
    struct r_box *boxes;
    vec2 lt;
};

static vec2 WTextShadow__layout_uv_shadow(struct r_textlayout *layout, char c)
{
    struct WTextShadow__layout_user *user = layout->user;
    return u_atlas_pos(WTheme_atlas(user->theme), WTheme_char_shadow_idx(c));
}

static void WTextShadow__layout_generic_shadow(struct r_textlayout *layout, osize idx, vec2 xy, vec2 uv)
{
    struct WTextShadow__layout_user *user = layout->user;
    WText *super = user->text_self;
    WTextShadow *self = user->text_self;
    osize shadow_idx = idx*2;
    user->boxes[shadow_idx].uv_rect = vec4_(uv.x, uv.y, super->char_size.x+1, super->char_size.y+1);
    user->boxes[shadow_idx].rect = vec4_(user->lt.x + xy.x, user->lt.y + xy.y, layout->size.x, layout->size.y);
    user->boxes[shadow_idx].fx = self->shadow_color_enabled_applied;
}

static vec2 WTextShadow__layout_uv(struct r_textlayout *layout, char c)
{
    struct WTextShadow__layout_user *user = layout->user;
    return u_atlas_pos(WTheme_atlas(user->theme), WTheme_char_idx(c));
}

static void WTextShadow__layout_generic(struct r_textlayout *layout, osize idx, vec2 xy, vec2 uv)
{
    struct WTextShadow__layout_user *user = layout->user;
    WText *super = user->text_self;
    osize base_idx = idx*2+1;
    user->boxes[base_idx].uv_rect = vec4_(uv.x, uv.y, super->char_size.x, super->char_size.y);
    user->boxes[base_idx].rect = vec4_(user->lt.x + xy.x, user->lt.y + xy.y, layout->size.x, layout->size.y);
    user->boxes[base_idx].fx = super->color_enabled_applied;
}


//
// public
//

WTextShadow *WTextShadow_init(oobj obj, oobj parent, const char *text)
{
    WText *super = obj;
    WTextShadow *self = obj;
    o_clear(self, sizeof *self, 1);

    WText_init(obj, parent, text);

    OObj_id_set(self, WTextShadow_ID);


    // vfuncs
    super->super.v_update = WTextShadow__v_update;
    super->super.v_style_apply = WTextShadow__v_style_apply;
    
    // apply style
    WObj_style_apply(self);

    return self;
}

//
// vfuncs
//

vec2 WTextShadow__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WTextShadow);
    WText *super = obj;
    WTextShadow *self = obj;


    //
    // mostly a copy of WText__v_update, but added boxes * 2 shadow stuff...
    //

    super->color_enabled_applied = WObj_enabled_apply_color(self, enabled, super->color);
    self->shadow_color_enabled_applied = WObj_enabled_apply_color(self, enabled, self->shadow_color);

    WText_char_size_update(self, theme);
    vec2 char_size = vec2_scale_v(super->char_size, super->char_scale);
    vec2 char_offset = vec2_scale_v(super->char_offset, super->char_scale);
    vec2 char_shadow_size = vec2_scale_v(vec2_add(super->char_size, 1), super->char_scale);


    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);
    vec2 size = vec2_max_v(child_size, min_size);

    vec2 fixed_size = WObj_fixed_size(self);
    if(fixed_size.x>=0) {
        size.x = fixed_size.x;
    }
    if(fixed_size.y>=0) {
        size.y = fixed_size.y;
    }

    struct r_box *boxes = WObj__alloc_boxes(self, theme, super->num*2);

    // setup textlayout for shadow
    struct r_textlayout layout = r_textlayout_new(char_shadow_size, char_offset, WTextShadow__layout_uv_shadow);
    layout.casing = (enum r_textlayout_casing) super->casing;
    layout.wrap = (enum r_textlayout_wrap) super->wrap;
    layout.align = (enum r_textlayout_align) super->align;
    layout.max_size = size;
    struct WTextShadow__layout_user user = {self, theme, boxes, lt};
    layout.user = &user;

    // run textlayout
    super->text_size = r_textlayout_set_generic(&layout, super->text, WTextShadow__layout_generic_shadow);
    super->cols_rows = ivec2_(layout.cols, layout.rows);

    // edit 'n run textlayout for default text
    layout.v_char_uv = WTextShadow__layout_uv;
    layout.size = char_size;
    r_textlayout_set_generic(&layout, super->text, WTextShadow__layout_generic);

    return vec2_max_v(super->text_size, child_size);
}

void WTextShadow__v_style_apply(oobj obj)
{
    WText__v_style_apply(obj);
    
    OObj_assert(obj, WTextShadow);
    WTextShadow *self = obj;
    oobj style = WObj_style(self);
    
    self->shadow_color = WStyle_textshadow_shadow_color(style);
}
