#include "w/WRestrict.h"
#include "o/OObj_builder.h"
#include "u/rect.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"

//
// public
//

WRestrict *WRestrict_init(oobj obj, oobj parent)
{
    WObj *super = obj;
    WRestrict *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WRestrict_ID);

    self->pointer_modes[0] = WRestrict_FULL_HISTORY;

    // vfuncs
    super->v_update = WRestrict__v_update;

    return self;
}

//
// vfuncs
//

vec2 WRestrict__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WRestrict);
    WRestrict *self = obj;

    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);
    vec2 size = vec2_max_v(child_size, min_size);
    vec2 fixed_size = WObj_fixed_size(self);
    if(fixed_size.x>=0) {
        size.x = fixed_size.x;
    }
    if(fixed_size.y>=0) {
        size.y = fixed_size.y;
    }


    vec4 rect = vec4_(lt.x, lt.y, size.x, size.y);
    for (int i=0; i<a_pointer_MAX; i++) {
        if (self->pointer_modes[i] == WRestrict_OFF) {
            continue;
        }
        struct a_pointer p = pointer_fn(i, 0);
        if (!u_rect_contains(rect, p.pos.xy)) {
            continue;
        }
        a_pointer_handled(i, self->pointer_modes[i] == WRestrict_CURRENT? 0 : -1);
    }
    
    return size;
}

void WRestrict_pointer_mode_set(oobj obj, int idx, enum WRestrict_pointer_mode mode)
{
    OObj_assert(obj, WRestrict);
    WRestrict *self = obj;

    if (idx < 0) {
        for (int i=0; i<a_pointer_MAX; i++) {
            WRestrict_pointer_mode_set(self, i, mode);
        }
        return;
    }
    assert(idx>=0 && idx<a_pointer_MAX);
    self->pointer_modes[idx] = mode;
}
