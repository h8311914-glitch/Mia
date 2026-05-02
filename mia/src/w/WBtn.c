#include "w/WBtn.h"
#include "o/OObj_builder.h"
#include "o/timer.h"
#include "a/app.h"
#include "w/WTheme.h"
#include "w/WStyle.h"
#include "w/WTextShadow.h"
#include "m/vec/vec2.h"
#include "u/rect.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"


bool pointer_clicked(WBtn *self)
{
    bool in_r = self->pointer.down
            && u_rect_contains(WObj_gen_rect(self), self->pointer.pos.xy);
    
    bool press = in_r;
    if(self->slideable) {
        press = press && self->pointer.down;
    } else {
        press = press && a_pointer_pressed(self->pointer);
    }
             

    bool clicked = a_pointer_released(self->pointer)
                   && WBtn_down(self)
                   && u_rect_contains(WObj_gen_rect(self), self->pointer_prev_pos);

    if(press) {
        WBtn_down_set(self, true);
    }
    if(!in_r || clicked) {
        WBtn_down_set(self, false);
    }

    self->auto_action = clicked;
    return clicked;
}


bool pointer_pressed(WBtn *self)
{
    bool press = u_rect_contains(WObj_gen_rect(self), self->pointer.pos.xy);
    
    if(self->slideable) {
        press = press && self->pointer.down;
    } else {
        press = press && a_pointer_pressed(self->pointer);
    }
                 
    if(press) {
        WBtn_down_set(self, true);
    }

    self->auto_action = press;
    return press;
}

bool pointer_toggled(WBtn *self)
{
    bool toggle = a_pointer_pressed(self->pointer)
                  && u_rect_contains(WObj_gen_rect(self), self->pointer.pos.xy);
    if(toggle) {
        WBtn_down_set(self, !WBtn_down(self));

    }
    self->auto_action = toggle;
    return toggle;
}

//
// public
//

WBtn *WBtn_init(oobj obj, oobj parent)
{
    WObj *super = obj;
    WBtn *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WBtn_ID);
    
    self->down_timer = 0;
    self->long_pressed_time = 0.5;
    
    // vfuncs
    super->v_update = WBtn__v_update;
    super->v_style_apply = WBtn__v_style_apply;
    
    // apply style
    WObj_style_apply(self);

    return self;
}


WBtn *WBtn_new_text(oobj parent, const char *text, oobj *opt_out_text)
{
    WBtn *self = WBtn_new(parent);
    oobj label = WText_new(self, text);
    o_opt_set(opt_out_text, label);
    return self;
}

WBtn *WBtn_new_text_shadow(oobj parent, const char *text, oobj *opt_out_text_shadow)
{
    WBtn *self = WBtn_new(parent);
    oobj label = WTextShadow_new(self, text);
    o_opt_set(opt_out_text_shadow, label);
    return self;
}

//
// vfuncs
//

