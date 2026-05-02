#include "mp/selection.h"

#include "o/log.h"
#include "o/OJson.h"
#include "m/vec/int.h"
#include "m/vec/vec4.h"
#include "m/io/ivec4.h"
#include "r/tex.h"
#include "r/RTex.h"
#include "r/RCam.h"
#include "r/RObjBox.h"
#include "r/RShaderBox.h"
#include "a/common.h"
#include "a/pointer.h"
#include "u/rect.h"

#include "mp/history.h"


#define MOVE_BORDER_RADIUS 2.0
#define MOVE_BORDER_SQR_RADIUS (MOVE_BORDER_RADIUS*MOVE_BORDER_RADIUS)

static ivec4 selection_history_get_rect(oobj step_json)
{
    oobj rect_json = OJson_child(step_json, "rect").o;
    if (!rect_json) {
        return ivec4_(0);
    }
    ivec4 *cache = o_user(rect_json);
    if(cache) {
        return *cache;
    }
    ivec4 rect;
    bool valid = ivec4_json_read(&rect, rect_json);
    if(!valid) {
        return ivec4_(0);
    }
    cache = o_user_new0(rect_json, *cache, 1);
    *cache = rect;
    return rect;
}

static osize selection_v_history_create(struct mp_history *history, oobj step_json, int step_pos, void *user_data)
{
    struct mp_selection *self = user_data;
    if(self->rect.width<=0 || self->rect.height<=0) {
        return 0;
    }
    oobj rect_json = ivec4_json_new(step_json, "rect", self->rect);
    ivec4 *cache = o_user_new0(rect_json, *cache, 1);
    *cache = self->rect;
    return 0;
}

static osize selection_v_history_apply(struct mp_history *history, oobj step_json, int step_pos, void *user_data)
{
    struct mp_selection *self = user_data;
    if (self->state == mp_selection_state_PASTE) {
        return true;
    }
    self->rect = selection_history_get_rect(step_json);

    if (self->rect.width <= 0 && self->state == mp_selection_state_APPLY) {
        self->state = mp_selection_state_OFF;
    } else if (self->rect.width > 0 && self->state == mp_selection_state_OFF) {
        self->state = mp_selection_state_APPLY;
    }
    return true;
}

static osize selection_v_history_cmp(struct mp_history *history, oobj step_json, int step_pos, void *user_data)
{
    struct mp_selection *self = user_data;
    ivec4 rect = selection_history_get_rect(step_json);
    if(rect.width <= 0) {
        return self->rect.width <= 0 || self->rect.height <= 0;
    }
    return ivec4_equals_v(rect, self->rect);
}



static void selection_update_create(struct mp_selection *self)
{
    struct a_pointer p = a_pointer(0, 0);
    vec4 canvas_rect = vec4_cast_int(self->canvas_rect.v);

    // check initial create at
    if (self->create_start.left < 0 || self->create_start.top < 0) {
        if (u_rect_contains(canvas_rect, p.pos.xy)) {
            self->rect.xy = ivec2_cast_float(p.pos.v);;
            self->rect.zw = ivec2_(1, 1);
            if (a_pointer_pressed(p)) {
                self->create_start = self->rect.xy;
            }
        } else {
            self->rect = ivec4_(0);
        }
        a_pointer_handled(0, -1);
        return;
    }

    if (p.down) {
        if (u_rect_contains(canvas_rect, p.pos.xy)) {
            ivec2 pos = ivec2_cast_float(p.pos.v);
            int left = o_min(pos.x, self->create_start.x);
            int top = o_min(pos.y, self->create_start.y);
            int right = o_max(pos.x+1, self->create_start.x);
            int bottom = o_max(pos.y+1, self->create_start.y);
            int width = o_max(1, right - left);
            int height = o_max(1, bottom - top);
            self->rect = ivec4_(left, top, width, height);
        }
    } else {
        // finished
        mp_selection_state_set(self, mp_selection_state_APPLY);
    }
    a_pointer_handled(0, -1);
}

