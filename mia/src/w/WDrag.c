#include "w/WDrag.h"
#include "o/OObj_builder.h"
#include "o/timer.h"
#include "w/WTheme.h"
#include "w/WStyle.h"
#include "m/sca/dbl.h"
#include "m/vec/vec2.h"
#include "m/vec/vec3.h"
#include "m/vec/ivecn.h"
#include "u/rect.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include <a/app.h>

#include "o/log.h"


static const float DRAG_MIX = 0.9;

#define DRAG_DBL_CLICK_TIME 0.33
#define DRAG_DBL_CLICK_DIST 4.0

//
// public
//

WDrag *WDrag_init(oobj obj, oobj parent)
{
    WObj *super = obj;
    WDrag *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WDrag_ID);


    self->progress = vec2_(0);
    self->progress_raw = vec2_(0);
    self->mode = WDrag_X;
    self->steps = vec2_(-1);
    self->on_unit = true;
    self->drag_time = vec2_(1.0);

    // vfuncs
    super->v_update = WDrag__v_update;
    super->v_style_apply = WDrag__v_style_apply;
    
    // apply style
    WObj_style_apply(self);

    return self;
}

//
// vfuncs
//

vec2 WDrag__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WDrag);
    WDrag *self = obj;
   
    assert(self->mode>=0 && self->mode<WDrag_NUM_MODES);
    
     
    int arrows = self->mode == WDrag_XY? 4 : 2;
    int progress_boxes = self->mode == WDrag_XY? 2:1;

    vec4 bg_color = WObj_enabled_apply_color(self, enabled, self->bg_color);
    vec4 progress_color_x = WObj_enabled_apply_color(self, enabled, self->progress_color_x);
    vec4 progress_color_y = WObj_enabled_apply_color(self, enabled, self->progress_color_y);
    vec4 arrow_color = WObj_enabled_apply_color(self, enabled, self->arrow_color);

    struct u_atlas atlas = WTheme_atlas(theme);

    int arrow_sprites[4];
    {
        int *it = arrow_sprites;
        if(self->mode == WDrag_X || self->mode == WDrag_XY) {
            *it++ = WTheme_ICON_DIR_LEFT;
            *it++ = WTheme_ICON_DIR_RIGHT;
        }
        if(self->mode == WDrag_Y || self->mode == WDrag_XY) {
            *it++ = WTheme_ICON_DIR_UP;
            *it++ = WTheme_ICON_DIR_DOWN;
        }
    }
    
    if(!self->pressed) {
        ivecn_set(arrow_sprites, WTheme_ICON_DOTS, 4);
    }

    vec2 size_arrow = u_atlas_size(atlas, arrow_sprites[0]);
    
    vec2 children_lt = lt;
    if(self->mode == WDrag_X || self->mode == WDrag_XY) {
        children_lt.x += size_arrow.x;
    }
    if(self->mode == WDrag_Y || self->mode == WDrag_XY) {
        children_lt.y += size_arrow.y;
    }

    vec2 children_size = WObj__update_children_stacked(self, children_lt, vec2_(0), enabled, theme, pointer_fn);

    vec2 full_size = children_size;
    if(self->mode == WDrag_X || self->mode == WDrag_XY) {
        full_size.x += size_arrow.x*2;
        full_size.y = o_max(full_size.y, size_arrow.y);
    }
    if(self->mode == WDrag_Y || self->mode == WDrag_XY) {
        full_size.x = o_max(full_size.x, size_arrow.x);
        full_size.y += size_arrow.y*2;
    }
    
    full_size = vec2_max_v(full_size, min_size);

    vec2 arrow_lts[4];
    {
        vec2 *it = arrow_lts;
        if(self->mode == WDrag_X || self->mode == WDrag_XY) {
            *it = lt;
            it->y += m_floor(full_size.y/2 - size_arrow.y/2);
            it++;
            
            *it = lt;
            it->y = it[-1].y;
            it->x += m_floor(full_size.x - size_arrow.x);
            it++;
        }
        
        if(self->mode == WDrag_Y || self->mode == WDrag_XY) {
            *it = lt;
            it->x += m_floor(full_size.x/2 - size_arrow.x/2);
            it++;
            
            *it = lt;
            it->x = it[-1].x;
            it->y += m_floor(full_size.y - size_arrow.y);
            it++;
        }
    }

    vec2 progress_lt = lt;
    vec2 progress_size = full_size;
    
    progress_size = vec2_scale_v(progress_size, self->progress);

    if(self->on_unit) {
        progress_size = vec2_round(progress_size);
    }

    
    progress_lt.y += (full_size.y - progress_size.y);

    // theme the badge boxes
    // bg + progress + arrows
    struct r_box *boxes = WObj__alloc_boxes(self, theme, 1+progress_boxes+arrows);
    struct r_box *b;
    int qi = 0;

    // background
    b = &boxes[qi++];
    b->uv_rect = u_atlas_rect(atlas, WTheme_WHITE);
    b->rect = u_rect_new_lt(m_2(lt), m_2(full_size));
    b->fx = bg_color;

    // progress
    if(self->mode == WDrag_X || self->mode == WDrag_XY) {
        b = &boxes[qi++];
        b->uv_rect = u_atlas_rect(atlas, WTheme_WHITE);
        b->rect = u_rect_new_lt(lt.x, lt.y, progress_size.x, full_size.y);
        b->fx = progress_color_x;
    }
    if(self->mode == WDrag_Y || self->mode == WDrag_XY) {
        b = &boxes[qi++];
        b->uv_rect = u_atlas_rect(atlas, WTheme_WHITE);
        b->rect = u_rect_new_lt(lt.x, progress_lt.y, full_size.x, progress_size.y);
        b->fx = progress_color_y;
    }
   

    // arrows
    for(int i=0;i<arrows; i++) {
        b = &boxes[qi++];
        b->uv_rect = u_atlas_rect(atlas, arrow_sprites[i]);
        b->rect = u_rect_new_lt(m_2(arrow_lts[i]), m_2(size_arrow));
        b->fx = self->pressed? self->arrow_pressed_color : arrow_color;
    }


    if (!enabled) {
        self->pressed = false;
        return full_size;
    }

    //
    // pointer stuff
    //

    vec4 rect = u_rect_new_lt(m_2(lt), m_2(full_size));

    // grab pointer
    struct a_pointer pointer = pointer_fn(0, 0);

    if(!pointer.down) {
        self->pressed = false;
    }

    vec2 pp = pointer.pos.xy;
    if(u_rect_contains(rect, pp)) {
        if(a_pointer_pressed(pointer)) {
            self->pressed = true;
            
            // dbl click to set that progress
            ou64 time = a_timer();
            if(o_timer_diff_s(self->dbl_click_time, time) <= DRAG_DBL_CLICK_TIME
                    && vec2_distance(self->dbl_click_pos, pointer.pos.xy) <= DRAG_DBL_CLICK_DIST) {
                 vec2 pos = {{pp.x - lt.x, pp.y - lt.y}};
                 pos = vec2_div_v(pos, full_size);
                 pos.y = 1.0f - pos.y;
                 WDrag_progress_raw_set(self, pos);
            } else {
                self->dbl_click_time = time;
                self->dbl_click_pos = pointer.pos.xy;
            }
        }
    }

    // o_timer is not stable enough between frames (cause the gl pipeline may work more or less in between)
    ou64 time = a_timer();

    if(!self->pressed) {
        self->prev_time = ou64_MAX;
        self->prev_speed = vec2_(0);
    }
    if(self->pressed) {
        vec2 pos = {{pp.x - lt.x, pp.y - lt.y}};
        pos = vec2_div_v(pos, full_size);

        if(time>self->prev_time) {
            vec2 diff_pos = vec2_sub_v(pos, self->prev_pos);
            float diff_time = (float) o_timer_diff_s(self->prev_time, time);
            vec2 drag_time = vec2_div_v(vec2_(diff_time), self->drag_time);
            vec2 speed = vec2_div_v(diff_pos, drag_time);
            speed = self->prev_speed = vec2_mix(speed, self->prev_speed, DRAG_MIX);

            speed.x = m_sign(speed.x) * m_min(2.0f, m_pow(speed.x, 2.0f));
            speed.y = m_sign(speed.y) * m_min(2.0f, m_pow(speed.y, 2.0f));

            vec2 add = vec2_scale(speed, diff_time);

            vec2 progress_raw = self->progress_raw;

            if(self->mode == WDrag_X || self->mode == WDrag_XY) {
                progress_raw.x = m_clamp(progress_raw.x + add.x, 0.0f, 1.0f);
            }
            if(self->mode == WDrag_Y || self->mode == WDrag_XY) {
                progress_raw.y = m_clamp(progress_raw.y - add.y, 0.0f, 1.0f);
            }

            // sets progress stepped
            WDrag_progress_raw_set(self, progress_raw);

            if(self->change_event) {
                self->change_event(self);
            }
        }

        self->prev_pos = pos;
        self->prev_time = time;


        a_pointer_handled(0, 0);
    }

    return full_size;
}

void WDrag__v_style_apply(oobj obj)
{
    WObj__v_style_apply(obj);
    
    OObj_assert(obj, WDrag);
    WDrag *self = obj;
    oobj style = WObj_style(self);
    
    self->bg_color = WStyle_drag_bg_color(style);
    self->progress_color_x = WStyle_drag_progress_color_x(style);
    self->progress_color_y = WStyle_drag_progress_color_y(style);
    self->arrow_color = WStyle_drag_arrow_color(style);
    self->arrow_pressed_color = WStyle_drag_arrow_pressed_color(style);
}

vec2 WDrag_progress_raw_set(oobj obj, vec2 set)
{
    OObj_assert(obj, WDrag);
    WDrag *self = obj;

    self->progress_raw = set;
    self->progress = self->progress_raw;
    if(self->steps.x>0) {
        self->progress.x -= m_mod(self->progress.x, self->steps.x);
    }
    if(self->steps.y>0) {
        self->progress.y -= m_mod(self->progress.y, self->steps.y);
    }
    return set;
}
