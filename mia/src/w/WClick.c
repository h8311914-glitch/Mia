#include "w/WClick.h"
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

WClick *WClick_init(oobj obj, oobj parent)
{
    WObj *super = obj;
    WClick *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WClick_ID);

    // vfuncs
    super->v_update = WClick__v_update;

    return self;
}

//
// vfuncs
//

vec2 WClick__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WClick);
    WClick *self = obj;


    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);
    vec2 size = vec2_max_v(child_size, min_size);

    self->pressed = self->clicked = false;

    if (!enabled) {
        self->down = false;
        return size;
    }

    self->pointer = pointer_fn(0, 0);

    vec2 pp = self->pointer.pos.xy;
    pp.x = pp.x - lt.x;
    pp.y = pp.y - lt.y;
    self->pos = pp;
    if (u_rect_contains(u_rect_new_lt(0, 0, m_2(size)), pp)) {
        if (a_pointer_pressed(self->pointer)) {
            self->down = true;
            self->pressed = true;
            if(self->pressed_event) {
                self->pressed_event(self);
            }
        } else if(self->down && a_pointer_released(self->pointer)) {
            self->clicked = true;
            self->down = false;
            if(self->clicked_event) {
                self->clicked_event(self);
            }
        }
    } else {
        self->down = false;
    }
    if (!self->pointer.down) {
        self->down = false;
    }
    
    return size;
}
