#include "w/WView.h"
#include "o/OObj_builder.h"
#include "r/RTex.h"
#include "m/vec/vec2.h"
#include "m/vec/ivec2.h"
#include "m/io/ivec4.h"
#include "u/rect.h"
#include "u/scroll.h"
#include "w/WTheme.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"

static void wview_theme_setup(oobj view)
{
    struct WView__theme_context *C = o_user(view);
    if (C->opt_scroll) {
        enum u_scroll_mode mode = u_scroll_XY;
        if (m_isnan(C->scroll_extra_bounds.x)) {
            mode = u_scroll_X;
            C->scroll_extra_bounds.x = 0;
        }
        if (m_isnan(C->scroll_extra_bounds.y)) {
            mode = u_scroll_Y;
            C->scroll_extra_bounds.y = 0;
        }
        *C->opt_scroll = u_scroll_new(mode, AView_cam(view));
    }
}

static void wview_theme_update(oobj view, oobj tex, float dt)
{
    struct WView__theme_context *C = o_user(view);

    if (C->opt_scroll) {
        // update scroll before WTheme widget stuff, so it's able to use the active unhandled pointer
        u_scroll_update(C->opt_scroll, dt);
    }

    WTheme_update_full_tex(C->theme, C->wobj, tex);

    if (C->opt_scroll) {
        vec2 gui_size = WObj_gen_size(C->wobj);
        vec4 limits = u_rect_new(-C->scroll_extra_bounds.v0,
            -C->scroll_extra_bounds.v1,
            gui_size.x + C->scroll_extra_bounds.v0 + C->scroll_extra_bounds.v2,
            gui_size.y + C->scroll_extra_bounds.v1 + C->scroll_extra_bounds.v3);
        if (C->opt_scroll->mode == u_scroll_Y) {
            limits.v0 = limits.v2 = 0;
        }
        if (C->opt_scroll->mode == u_scroll_X) {
            limits.v1 = limits.v3 = 0;
        }
        C->opt_scroll->cam_limits_rect = limits;
    }
}

static void wview_theme_render(oobj view, oobj tex, float dt)
{
    struct WView__theme_context *C = o_user(view);
    WTheme_render(C->theme, tex);
}


//
// public
//

WView *WView_init(oobj obj, oobj parent, oobj view, bool move_view)
{
    WObj *super = obj;
    WView *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WView_ID);

    self->view = view;
    if(move_view) {
        o_move(view, self);
    }

    // vfuncs
    super->v_update = WView__v_update;

    return self;
}


WView *WView_new_theme(oobj parent, oobj theme, bool move_theme, oobj wobj, bool move_wobj,
    const vec4 *opt_scrollable_extra_bounds)
{
    oobj view = AView_new(parent, wview_theme_setup, wview_theme_update, wview_theme_render);
    struct WView__theme_context *C = o_user_new0(view, *C, 1);
    C->theme = theme;
    C->wobj = wobj;

    if (opt_scrollable_extra_bounds) {
        C->opt_scroll = o_new0(view, *C->opt_scroll, 1);
        C->scroll_extra_bounds = *opt_scrollable_extra_bounds;
    }
    
    if(move_theme) {
        o_move(theme, view);
    }
    if(move_wobj) {
        o_move(wobj, view);
    }
    
    return WView_new(parent, view, true);
}

WView *WView_new_theme_tiny(oobj parent, oobj wobj, bool move_wobj,
    const vec4 *opt_scrollable_extra_bounds)
{
    oobj theme = WTheme_new_tiny(parent);
    return WView_new_theme(parent, theme, true, wobj, move_wobj, opt_scrollable_extra_bounds);
}

//
// vfuncs
//

vec2 WView__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WView);
    WView *self = obj;

    vec2 child_size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);
    vec2 size = vec2_max_v(child_size, min_size);

    // fixed size
    if (self->super.fixed_size.x >= 0) {
        size.x = self->super.fixed_size.x;
    }
    if (self->super.fixed_size.y >= 0) {
        size.y = self->super.fixed_size.y;
    }

    self->rect = vec4_(lt.x, lt.y, size.x, size.y);

    return size;
}

AView *WView_view_set(oobj obj, oobj view, bool del_old)
{
    OObj_assert(obj, WView);
    WView *self = obj;
    if(del_old) {
        o_del(self->view);
    }
    self->view = view;
    return self->view;
}

ivec4 WView_viewport(oobj obj, oobj tex)
{
    ivec2 off = RTex_viewport(tex).xy;
    ivec4 vp = r_proj_rect_to_viewport(RTex_proj(tex), WView_rect(obj));
    vp.xy = ivec2_add_v(vp.xy, off);
    return vp;
}


void WView_update(oobj obj, oobj tex)
{
    OObj_assert(obj, WView);
    WView *self = obj;

    if(self->hidden || self->view_update) {
        return;
    }
    
    AView_update(self->view, tex, WView_viewport(self, tex));
    
    self->view_update = true;
}

void WView_render(oobj obj, oobj tex)
{
    OObj_assert(obj, WView);
    WView *self = obj;

    if(self->hidden) {
        return;
    }
    if(!self->view_update) {
        AView_update(self->view, tex, WView_viewport(self, tex));
    }
    AView_render(self->view, tex);
    self->view_update = false;
}
