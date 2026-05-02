#include "w/WWindow.h"
#include "o/OObj_builder.h"
#include "m/vec/vec2.h"
#include "u/rect.h"
#include "u/btn.h"
#include "w/WTheme.h"
#include "w/WStyle.h"
#include "w/WPane.h"
#include "w/WAlign.h"
#include "w/WText.h"
#include "w/WTextShadow.h"
#include "o/timer.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"

#include "o/log.h"


static const vec4 HIDE_BTN_COLOR = {{0.66, 0.66, 0.66, 0.66}};

static const float DBL_TAP_HIDE_TIME = 0.33;
static const float HIDE_BLOCK_TIME = 0.33;

//
// public
//

WWindow *WWindow_init(oobj obj, oobj parent, vec2 header_min_size)
{
    WObj *super = obj;
    WWindow *self = obj;
    o_clear(self, sizeof *self, 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WWindow_ID);

    if (header_min_size.x < 0) {
        header_min_size.x = 32;
    }
    if (header_min_size.y < 0) {
        header_min_size.y = 10;
    }

    self->lt_on_unit = true;
    self->hideable = true;
    self->draggable = true;
    self->header_min_size = header_min_size;
    self->body_offset = vec2_(0, -1);

    self->container = OObj_new(self);
    self->header = WPane_new(self->container);
    self->body = WPane_new(self->container);
    
    self->btn_hide = u_btn_new_box_atlas(NULL, NULL, WTheme_ICON_WND_MIN, WTheme_ICON_WND_MAX);
    self->btn_hide.state_up.fx = self->btn_hide.state_down.fx = HIDE_BTN_COLOR;

    // vfuncs
    super->v_update = WWindow__v_update;
    super->v_list = WWindow__v_list;
    super->v_style_apply = WWindow__v_style_apply;
    
    // apply style
    WObj_style_apply(self);

    return self;
}

WWindow *WWindow_new_title(oobj parent, vec2 header_min_size,
        const char *title, oobj *opt_out_title)
{
    oobj wnd = WWindow_new(parent, header_min_size);
    oobj txt = WTextShadow_new(WWindow_header(wnd), title);
    // padding right for the hide btn
    WObj_padding_ref(txt)->v2=6;
    o_opt_set(opt_out_title, txt);
    return wnd;
}

//
// vfuncs
//


