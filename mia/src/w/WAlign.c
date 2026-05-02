#include "w/WAlign.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "m/vec/vec2.h"
#include "w/WTheme.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"


static bool align_update_children(WAlign *self, vec2 *out_res_size, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    bool reupdate = false;
    vec2 res_size = {{0, 0}};
    
    osize list_len;
    WObj **list = WObj_list_direct(self, &list_len);
    for(osize i=list_len-1; i>=0; i--) {
        const char *option_h = WObj_option(list[i], WAlign_KEY_H);
        const char *option_v = WObj_option(list[i], WAlign_KEY_V);
        
        enum WAlign_mode mode_h = self->align_h;
        enum WAlign_mode mode_v = self->align_v;
        
        for(int i=0; option_h&&i<WAlign_NUM_MODES; i++) {
            if(o_str_equals(option_h, WAlign_VALUES[i])) {
                mode_h = i;
                break;
            }
        }
        for(int i=0; option_v&&i<WAlign_NUM_MODES; i++) {
            if(o_str_equals(option_v, WAlign_VALUES[i])) {
                mode_v = i;
                break;
            }
        }
        
        vec2 prev_size = WObj_gen_padding_size(list[i]);
        
        vec2 room = vec2_sub_v(min_size, prev_size);
        room = vec2_max(room, 0);
        
        vec2 child_lt = lt;
        vec2 child_min_size = vec2_(0);
        
        switch(mode_h) {
        default:
        case WAlign_START:
            // nothing to do
            break;
        case WAlign_CENTER:
            child_lt.x += room.x/2.0f;
            break;
        case WAlign_END:
            child_lt.x += room.x;
            break;
        case WAlign_FIT:
            child_min_size.x = min_size.x;
            break;
        case WAlign_AT_REL:
            child_lt.x += m_clamp(self->at.x, 0, room.x);
            break;
        case WAlign_AT_ABS:
            {
                float rel = self->at.x - lt.x;
                child_lt.x += m_clamp(rel, 0, room.x);
                break;
            }
        }
    
        switch(mode_v) {
        default:
        case WAlign_START:
            // nothing to do
            break;
        case WAlign_CENTER:
            child_lt.y += room.y/2.0f;
            break;
        case WAlign_END:
            child_lt.y += room.y;
            break;
        case WAlign_FIT:
            child_min_size.y = min_size.y;
            break;
        case WAlign_AT_REL:
            child_lt.y += m_clamp(self->at.y, 0, room.y);
            break;
        case WAlign_AT_ABS:
            {
                float rel = self->at.y - lt.y;
                child_lt.y += m_clamp(rel, 0, room.y);
                break;
            }
        }

        if (self->on_unit) {
            child_lt = vec2_floor(child_lt);
            child_min_size = vec2_floor(child_min_size);
        }

        vec2 child_size = WObj_update(list[i], child_lt, child_min_size, enabled, theme, pointer_fn);

        vec2 used_size = vec2_(child_size.x + child_lt.x - lt.x,
                               child_size.y + child_lt.y - lt.y);

        res_size = vec2_max_v(res_size, used_size);
        
        if(!vec2_equals_eps_v(prev_size, child_size, 0.1)) {
            reupdate = true;
        }
    }
    
    o_free(self, list);
    *out_res_size = res_size;
    return reupdate;
}


//
// public
//

WAlign *WAlign_init(oobj obj, oobj parent)
{
    WObj *super = obj;
    WAlign *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WAlign_ID);

    self->align_h = self->align_v = WAlign_START;
    self->on_unit = true;

    // vfuncs
    super->v_update = WAlign__v_update;

    return self;
}

WAlign *WAlign_new_center(oobj parent)
{
    WAlign *self = WAlign_new(parent);
    WAlign_align_set(self, WAlign_CENTER, WAlign_CENTER);
    return self;
}

WAlign *WAlign_new_center_h(oobj parent)
{
    WAlign *self = WAlign_new(parent);
    WAlign_align_h_set(self, WAlign_CENTER);
    return self;
}

WAlign *WAlign_new_center_v(oobj parent)
{
    WAlign *self = WAlign_new(parent);
    WAlign_align_v_set(self, WAlign_CENTER);
    return self;
}

//
// vfuncs
//

vec2 WAlign__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WAlign);
    WAlign *self = obj;
    
    vec2 res_size = vec2_(0);
    
    int prev_num = WTheme_num(theme);
    bool reupdate = align_update_children(self, &res_size, lt, min_size, enabled, theme, pointer_fn);
    
    if(reupdate) {
        WTheme_reset_to(theme, prev_num);
        align_update_children(self, &res_size, lt, min_size, enabled, theme, pointer_fn);
    }
    
    
    return res_size;
}
