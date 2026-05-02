#include "w/WTooltip.h"
#include "o/OObj_builder.h"
#include "o/OList.h"
#include "o/timer.h"
#include "o/str.h"
#include "m/vec/vec2.h"
#include "a/common.h"
#include "w/WTip.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"

#define TOOLTIP_DELAY_TIME 1.5f
#define TOOLTIP_DELAY_ADD_TIME 3.5f
#define TOOLTIP_FADE_IN_TIME 0.5f
#define TOOLTIP_FADE_OUT_TIME 0.5f
#define TOOLTIP_MOVEMENT_MAX 16.0f


static void tooltip_auto_gathering(WTooltip *self, struct a_pointer pointer)
{
    vec2 pos = pointer.pos.xy;
    oobj wobj = NULL;
    osize num = OList_num(self->gui_list);
    
    // if ignoring, no meed to call WObj_pick and reset delay to fade out
    if(self->gather_ignore) {
        self->gather_delay = TOOLTIP_DELAY_TIME;
        num = 0;
    }
    
    for (osize i=0; i<num; i++) {
        WObj *gui = OList_at(self->gui_list, i);
        OObj_assert(gui, WObj);
        wobj = WObj_pick(gui, WObj, NULL, pos, true, oi32_MAX).o;
    }
    // special case to ignore hidden picks, such as from a WPane
    if (wobj && o_str_equals(WObj_tooltip(wobj), "")) {
        wobj = NULL;
    }

    if (self->gather_current != wobj) {
        self->gather_current = wobj;
        if (self->gather_current) {
            const char *tooltip = WObj_tooltip(self->gather_current);
            assert(tooltip);
            WTip_set(self->tip, tooltip);
            self->gather_pos_start = pos;
            WTip_pos_set(self->tip, pos);
            WTip_offset_set(self->tip, pointer.down? 28.0f : 8.0f);
            self->gather_delay_add = 0;
        }
    }

    // NOTE: self->gather_current may be NULL or some OOBj (not WObj!!!)

    float dt = a_dt();
    float dist = vec2_distance(pos, self->gather_pos_start);
    
    // stop ignoring if either hover stopped or moved a bit
    // start ignoring on release
    // touch will have a fixed "hover" state after release
    // thats why we have that ignore flag
    if(pointer.down || dist>=TOOLTIP_MOVEMENT_MAX) {
        self->gather_ignore = false;
    }
    if(a_pointer_released(pointer)) {
        self->gather_ignore = true;
    }

    if (pointer.down && self->gather_delay_add>0.0f) {
        // Additional delay, if moved in the same tooltip wobj while down
        // Within that time the pos needs to match, else resetting
        self->gather_delay_add -= dt;
        if (dist > TOOLTIP_MOVEMENT_MAX) {
            self->gather_pos_start = pos;
            self->gather_delay = TOOLTIP_DELAY_TIME;
            self->gather_delay_add = TOOLTIP_DELAY_ADD_TIME;
        }
    } else if (self->gather_delay>=0.0f) {
        // Default delay.
        // Within that time the pos needs to match, else resetting
        self->gather_delay -= dt;
        if (dist > TOOLTIP_MOVEMENT_MAX) {
            self->gather_pos_start = pos;
            self->gather_delay = TOOLTIP_DELAY_TIME;
        }
    }

    if (!self->gather_current 
            || self->gather_delay>=0.0f) {
        // fade out
        float sub_alpha = dt / TOOLTIP_FADE_OUT_TIME;
        float alpha = WTip_alpha(self->tip) - sub_alpha;
        if (alpha < 0.0f) {
            alpha = 0.0f;
            WTip_pos_set(self->tip, pos);
            if (!self->gather_current) {
                self->gather_delay = TOOLTIP_DELAY_TIME;
            }
        }
        WTip_alpha_set(self->tip, alpha);
    } else {
        // fade in
        float add_alpha = dt / TOOLTIP_FADE_IN_TIME;
        float alpha = WTip_alpha(self->tip) + add_alpha;
        if (alpha > 1.0f) {
            alpha = 1.0f;
        }
        if (dist > TOOLTIP_MOVEMENT_MAX || a_pointer_released(pointer)) {
            self->gather_pos_start = pos;
            self->gather_delay = TOOLTIP_DELAY_TIME;
            self->gather_delay_add = TOOLTIP_DELAY_ADD_TIME;
        }
        WTip_alpha_set(self->tip, alpha);
    }
}

//
// public
//

WTooltip *WTooltip_init(oobj obj, oobj parent)
{
    WObj *super = obj;
    WTooltip *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WTooltip_ID);

    self->auto_gather = true;
    
    self->container = OObj_new(self);

    self->tip = WTip_new(self->container);

    // defaults to {self}
    self->gui_list = OList_new(self, (void**) &self, 1);

    // vfuncs
    super->v_update = WTooltip__v_update;

    return self;
}

//
// vfuncs
//

vec2 WTooltip__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WTooltip);
    WTooltip *self = obj;

    if (self->auto_gather) {
        tooltip_auto_gathering(self, pointer_fn(0, 0));
    }

    bool hidden = self->hidden || WTip_alpha(self->tip) <= 0.0f;

    if (!hidden) {
        // pre run update on the pane to calc sizes AND to set it on top of the children
        // and ignore allocations on next update below
        WObj__ignore_alloc_boxes_set(self->tip, false);
        WObj_update(self->tip, lt, vec2_(0), enabled, theme, pointer_fn);
    }

    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);
    vec2 size = vec2_max_v(child_size, min_size);
    vec2 tip_size = vec2_(0);

    if (!hidden) {
        WObj__ignore_alloc_boxes_set(self->tip, true);
        tip_size = WObj_update(self->tip, lt, size, enabled, theme, pointer_fn);
    }

    return vec2_max_v(size, tip_size);
}

//
// object functions
//