static void selection_update_move(struct mp_selection *self)
{
    struct a_pointer p = a_pointer(0, 0);
    vec4 rect = vec4_cast_int(self->rect.v);
    ivec2 pos = ivec2_cast_float(p.pos.v);

    if (a_pointer_pressed(p)) {
        
        vec2 lt = rect.xy;
        vec2 rb = vec2_add_v(lt, rect.zw);
        vec2 rt = vec2_(rb.x, lt.y);
        vec2 lb = vec2_(lt.x, rb.y);
        
        self->move_offset = ivec2_(0);
        
        // bvec4 border_moved is ltrb
        
        if(vec2_sqr_distance(p.pos.xy, lt) <= MOVE_BORDER_SQR_RADIUS) {
            // lt
            self->border_moved = bvec4_(1, 1, 0, 0);
        } else if(vec2_sqr_distance(p.pos.xy, rb) <= MOVE_BORDER_SQR_RADIUS) {
            // rb
            self->border_moved = bvec4_(0, 0, 1, 1);
        } else if(vec2_sqr_distance(p.pos.xy, rt) <= MOVE_BORDER_SQR_RADIUS) {
            // rt
            self->border_moved = bvec4_(0, 1, 1, 0);
        } else if(vec2_sqr_distance(p.pos.xy, lb) <= MOVE_BORDER_SQR_RADIUS) {
            // lb
            self->border_moved = bvec4_(1, 0, 0, 1);
        } else if(m_abs(p.pos.x - lt.x) <= MOVE_BORDER_RADIUS) {
            // left
            self->border_moved = bvec4_(1, 0, 0, 0);
        } else if(m_abs(p.pos.y - lt.y) <= MOVE_BORDER_RADIUS) {
            // top
            self->border_moved = bvec4_(0, 1, 0, 0);
        } else if(m_abs(p.pos.x - rb.x) <= MOVE_BORDER_RADIUS) {
            // right
            self->border_moved = bvec4_(0, 0, 1, 0);
        } else if(m_abs(p.pos.y - rb.y) <= MOVE_BORDER_RADIUS) {
            // bottom
            self->border_moved = bvec4_(0, 0, 0, 1);
        } else if (u_rect_contains(rect, p.pos.xy)) {
            // move selection relative
            self->move_offset = ivec2_sub_v(self->rect.xy, pos);
            self->border_moved = bvec4_(true);
        } 
        
        a_pointer_handled(0, -1);
        return;
    }

    if (p.down) {
        ivec2 offset = ivec2_add_v(pos, self->move_offset);
        if (self->border_moved.v0 && self->border_moved.v2) {
            // shift x
            int room = self->canvas_rect.width - self->rect.width;
            self->rect.x = offset.x;
            self->rect.x = o_clamp(self->rect.x, 0, room);
        } else if(self->border_moved.v2) {
            // right border == resize
            int room = self->canvas_rect.width - self->rect.left;
            self->rect.width = offset.x - self->rect.x;
            self->rect.width = o_clamp(self->rect.width, 1, room);
        } else if(self->border_moved.v0) {
            // left border
            int right = self->rect.x + self->rect.width;
            int room = right - 1;
            self->rect.x = o_clamp(offset.x, 0, room);
            self->rect.width = right - self->rect.x;
        }
        
        
        if (self->border_moved.v1 && self->border_moved.v3) {
            // shift y
            int room = self->canvas_rect.height - self->rect.height;
            self->rect.y = offset.y;
            self->rect.y = o_clamp(self->rect.y, 0, room);
        } else if(self->border_moved.v3) {
            // bottom border == resize
            int room = self->canvas_rect.height - self->rect.top;
            self->rect.height = offset.y - self->rect.y;
            self->rect.height = o_clamp(self->rect.height, 1, room);
        } else if(self->border_moved.v1) {
            // top border
            int bottom = self->rect.y + self->rect.height;
            int room = bottom - 1;
            self->rect.y = o_clamp(offset.y, 0, room);
            self->rect.height = bottom - self->rect.y;
        }
        
        
    } else {
        self->border_moved = bvec4_(false);
    }
    a_pointer_handled(0, -1);
}

static void selection_update_paste(struct mp_selection *self)
{
    struct a_pointer p = a_pointer(0, 0);
    vec4 rect = vec4_cast_int(self->rect.v);
    ivec2 pos = ivec2_cast_float(p.pos.v);

    if (a_pointer_pressed(p)) {
        
        if (u_rect_contains(rect, p.pos.xy)) {
            // move selection relative
            self->move_offset = ivec2_sub_v(self->rect.xy, pos);
            self->border_moved = bvec4_(true);
        } else if (u_rect_contains(vec4_cast_int(self->canvas_rect.v), p.pos.xy)) {
            // move selection absolute via center
            self->move_offset = ivec2_(-self->rect.width/2, -self->rect.height/2);
            self->border_moved = bvec4_(true);
        }
        
        a_pointer_handled(0, -1);
        return;
    }

    if (p.down) {
        ivec2 offset = ivec2_add_v(pos, self->move_offset);
        if (self->border_moved.v0 && self->border_moved.v2) {
            // shift x
            int room = self->canvas_rect.width - self->rect.width;
            self->rect.x = offset.x;
            self->rect.x = o_clamp(self->rect.x, 0, room);
        }
        
        if (self->border_moved.v1 && self->border_moved.v3) {
            // shift y
            int room = self->canvas_rect.height - self->rect.height;
            self->rect.y = offset.y;
            self->rect.y = o_clamp(self->rect.y, 0, room);
        } 
        
    } else {
        self->border_moved = bvec4_(false);
    }
    a_pointer_handled(0, -1);
}

//
// public
//

struct mp_selection *mp_selection_new(oobj parent, struct mp_history *history)
{
    oobj container = OObj_new(parent);
    struct mp_selection *self = o_new0(container, *self, 1);
    self->container = container;

