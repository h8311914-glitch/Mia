#include "mp/surface.h"

#include "o/file.h"
#include "o/log.h"
#include "o/str.h"
#include "o/timer.h"
#include "o/OJson.h"
#include "o/OPtr.h"
#include "m/vec/vec2.h"
#include "m/vec/ivec2.h"
#include "m/MMat_ex.h"
#include "r/RTex.h"
#include "u/color.h"

#include "mp/history.h"



static struct oobj_opt surface_history_get_mat(struct mp_surface *self, struct mp_history *history, 
    int step_pos, int *opt_out_ring_pos)
{
    oobj json_mat = mp_history_at(history, step_pos, "surface/mat").o;
    if (json_mat) {
        oobj mat = o_user(json_mat);
        MMat_assert(mat);
        int ring_pos = mp_history_ring_pos(history, step_pos);
        o_opt_set(opt_out_ring_pos, ring_pos);
        return oobj_opt(mat);
    }
    oobj json_ptr = mp_history_at(history, step_pos, "surface/ptr").o;
    if (json_ptr) {
        oobj ptr = o_user(json_ptr);
        oobj mat = OPtr_get(ptr).o;
        if (mat) {
            MMat_assert(mat);
            assert(OJson_number(json_ptr));
            int ring_pos = (int) *OJson_number(json_ptr);
            o_opt_set(opt_out_ring_pos, ring_pos);
            return oobj_opt(mat);
        }
    }
    return oobj_opt(NULL);
}


static osize surface_v_history_create(struct mp_history *history, oobj step_json, int step_pos, void *user_data)
{
    struct mp_surface *self = user_data;
    
    oobj prev_mat = NULL;
    int ring_pos = -1;
    if (step_pos > 0) {
        prev_mat = surface_history_get_mat(self, history, step_pos - 1, &ring_pos).o;
    }
    if (prev_mat && MMat_equals(self->canvas_mat, prev_mat)) {
        // ptr to equal prev mat
        oobj json_ptr = OJson_new_number(step_json, "ptr", ring_pos);
        oobj ptr = OPtr_new(json_ptr, prev_mat);
        o_user_set(json_ptr, ptr);
        return 0;
    }
    
    // new mat
    {
        ring_pos = mp_history_ring_pos(history, step_pos);
        oobj json_mat = OJson_new_number(step_json, "mat", ring_pos);
        oobj mat = MMat_clone(self->canvas_mat);
        o_move(mat, json_mat);
        o_user_set(json_mat, mat);
        
        if(mp_history_auto_save(history)) {
            char file[32];
        o_strf_buf(file, "&mp_surface_%03i.png", ring_pos);
            MMat_write_file_image(mat, file);
        }

        // rough size of the cached mat
        return MMat_data_size(mat);
    }
}

static osize surface_v_history_apply(struct mp_history *history, oobj step_json, int step_pos, void *user_data)
{
    struct mp_surface *self = user_data;
    oobj mat = surface_history_get_mat(self, history, step_pos, NULL).o;
    if (!mat) {
        o_log_warn("failed to parse json, mat not found");
        return false;
    }
    if (MMat_layout_equals(mat, self->canvas_mat)) {
        MMat_blit(self->canvas_mat, mat, 0, 0);
    } else {
        o_del(self->canvas_mat);
        self->canvas_mat = MMat_clone(mat);
        o_move(self->canvas_mat, self->container);
        mp_surface_view_rect_set(self, ivec4_(0));
    }
    self->canvas_dirty = true;
    return true;
}

static osize surface_v_history_cmp(struct mp_history *history, oobj step_json, int step_pos, void *user_data)
{
    struct mp_surface *self = user_data;
    oobj mat = surface_history_get_mat(self, history, step_pos, NULL).o;
    if (!mat) {
        o_log_warn("failed to parse json, mat not found");
        return false;
    }
    return MMat_equals(self->canvas_mat, mat);
}

static osize surface_v_history_valid(struct mp_history *history, oobj step_json, int step_pos, void *user_data)
{
    struct mp_surface *self = user_data;
    return surface_history_get_mat(self, history, step_pos, NULL).o != NULL;
}