vec2 WBtn__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WBtn);
    WBtn *self = obj;

    struct u_atlas atlas = WTheme_atlas(theme);

    int btn_idx;
    switch(self->style) {
        default:
        case WBtn_FLAT:
            btn_idx = WTheme_BTN_FLAT;
            break;
        case WBtn_FLAT_ROUND:
            btn_idx = WTheme_BTN_FLAT_ROUND;
            break;
            
        case WBtn_DEF:
            btn_idx = WTheme_BTN_DEF;
            break;
        case WBtn_DEF_ROUND:
            btn_idx = WTheme_BTN_DEF_ROUND;
            break;
        case WBtn_DEF_DUAL:
            btn_idx = WTheme_BTN_DEF_DUAL;
            break;
        case WBtn_DEF_DUAL_L:
            btn_idx = WTheme_BTN_DEF_DUAL_L;
            break;
        case WBtn_DEF_DUAL_R:
            btn_idx = WTheme_BTN_DEF_DUAL_R;
            break;
        case WBtn_DEF_ROUND_DUAL_L:
            btn_idx = WTheme_BTN_DEF_ROUND_DUAL_L;
            break;
        case WBtn_DEF_ROUND_DUAL_R:
            btn_idx = WTheme_BTN_DEF_ROUND_DUAL_R;
            break;
            
        case WBtn_BIG:
            btn_idx = WTheme_BTN_BIG;
            break;
        case WBtn_BIG_ROUND:
            btn_idx = WTheme_BTN_BIG_ROUND;
            break;
        case WBtn_BIG_DUAL:
            btn_idx = WTheme_BTN_BIG_DUAL;
            break;
        case WBtn_BIG_DUAL_L:
            btn_idx = WTheme_BTN_BIG_DUAL_L;
            break;
        case WBtn_BIG_DUAL_R:
            btn_idx = WTheme_BTN_BIG_DUAL_R;
            break;
        case WBtn_BIG_ROUND_DUAL_L:
            btn_idx = WTheme_BTN_BIG_ROUND_DUAL_L;
            break;
        case WBtn_BIG_ROUND_DUAL_R:
            btn_idx = WTheme_BTN_BIG_ROUND_DUAL_R;
            break;
            
        case WBtn_RADIO:
            btn_idx = WTheme_BTN_RADIO;
            break;
        case WBtn_CHECK:
            btn_idx = WTheme_BTN_CHECK;
            break;
        case WBtn_SWITCH:
            btn_idx = WTheme_BTN_SWITCH;
            break;
    }

    int sprite = btn_idx;
    if(self->mode == 1) {
        sprite += WTheme_BTN__SPRITES;
    }

    vec4 color = WObj_enabled_apply_color(self, enabled, self->color);

    vec2 size_corner_lt = u_atlas_size(atlas, sprite);
    vec2 size_corner_rb = u_atlas_size(atlas, sprite + 8);
    vec2 frame_size = vec2_add_v(size_corner_lt, size_corner_rb);


    vec2 children_lt = vec2_(lt.x + size_corner_lt.x, lt.y + size_corner_lt.y);
    vec2 min_inner_size = vec2_sub_v(min_size, frame_size);

    vec2 inner_size = WObj__update_children_stacked(self, children_lt, min_inner_size, enabled, theme, pointer_fn);

    inner_size = vec2_max_v(inner_size, min_inner_size);
    inner_size = vec2_max(inner_size, 0);

    // theme the boxes
    struct r_box *boxes = WObj__alloc_boxes(self, theme, 9);

    vec2 offsets[9] = {
            {{0, 0}},
            {{size_corner_lt.x, 0}},
            {{size_corner_lt.x + inner_size.x, 0}},
            {{0, size_corner_lt.y}},
            {{size_corner_lt.x, size_corner_lt.y}},
            {{size_corner_lt.x + inner_size.x, size_corner_lt.y}},
            {{0, size_corner_lt.y + inner_size.y}},
            {{size_corner_lt.x, size_corner_lt.y + inner_size.y}},
            {{size_corner_lt.x + inner_size.x, size_corner_lt.y + inner_size.y}},
    };
    vec2 sizes[9] = {
            {{size_corner_lt.x, size_corner_lt.y}},
            {{inner_size.x, size_corner_lt.y}},
            {{size_corner_rb.x, size_corner_lt.y}},
            {{size_corner_lt.x, inner_size.y}},
            {{inner_size.x, inner_size.y}},
            {{size_corner_rb.x, inner_size.y}},
            {{size_corner_lt.x, size_corner_rb.y}},
            {{inner_size.x, size_corner_rb.y}},
            {{size_corner_rb.x, size_corner_rb.y}}
    };

    for (int i = 0; i < 9; i++) {
        struct r_box *b = &boxes[i];
        b->uv_rect = u_atlas_rect(atlas, sprite + i);
        b->rect = u_rect_new_lt(lt.x + offsets[i].x,
                                lt.y + offsets[i].y,
                                m_2(sizes[i]));
        b->fx = color;
    }

    vec2 full_size = vec2_add_v(inner_size, frame_size);
    vec4 rect = u_rect_new_lt(lt.x, lt.y, full_size.x, full_size.y);

    self->auto_action = false;

    if (!enabled) {
        return full_size;
    }

    //
    // pointer stuff
    //

    // grab pointers
    self->pointer = pointer_fn(0, 0);

    vec2 pp = self->pointer.pos.xy;
    if (u_rect_contains(rect, pp)) {
        a_pointer_handled(0, 0);
    }


    switch(self->auto_mode) {
        default:
        case WBtn_auto_OFF:
            break;
        case WBtn_auto_CLICKED:
            if(pointer_clicked(self) && self->auto_event) {
                self->auto_event(self);
            }
            break;
        case WBtn_auto_PRESSED:
            if(pointer_pressed(self) && self->auto_event) {
                self->auto_event(self);
            }
            break;
        case WBtn_auto_TOGGLED:
            if(pointer_toggled(self) && self->auto_event) {
                self->auto_event(self);
            }
            break;
        case WBtn_auto_LONG_PRESSED:
            if (WBtn_long_pressed(self) && self->auto_event) {
                self->auto_event(self);
            }
            break;
        case WBtn_auto_CLICKED_AND_LONG_PRESSED:
            if ((pointer_clicked(self) || WBtn_long_pressed(self)) && self->auto_event) {
                self->auto_event(self);
            }
            break;
    }

    // needed for pointer_clicked
    self->pointer_prev_pos = pp;
    
    // down time for WBtn_long_pressed, etc.
    // updated if >not< down, so counts down time
    if(!WBtn_down(self)) {
        self->down_timer = a_timer();
    }

    return full_size;
}

