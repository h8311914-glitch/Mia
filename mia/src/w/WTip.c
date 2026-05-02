#include "w/WTip.h"
#include "o/OObj_builder.h"
#include "o/OList.h"
#include "m/vec/vec2.h"
#include "w/WPane.h"
#include "w/WText.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"

//
// public
//

WTip *WTip_init(oobj obj, oobj parent)
{
    WObj *super = obj;
    WTip *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WTip_ID);

    self->lt_on_unit = true;
    self->offset = 16.0f;
    self->alpha = 0.0;

    self->pane = WPane_new(self);
    WPane_color_set(self->pane, vec4_(0.8, 0.6, 0.2, 1.0));
    self->text = WText_new(self->pane, "");
    WText_wrap_size_set(self->text, vec2_(128, -1));
    WText_casing_set(self->text, WText_casing_UPPER);
    WText_wrap_set(self->text, WText_wrap_WORD);
    WText_align_set(self->text, WText_align_LEFT);
    WText_color_set(self->text, R_BLACK);

    // vfuncs
    super->v_update = WTip__v_update;

    return self;
}

//
// vfuncs
//

vec2 WTip__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WTip);
    WTip *self = obj;

    vec4 color = WPane_color(self->pane);
    color.a = self->alpha;
    WPane_color_set(self->pane, color);

    color = WText_color(self->text);
    color.a = self->alpha;
    WText_color_set(self->text, color);

    vec2 prev_size = WObj_gen_padding_size(self->pane);

    vec2 rem_room = vec2_max(vec2_sub_v(min_size,   prev_size), 0);

    vec2 rel_pos = vec2_sub_v(self->pos, lt);

    vec2 rel_pane;
    rel_pane.x = rel_pos.x - prev_size.x / 2;
    rel_pane.y = rel_pos.y - self->offset - prev_size.y;
    if (rel_pane.y < 0) {
        // fallback below:
        rel_pane.y = rel_pos.y + self->offset;
    }
    rel_pane.y = m_clamp(rel_pane.y, 0, rem_room.y);
    rel_pane.x = m_clamp(rel_pane.x, 0, rem_room.x);

    vec2 pane_lt = vec2_add_v(lt, rel_pane);
    if(self->lt_on_unit) {
        pane_lt = vec2_floor(pane_lt);
    }

    vec2 pane_size = WObj_update(self->pane, pane_lt, vec2_(0), enabled, theme, pointer_fn);

    vec2 pane_res_size = vec2_add_v(rel_pane, pane_size);
    return vec2_max_v(pane_res_size, min_size);
}

//
// object functions
//


void WTip_set(oobj obj, const char *text)
{
    WText_text_set(WTip_text(obj), text);
}