static osize surface_v_history_load(struct mp_history *history, oobj step_json, int step_pos, void *user_data)
{
    struct mp_surface *self = user_data;

    oobj json_mat = OJson_child(step_json, "mat").o;
    if(json_mat) {
        if(!OJson_number(json_mat)) {
            o_log_warn("invalid json type for mat");
            return 0;
        }
        int ring_pos = (int) *OJson_number(json_mat);
        char file[32];
        o_strf_buf(file, "&mp_surface_%03i.png", ring_pos);
        
        oobj mat = MMat_new_file_image(step_json, file).o;
        if(!mat) {
            o_log_warn("failed to load %s", file);
            return 0;
        }
        o_user_set(json_mat, mat);
        
        // rough size of the cached mat
        return MMat_data_size(mat);
    }
    
    
    oobj json_ptr = OJson_child(step_json, "ptr").o;
    if (!json_ptr) {
        o_log_warn("neither json mat nor ptr children found");
        return 0;
    } else {
        if(!OJson_number(json_ptr)) {
            o_log_warn("invalid json type for ptr");
            return 0;
        }
        int ring_pos = (int) *OJson_number(json_ptr);
        
        int mat_step_pos = mp_history_step_pos(history, ring_pos);
        
        int found_ring_pos;
        oobj mat = surface_history_get_mat(self, history, mat_step_pos, &found_ring_pos).o;
        if(!mat) {
            o_log_warn("ptr mat not found");
            return 0;
        }
        if(ring_pos != found_ring_pos) {
            o_log_warn("invalid ring_pos");
            return 0;
        }
        
        oobj ptr = OPtr_new(step_json, mat);
        o_user_set(json_ptr, ptr);
    }
    
    return 0;
}

//
// public
//

struct mp_surface *mp_surface_new(oobj parent, struct mp_history *history)
{
    oobj container = OObj_new(parent);
    struct mp_surface *self = o_new0(container, *self, 1);
    self->container = container;

    self->dashing = 2.0;
    self->brush = MMat_new(container, &U_WHITE, 1, 1, M_FORMAT_RGBA_U8);

    int cols = 64;
    int rows = 64;

    self->canvas_mat = MMat_new_0(container, cols, rows, M_FORMAT_RGBA_U8);
    self->canvas_tex = RTex_new(container, NULL, cols, rows);
    self->canvas_dirty = true;

    self->draw_dabs_offset_array = OArray_new_dyn(container, NULL, sizeof(vec2), 0, 128);


    self->canvas_mat_view = NULL;
    mp_surface_view_rect_set(self, ivec4_(0));

    mp_history_module_add(history, "surface", self,
                          surface_v_history_create,
                          surface_v_history_apply,
                          surface_v_history_cmp,
                          surface_v_history_valid,
                          surface_v_history_load);

    return self;
}

oobj mp_surface_canvas_tex(struct mp_surface *self)
{
    if (self->canvas_dirty) {
        RTex_set_from_mat(self->canvas_tex, self->canvas_mat);
        self->canvas_dirty = false;
    }
    return self->canvas_tex;
}

void mp_surface_color_swap(struct mp_surface *self)
{
    vec4 tmp = self->color;
    self->color = self->color_2nd;
    self->color_2nd = tmp;
}

void mp_surface_view_rect_set(struct mp_surface *self, ivec4 view_rect)
{
    o_del(self->canvas_mat_view);
    self->canvas_mat_view = MMat_window_rect(self->canvas_mat, view_rect, true);
    self->canvas_mat_view_offset_int = view_rect.xy;
    self->canvas_mat_view_offset = vec2_(m_2(self->canvas_mat_view_offset_int));
}

// just to test?
void mp_surface_update(struct mp_surface *self)
{
    // noop?
}

//
// tools
//

void mp_surface_clear(struct mp_surface *self)
{
    MMat_clear_vec4(self->canvas_mat_view, R_TRANSPARENT);
    self->canvas_dirty = true;
}

bool mp_surface_pick(struct mp_surface *self, vec4 *out_picked, vec2 pos)
{
    // ignoring view mat and pick in the full canvas
    ivec2 cr = ivec2_cast_float(pos.v);
    ivec2 size = MMat_size_int(self->canvas_mat);
    if (cr.x < 0 || cr.y < 0 || cr.x >= size.x || cr.y >= size.y) {
        return false;
    }
    *out_picked = MMat_at_vec4(self->canvas_mat, cr.x, cr.y);
    return true;
}


void mp_surface_draw_dabs(struct mp_surface *self, const vec2 *dabs, int num)
{
    OArray_resize(self->draw_dabs_offset_array, num);
    vec2 *dabs_offset = OArray_data(self->draw_dabs_offset_array, vec2);
    for (int i = 0; i < num; i++) {
        dabs_offset[i] = vec2_sub_v(dabs[i], self->canvas_mat_view_offset);
    }
    MMat_draw_dabs(self->canvas_mat_view, self->color, dabs_offset, num, self->brush);
    self->canvas_dirty = num > 0;
}