void WBtn__v_style_apply(oobj obj) 
{
    WObj__v_style_apply(obj);
    
    OObj_assert(obj, WBtn);
    WBtn *self = obj;
    oobj style = WObj_style(self);

    self->color = WStyle_btn_color(style);
    self->style = WStyle_btn_style(style);
}


void WBtn_set_group(oobj obj, oobj *group, osize group_num, bool pressed)
{
    if (group_num<0) {
        group_num = o_list_num(group);
    }
    for (osize i = 0; i < group_num; i++) {
        if (group[i] == obj) {
            continue;
        }
        if (OObj_check(group[i], WBtn)) {
            WBtn_down_set(group[i], pressed);
        }
    }
}

ou64 WBtn_down_timer_reset(oobj obj)
{
    OObj_assert(obj, WBtn);
    WBtn *self = obj;
    self->down_timer = a_timer();
    return self->down_timer;
}

bool WBtn_clicked(oobj obj)
{
    OObj_assert(obj, WBtn);
    WBtn *self = obj;
    if(self->auto_mode == WBtn_auto_OFF) {
        pointer_clicked(self);
    }
    return self->auto_action;
}

bool WBtn_pressed(oobj obj)
{
    OObj_assert(obj, WBtn);
    WBtn *self = obj;
    if(self->auto_mode == WBtn_auto_OFF) {
        pointer_pressed(self);
    }
    return self->auto_action;
}

bool WBtn_toggled(oobj obj)
{
    OObj_assert(obj, WBtn);
    WBtn *self = obj;
    if(self->auto_mode == WBtn_auto_OFF) {
        pointer_toggled(self);
    }
    return self->auto_action;
}

bool WBtn_long_pressed(oobj obj)
{
    OObj_assert(obj, WBtn);
    WBtn *self = obj;
    if (!WBtn_down(self)) {
        return false;
    }
    double down_time = o_timer_diff_s(self->down_timer, a_timer());
    if(down_time >= self->long_pressed_time && down_time <self->long_pressed_time+999) {
        self->down_timer += o_timer_freq_s()*1000;
        return true;
    }
    return false;
}
