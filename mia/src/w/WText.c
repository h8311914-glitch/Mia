#include "w/WText.h"
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
#include <stdlib.h>


struct WText__layout_user {
    oobj text_self;
    oobj theme;
    struct r_box *boxes;
    vec2 lt;
};

static vec2 WText__layout_uv(struct r_textlayout *layout, char c)
{
    struct WText__layout_user *user = layout->user;
    return u_atlas_pos(WTheme_atlas(user->theme), WTheme_char_idx(c));
}

static void WText__layout_generic(struct r_textlayout *layout, osize idx, vec2 xy, vec2 uv)
{
    struct WText__layout_user *user = layout->user;
    WText *self = user->text_self;
    user->boxes[idx].uv_rect = vec4_(uv.x, uv.y, self->char_size.x, self->char_size.y);
    user->boxes[idx].rect = vec4_(user->lt.x + xy.x, user->lt.y + xy.y, layout->size.x, layout->size.y);
    user->boxes[idx].fx = self->color_enabled_applied;
}

//
// public
//

WText *WText_init(oobj obj, oobj parent, const char *text)
{
    WObj *super = obj;
    WText *self = obj;
    o_clear(self, sizeof *self, 1);

    text = o_or(text, "");

    WObj_init(obj, parent);
    OObj_id_set(self, WText_ID);

    self->casing = WText_casing_UNCHANGED;
    self->wrap = WText_wrap_OFF;
    self->align = WText_align_LEFT;

    self->char_size = vec2_(-1);
    self->char_offset = vec2_(-1);
    self->char_scale = vec2_(1.0);
    self->wrap_size = vec2_(-1);

    WText_text_set(self, text);

    // vfuncs
    super->v_update = WText__v_update;
    super->v_style_apply = WText__v_style_apply;
    
    // apply style
    WObj_style_apply(self);

    return self;
}

//
// vfuncs
//

vec2 WText__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WText);
    WText *self = obj;

    self->color_enabled_applied = WObj_enabled_apply_color(self, enabled, self->color);

    WText_char_size_update(self, theme);
    vec2 char_size = vec2_scale_v(self->char_size, self->char_scale);
    vec2 char_offset = vec2_scale_v(self->char_offset, self->char_scale);

    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);
    vec2 size = vec2_max_v(child_size, min_size);
    if (self->wrap_size.x >= -0.01) {
        size.x = self->wrap_size.x;
    }
    if (self->wrap_size.y >= -0.01) {
        size.y = self->wrap_size.y;
    }

    struct r_box *boxes = WObj__alloc_boxes(self, theme, self->num);

    // setup textlayout
    struct r_textlayout layout = r_textlayout_new(char_size, char_offset, WText__layout_uv);
    layout.casing = (enum r_textlayout_casing) self->casing;
    layout.wrap = (enum r_textlayout_wrap) self->wrap;
    layout.align = (enum r_textlayout_align) self->align;
    layout.max_size = size;
    struct WText__layout_user user = {self, theme, boxes, lt};
    layout.user = &user;

    // run textlayout
    self->text_size = r_textlayout_set_generic(&layout, self->text, WText__layout_generic);
    self->cols_rows = ivec2_(layout.cols, layout.rows);

    return vec2_max_v(self->text_size, child_size);
}

void WText__v_style_apply(oobj obj)
{
    WObj__v_style_apply(obj);
    
    OObj_assert(obj, WText);
    WText *self = obj;
    oobj style = WObj_style(self);
    
    self->color = WStyle_text_color(style);
}

void WText_char_size_update(oobj obj, oobj theme)
{
    OObj_assert(obj, WText);
    WText *self = obj;

    // init char offset + size
    if(self->char_size.x < 0) {
        self->char_size = u_atlas_size(WTheme_atlas(theme), WTheme_char_idx('0'));
        self->char_offset = vec2_add(self->char_size, 1);
    }
}

const char *WText_text_set(oobj obj, const char *text)
{
    OObj_assert(obj, WText);
    WText *self = obj;
    o_free(self, self->text);
    self->text = o_str_clone(self, text);
    self->num = (int) o_strlen(text);
    return self->text;
}