void mp_surface_draw_dot(struct mp_surface *self, vec2 pos)
{
    pos = vec2_sub_v(pos, self->canvas_mat_view_offset);
    MMat_draw_dot(self->canvas_mat_view, self->color, pos, self->brush);
    self->canvas_dirty = true;
}

void mp_surface_draw_line(struct mp_surface *self, vec2 a, vec2 b)
{
    a = vec2_sub_v(a, self->canvas_mat_view_offset);
    b = vec2_sub_v(b, self->canvas_mat_view_offset);
    MMat_draw_line(self->canvas_mat_view, self->color, a, b, self->brush);
    self->canvas_dirty = true;
}

void mp_surface_draw_box(struct mp_surface *self, vec2 a, vec2 b)
{
    a = vec2_sub_v(a, self->canvas_mat_view_offset);
    b = vec2_sub_v(b, self->canvas_mat_view_offset);
    MMat_draw_box(self->canvas_mat_view, self->color, a, b, self->brush);
    self->canvas_dirty = true;
}

void mp_surface_draw_circle(struct mp_surface *self, vec2 center, float radius)
{
    center = vec2_sub_v(center, self->canvas_mat_view_offset);
    MMat_draw_circle(self->canvas_mat_view, self->color, center, radius, self->brush);
    self->canvas_dirty = true;
}

void mp_surface_fill(struct mp_surface *self, vec2 pos, bool mode8)
{
    pos = vec2_sub_v(pos, self->canvas_mat_view_offset);
    MMat_fill_vec4(self->canvas_mat_view, pos.x, pos.y, mode8, self->color);
    self->canvas_dirty = true;
}

void mp_surface_replace(struct mp_surface *self, vec2 pos)
{
    pos = vec2_sub_v(pos, self->canvas_mat_view_offset);
    ivec2 cr = ivec2_cast_float(pos.v);
    ivec2 size = MMat_size_int(self->canvas_mat_view);
    if (cr.x < 0 || cr.y < 0 || cr.x >= size.x || cr.y >= size.y) {
        return;
    }
    vec4 from = MMat_at_vec4(self->canvas_mat_view, cr.x, cr.y);
    MMat_replace(self->canvas_mat_view, from, self->color);
    self->canvas_dirty = true;
}

//
// paste_mat
//

void mp_surface_paste_draw(struct mp_surface *self)
{
    if (!self->paste_mat) {
        return;
    }
    void (*paste_fn)(oobj, oobj, int, int) = self->paste_mat_blend ? MMat_blend : MMat_blit;
    paste_fn(self->canvas_mat, self->paste_mat, m_2(self->paste_mat_offset));
    self->canvas_dirty = true;
}

void mp_surface_paste_copy(struct mp_surface *self, ivec4 rect, bool cut)
{
    if (rect.left<0) {
        rect.width += rect.left;
        rect.left = 0;
    }
    if (rect.top<0) {
        rect.height += rect.top;
        rect.top = 0;
    }
    ivec2 max_size = ivec2_sub_v(MMat_size_int(self->canvas_mat), rect.xy);
    rect.zw = ivec2_clamp_v(rect.zw, ivec2_(0), max_size);
    if (rect.width <= 0 || rect.height <= 0) {
        return;
    }
    // tmp MMat, needs to be o_del'ed
    oobj container = MMat_window_rect(self->canvas_mat, rect, false);

    o_del(self->paste_mat);
    self->paste_mat = MMat_clone(container);
    o_move(self->paste_mat, self->container);
    self->paste_mat_offset = rect.xy;

    if (cut) {
        MMat_clear_vec4(container, R_TRANSPARENT);
    }

    o_del(container);
}

void mp_surface_paste_rotate(struct mp_surface *self, bool left)
{
    if (!self->paste_mat) {
        return;
    }
    if (left) {
        self->paste_mat = MMat_rotate_left(self->paste_mat);
    } else {
        self->paste_mat = MMat_rotate_right(self->paste_mat);
    }
}

void mp_surface_paste_mirror(struct mp_surface *self, bool h)
{
    if (!self->paste_mat) {
        return;
    }
    if (h) {
        self->paste_mat = MMat_mirror_h(self->paste_mat);
    } else {
        self->paste_mat = MMat_mirror_v(self->paste_mat);
    }
}