    self->history = history;
    
    mp_history_module_add(history, "selection", self,
             selection_v_history_create,
             selection_v_history_apply,
             selection_v_history_cmp,
             NULL, NULL);

    self->state = mp_selection_state_OFF;
    self->rect = ivec4_(0);


    // chess pattern for the selection border to be rendered (wrap repeat)
    bvec4 pattern_buf[4];
    pattern_buf[0] = pattern_buf[3] = bvec4_(0xff, 0xff, 0xff, 0x60);
    pattern_buf[1] = pattern_buf[2] = bvec4_(0x00, 0x00, 0x00, 0x60);
    oobj pattern = RTex_new(self->container, pattern_buf, 2, 2);
    RTex_wrap_set(pattern, RTex_wrap_REPEAT);

    self->border_ro = RObjBox_new_tex_rgba(self->container, 4, pattern, true, 1, 1);

    // as we want to create a zoom independent border, but precisly on the same pixel (not rounded), we disable snap)
    RShaderBox_pixel_snap_set(RObjBox_shader(self->border_ro, 0), false);
    return self;
}


void mp_selection_state_set(struct mp_selection *self, enum mp_selection_state state)
{
    if (state == mp_selection_state_OFF || state == mp_selection_state_CREATE) {
        self->rect = ivec4_(0);
        self->create_start = ivec2_(-1);
    } else if (self->rect.width <= 0 || self->rect.height <= 0) {
        self->rect = ivec4_(0);
        state = mp_selection_state_OFF;
    }
    if(state == mp_selection_state_APPLY
        || state == mp_selection_state_OFF) {
        mp_history_commit(self->history);
    }
    self->state = state;
    o_log("switched state to: %i", state);
}

void mp_selection_rotate(struct mp_selection *self)
{
    int width = self->rect.width;
    self->rect.width = self->rect.height;
    self->rect.height = width;
}

void mp_selection_update(struct mp_selection *self, ivec4 canvas_rect)
{
    self->canvas_rect = canvas_rect;
    switch (self->state) {
        default:
        case mp_selection_state_OFF:
            self->rect = ivec4_(0);
            self->border_moved = bvec4_(false);
            return;
        case mp_selection_state_CREATE:
            selection_update_create(self);
            self->border_moved = bvec4_(false);
            return;
        case mp_selection_state_MOVE:
            selection_update_move(self);
            return;
        case mp_selection_state_APPLY:
            self->border_moved = bvec4_(false);
            return;
        case mp_selection_state_PASTE:
            selection_update_paste(self);
            return;
    }
}


void mp_selection_render(struct mp_selection *self, oobj tex)
{
    if (self->rect.width <= 0 || self->rect.height <= 0) {
        return;
    }

    vec4 sr = vec4_cast_int(self->rect.v);

    // lets the pattern move along the border
    self->border_t += 0.5 * a_dt();
    self->border_t = m_mod(self->border_t, 2);
    float b_t = self->border_t;

    // border stroke / width is dependent on the current zoom, and so independent of the canvas zoom
    float zoom = RCam_zoom(a_cam());
    float stroke = zoom * 2.0;

    // rect is along the borders, uv is setup to match the moving chess pattern correctly
    struct r_box *b = RObjBox_at(self->border_ro, 0);
    
    vec4 fx_def = self->state == mp_selection_state_MOVE? vec4_(1.0, 1.0, 0.33, 1.0) : R_WHITE;
    vec4 fx_move = R_GREEN;
    

    // left
    b[0].rect = vec4_(sr.x-stroke, sr.y, stroke, sr.height+stroke);
    b[0].uv_rect = vec4_(0, -b_t, b[0].rect.width / stroke, b[0].rect.height / stroke);
    b[0].fx = self->border_moved.v[0]? fx_move : fx_def;

    // top
    b[1].rect = vec4_(sr.x-stroke, sr.y-stroke, sr.width+stroke, stroke);
    b[1].uv_rect = vec4_(b_t, 0, b[1].rect.width / stroke, b[1].rect.height / stroke);
    b[1].fx = self->border_moved.v[1]? fx_move : fx_def;

    // right
    b[2].rect = vec4_(sr.x+sr.width, sr.y-stroke, stroke, sr.height+stroke);
    b[2].uv_rect = vec4_(0, b_t, b[2].rect.width / stroke, b[2].rect.height / stroke);
    b[2].fx = self->border_moved.v[2]? fx_move : fx_def;

    // bottom
    b[3].rect = vec4_(sr.x, sr.y+sr.height, sr.width+stroke, stroke);
    b[3].uv_rect = vec4_(-b_t, 0, b[3].rect.width / stroke, b[3].rect.height / stroke);
    b[3].fx = self->border_moved.v[3]? fx_move : fx_def;

    // render the borders
    RTex_ro(tex, self->border_ro);
}