vec2 WWindow__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WWindow);
    WWindow *self = obj;
    
    
    // active gets true if pointer was activr before updating children 
    // and it was in the resulting used size
    bool pointer_active = pointer_fn(0, 0).down;
    self->active = false;
    
    
    vec2 field_size = vec2_max_v(self->children_size_prev, min_size);

    //
    // clamp lt
    //

    self->lt = vec2_max(self->lt, 0);

    if (self->hidden) {
        self->lt.x = o_min(self->lt.x, field_size.x
                                       - WObj_gen_size(self->header).x);
        self->lt.y = o_min(self->lt.y, field_size.y
                                       - WObj_gen_size(self->header).y);
    } else {
        self->lt.x = o_min(self->lt.x, field_size.x
                                       - WObj_gen_size(self->body).x
                                       - self->body_offset.x);
        self->lt.y = o_min(self->lt.y, field_size.y
                                       - WObj_gen_size(self->body).y
                                       - self->header_min_size.y
                                       - self->body_offset.y);
    }

    vec2 drag_lt = self->lt;
    if (self->lt_on_unit) {
        drag_lt = vec2_floor(drag_lt);
    }

    //
    // update
    //
    
    vec2 header_lt, header_size;
    vec2 body_lt, body_size;
    int theme_num = WTheme_num(theme);
    for(int reupdate=0;reupdate<2;reupdate++) {

        // allocated before udpates, so that it appears on top
        WObj__alloc_boxes(self, theme, 1);

        vec2 header_min_size = self->header_min_size;
        header_lt = vec2_(lt.x + drag_lt.x, lt.y + drag_lt.y);
    
        if (!self->hidden) {
            header_min_size.x = o_max(header_min_size.x, self->body_size_prev.x);
        }
        WObj_min_size_set(self->header, header_min_size);
        header_size = WObj_update(self->header, header_lt, vec2_(0), true, theme, pointer_fn);
    
    
        body_lt = vec2_(header_lt.x + self->body_offset.x,
                         header_lt.y + header_size.y + self->body_offset.y);

        WObj_min_size_set(self->body, vec2_(header_min_size.x, 0));
        body_size = vec2_(0);
        if (!self->hidden) {
            body_size = WObj_update(self->body, body_lt, vec2_(0), enabled, theme, pointer_fn);
        }
    
        vec2 prev = self->body_size_prev;
        self->body_size_prev = body_size;
        
        if(m_equals_eps(body_size.x, self->body_size_prev.x, 0.1)) {
            break;
        }
        
        WTheme_reset_to(theme, theme_num);
    }
    
    
    vec2 rb = vec2_(o_max(header_lt.x + header_size.x, body_lt.x + body_size.x),
                    o_max(header_lt.y + header_size.y, body_lt.y + body_size.y));
    vec2 size = vec2_(rb.x - lt.x, rb.y - lt.y);

    //
    // hide btn
    //



    // may become invalid in pointer event!
    self->btn_hide.opt_box_ref = WObj_gen_boxes(self);
    self->btn_hide.opt_atlas_ref = WTheme_atlas_ref(theme);

    vec2 icon_size = u_atlas_size(WTheme_atlas(theme), self->btn_hide.state_up.atlas);
    vec2 pos = {{rb.x - 2.0f, header_lt.y + 1.0f}};
    pos = vec2_floor(pos);
    self->btn_hide.opt_box_ref->rect = u_rect_new_rt(m_2(pos), m_2(icon_size));
    u_btn_down_set(&self->btn_hide, self->hidden);

    if(!self->hideable) {
        self->btn_hide.opt_box_ref->rect.a = 0;
    }


    
    //
    // render children behind the actual window and grab size
    //

    self->children_size_prev = WObj__update_children_stacked(self, lt, min_size, enabled, theme, pointer_fn);

    vec2 full_size = vec2_max_v(size, field_size);

    //
    // pointer stuff
    //
    
    // may become invalid in contrast from update
    self->btn_hide.opt_box_ref = WObj_gen_boxes(self);

    // while dragging, pointer should be handled, so grabbing before
    struct a_pointer pointer = pointer_fn(0, 0);
    if (self->dragging) {
        a_pointer_handled(0, 0);
    }

    //
    // pointer
    //

    vec2 pp = pointer.pos.xy;

    bool hide_block = o_timer_elapsed_s(self->hide_block_timer) <= HIDE_BLOCK_TIME;
    bool toggled = false;
    
    hide_block |= !self->hideable;
   
    if (!hide_block) {
        if (u_btn_toggled(&self->btn_hide, pointer)) {
            self->hidden = u_btn_down(&self->btn_hide);
            self->hide_block_timer = o_timer();
            toggled = true;
        }
    }

    vec4 header_rect = u_rect_new_lt(m_2(header_lt), m_2(header_size));

    if (!toggled && a_pointer_pressed(pointer)
        && u_rect_contains(header_rect, pointer.pos.xy)) {

        if (!self->dragging) {
            self->dragging = true;
            self->dragging_pos = pp;
            self->dragging_lt = self->lt;
        }

        if (!hide_block && pp.x < u_rect_left(self->btn_hide.opt_box_ref->rect)) {
            if (o_timer_elapsed_s(self->dbl_tap_hide_timer) <= DBL_TAP_HIDE_TIME) {
                self->hidden = !self->hidden;
                self->dbl_tap_hide_timer = 0;
                self->hide_block_timer = o_timer();
            } else {
                self->dbl_tap_hide_timer = o_timer();
            }
        }
    }


    if (!pointer.down) {
        self->dragging = false;
    }

    self->dragging &= self->draggable;
    if (self->dragging) {
        vec2 diff = vec2_sub_v(pp, self->dragging_pos);
        self->lt = vec2_(self->dragging_lt.x + diff.x, self->dragging_lt.y + diff.y);
    }

    //
    // pointer handled if in window frame
    //

    if (header_lt.x <= pp.x && pp.x <= rb.x
        && header_lt.y <= pp.y && pp.y <= rb.y) {
        a_pointer_handled(0, 0);
        
        if(pointer_active) {
            self->active = true;
        }
    }

    return full_size;
}

oobj *WWindow__v_list(oobj obj, osize *opt_out_num)
{
    OObj_assert(obj, WWindow);
    WWindow *self = obj;

    osize direct_size;
    WObj **direct = WObj_list_direct(self, &direct_size);

    // directs + label + NULL
    direct = o_realloc(self, direct, sizeof(WObj *), direct_size + 3);
    direct[direct_size] = self->header;
    direct[direct_size + 1] = self->body;
    direct[direct_size + 2] = NULL;
    o_opt_set(opt_out_num, direct_size + 2);
    return (oobj *) direct;
}

void WWindow__v_style_apply(oobj obj)
{
    WObj__v_style_apply(obj);
    
    OObj_assert(obj, WWindow);
    WWindow *self = obj;
    oobj style = WObj_style(self);
    
    WPane_color_set(self->header, WStyle_window_header_color(style));
    WPane_style_set(self->header, WStyle_window_header_style(style));
    WPane_color_set(self->body, WStyle_window_body_color(style));
    WPane_style_set(self->body, WStyle_window_body_style(style));
}
