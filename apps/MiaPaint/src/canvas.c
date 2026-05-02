#include "mp/canvas.h"
#include "o/log.h"
#include "m/vec/ivec2.h"
#include "m/vec/vec2.h"
#include "a/action.h"
#include "a/AView.h"
#include "o/str.h"
#include "o/timer.h"
#include "r/RObjBox.h"
#include "u/zoom.h"
#include "u/pose.h"
#include "u/rect.h"
#include "r/RTex_blit.h"
#include "m/MMat.h"
#include "r/RCam.h"
#include "a/common.h"
#include "a/input.h"
#include "m/io/vec4.h"
#include "m/vec/vec4.h"
#include "o/img.h"
#include "r/tex.h"
#include "x/cursor.h"
#include "x/terminal.h"

#define CANVAS_VIEW_NAME "MpCanvas"

#define CANVAS_MOVE_TIMEOUT 1.5f
#define CANVAS_WHEEL_ZOOM 1.1f

#define CANVAS_UNDO_IGNORE_TIME_S 1.5


// bright and dark
static const obyte CANVAS_PATTERN_COLORSoff[2][4] = {
    {204, 200, 192, 187},
    {179, 175, 166, 161}
};
// bright and dark
static const obyte CANVAS_PATTERN_COLORS[2][4] = {
    {200, 198, 196, 194},
    {176, 174, 172, 170}
};

struct canvas_context {
    oobj view;
    struct mp_surface *surface;
    struct mp_selection *selection;
    struct mp_history *history;

    enum mp_canvas_tool tool;

    vec4 bg_color;

    struct u_zoom zoom;

    // pattern bg
    bool pattern_show;
    ivec2 pattern_size;
    oobj pattern_tex;

    struct a_pointer prev;

    bool x_cursor_active;
    
    bool hover_preview;
};


static void cam_home(struct canvas_context *C, bool selection)
{
    vec4 rect = vec4_(0);
    if (selection && C->selection->rect.width>0 && C->selection->rect.height>0) {
        rect = vec4_cast_int(C->selection->rect.v);
    } else {
        rect.zw = MMat_size(C->surface->canvas_mat);
    }
    RCam_fit(AView_cam(C->view), rect, false, 1.2f);
}

static void handle_hover_preview(struct canvas_context *C)
{
    if(a_pointer_source() == a_pointer_source_TOUCH) {
        return;
    }
    
    struct a_pointer p = a_pointer(0, 0);
    
    if(!p.down && !p.prev_down) {
        mp_surface_draw_dot(C->surface, p.pos.xy);
        C->hover_preview = true;
    }

}


static void handle_tool_pick(struct canvas_context *C)
{
    vec2 canvas_size = MMat_size(C->surface->canvas_mat);
    struct a_pointer p = a_pointer(0, 0);
    if (a_pointer_pressed(p)) {
        mp_surface_pick(C->surface, &C->surface->color, p.pos.xy);
        // o_log("picked: %s", vec4_str_a(C->surface->color, true));
    }
}


static void handle_tool_free(struct canvas_context *C)
{
    handle_hover_preview(C);
    
    /**
     * To handle all occured pointer events (in between frames) and not only the current one, use the history parameter
     */
    vec2 canvas_size = MMat_size(C->surface->canvas_mat);

    for (osize i=a_pointer_history_num(0); i>=0; i--) {
        struct a_pointer p = a_pointer(0, i);


        if (p.down) {
            
            if (C->prev.down) {
                mp_surface_draw_line(C->surface,
                        C->prev.pos.xy,
                        p.pos.xy);
            } else {
                mp_history_abort(C->history);
                mp_surface_draw_line(C->surface, p.pos.xy, p.pos.xy);
            } 
        }
        C->prev = p;
    }

    if (a_pointer_released(a_pointer(0, 0))) {
        o_log("free commit");
        mp_history_commit(C->history);
    }
}

static void handle_tool_line(struct canvas_context *C)
{
    handle_hover_preview(C);
    
    
    vec2 canvas_size = MMat_size(C->surface->canvas_mat);

    struct a_pointer p = a_pointer(0, 0);

    if (a_pointer_pressed(p)) {
        C->prev = p;
        mp_surface_draw_line(C->surface, p.pos.xy, p.pos.xy);
    } else if (p.down && C->prev.down) {
        mp_history_abort(C->history);
        mp_surface_draw_line(C->surface,
                C->prev.pos.xy,
                p.pos.xy);
    }

    if (a_pointer_released(p)) {
        o_log("line commit");
        mp_history_commit(C->history);
        C->prev = p;
    }
}


static void handle_tool_box(struct canvas_context *C)
{
    handle_hover_preview(C);
    
    
    vec2 canvas_size = MMat_size(C->surface->canvas_mat);

    struct a_pointer p = a_pointer(0, 0);

    
    if (a_pointer_pressed(p)) {
        C->prev = p;
        mp_surface_draw_line(C->surface, p.pos.xy, p.pos.xy);
    } else if (p.down && C->prev.down) {
        mp_history_abort(C->history);
        mp_surface_draw_box(C->surface, C->prev.pos.xy, p.pos.xy);
    }

    if (a_pointer_released(p)) {
        o_log("box commit");
        mp_history_commit(C->history);
        C->prev = p;
    }
}

static void handle_tool_circle(struct canvas_context *C)
{
    handle_hover_preview(C);
    
    
    vec2 canvas_size = MMat_size(C->surface->canvas_mat);

    struct a_pointer p = a_pointer(0, 0);

    if (a_pointer_pressed(p)) {
        C->prev = p;
        mp_surface_draw_circle(C->surface, p.pos.xy, -1);
    } else if (p.down && C->prev.down) {
        mp_history_abort(C->history);
        vec2 center = vec2_floor(C->prev.pos.xy);
        vec2 arcpnt = vec2_floor(p.pos.xy);
        float radius = vec2_distance(center, arcpnt);
        mp_surface_draw_circle(C->surface,
                center,
                radius);
    }

    if (a_pointer_released(p)) {
        o_log("circle commit");
        mp_history_commit(C->history);
        C->prev = p;
    }
}

static void handle_tool_fill(struct canvas_context *C, bool mode8)
{
    vec2 canvas_size = MMat_size(C->surface->canvas_mat);
    struct a_pointer p = a_pointer(0, 0);
    if (a_pointer_pressed(p)) {
        mp_surface_fill(C->surface, p.pos.xy, mode8);
        o_log("fill commit");
        mp_history_commit(C->history);
    }
}

static void handle_tool_replace(struct canvas_context *C)
{
    vec2 canvas_size = MMat_size(C->surface->canvas_mat);
    struct a_pointer p = a_pointer(0, 0);
    if (a_pointer_pressed(p)) {
        mp_surface_replace(C->surface, p.pos.xy);
        o_log("replace commit");
        mp_history_commit(C->history);
    }
}

static void canvas_setup(oobj view)
{
    struct canvas_context *C = o_user(view);
    C->zoom = u_zoom_new(a_cam());
    cam_home(C, false);
}

static void canvas_update(oobj view, oobj tex, float dt)
{
    struct canvas_context *C = o_user(view);

    bool move = u_zoom_update(&C->zoom, dt);
    bool allow_manip = !move;
    if (move || C->hover_preview) {
        C->hover_preview = false;
        mp_history_abort(C->history);
    }


    if (allow_manip) {
        mp_selection_update(C->selection, MMat_rect_int(C->surface->canvas_mat, 0, 0));
        if (mp_selection_uses_pointer(C->selection)) {
            allow_manip = false;
        }
        mp_surface_view_rect_set(C->surface, mp_selection_rect_applied(C->selection));
        if (C->selection->state == mp_selection_state_PASTE) {
            C->hover_preview = true;
            mp_surface_paste_draw(C->surface);
        }
    } 
    
    if(allow_manip) {
        switch (C->tool) {
            default:
            case mp_canvas_TOOL_PICK:
                handle_tool_pick(C);
                break;
            case mp_canvas_TOOL_FREE:
                handle_tool_free(C);
                break;
            case mp_canvas_TOOL_LINE:
                handle_tool_line(C);
                break;
            case mp_canvas_TOOL_BOX:
                handle_tool_box(C);
                break;
            case mp_canvas_TOOL_CIRCLE:
                handle_tool_circle(C);
                break;
            case mp_canvas_TOOL_FILL4:
                handle_tool_fill(C, false);
                break;
            case mp_canvas_TOOL_FILL8:
                handle_tool_fill(C, true);
                break;
            case mp_canvas_TOOL_REPLACE:
                handle_tool_replace(C);
                break;
        }
    } else {
        C->prev.down = false;
    }

    if (a_action_pressed("palette_prev")) {
        o_log("palette_prev");
        C->bg_color = R_RED;
    }
    if (a_action_pressed("palette_next")) {
        o_log("palette_next");
        C->bg_color = R_GREEN;
    }
    if (a_action_pressed("brush_prev")) {
        o_log("brush_prev");
        C->bg_color = R_YELLOW;
    }
    if (a_action_pressed("brush_next")) {
        o_log("brush_next");
        C->bg_color = R_CYAN;
    }

    if (x_cursor_active() && ! C->x_cursor_active) {
        o_log("vcursor abort");
        mp_history_abort(C->history);
        
    }
    C->x_cursor_active = x_cursor_active();
}

static void canvas_render(oobj view, oobj tex, float dt)
{
    struct canvas_context *C = o_user(view);
    RTex_clear(tex, C->bg_color);
    if (C->pattern_show) {
        vec4 rect = RTex_rect(C->surface->canvas_tex, 0, 0);
        // pattern is doubled in resolution, so each pixel to draw has 4 pattern pixels
        vec4 uv_rect = vec4_(rect.x, rect.y, rect.width*2, rect.height*2);
        RTex_blit_rect(tex, C->pattern_tex, rect, uv_rect);
    }

    oobj canvas = mp_surface_canvas_tex(C->surface);
    RTex_blend(tex, canvas, 0, 0);

    mp_selection_render(C->selection, tex);
}

oobj mp_canvas_new(oobj parent, struct mp_surface *surface, struct mp_selection *selection, struct mp_history *history)
{
    oobj view = AView_new(parent, canvas_setup, canvas_update, canvas_render);
    OObj_name_set(view, CANVAS_VIEW_NAME);
    struct canvas_context *C = o_user_new0(view, *C, 1);

    C->view = view;
    C->tool = mp_canvas_TOOL_FREE;

    C->surface = surface;
    C->selection = selection;
    C->history = history;
    C->bg_color = R_GRAY_X(0.33);
    
    mp_canvas_pattern_set(view, 8, 8);

    a_action_bind_remove_all();
    a_action_bind_digital("palette_prev", A_INPUT_1, 1);
    a_action_bind_digital("palette_next", A_INPUT_2, 1);
    a_action_bind_digital("brush_prev", A_INPUT_3, 1);
    a_action_bind_digital("brush_next", A_INPUT_4, 1);


    if (x_cursor_installed()) {
        x_cursor_gesture_bind_remove_all();
        x_cursor_gesture_left_bind_action("palette_prev");
        x_cursor_gesture_right_bind_action("palette_next");
        x_cursor_gesture_down_bind_action("brush_prev");
        x_cursor_gesture_up_bind_action("brush_next");
    }

    return view;
}

void mp_canvas_cam_home(oobj view, bool selection)
{

    assert(o_str_equals(OObj_name(view), CANVAS_VIEW_NAME));
    struct canvas_context *C = o_user(view);
    cam_home(C, selection);
}

void mp_canvas_pattern_set(oobj view, int cols, int rows)
{
    assert(o_str_equals(OObj_name(view), CANVAS_VIEW_NAME));
    struct canvas_context *C = o_user(view);

    if (ivec2_equals_v(C->pattern_size, ivec2_(cols, rows))) {
        return;
    }

    C->pattern_size = ivec2_(cols, rows);
    C->pattern_show = cols>0 && rows>0;
    if (!C->pattern_show) {
        return;
    }

    struct o_img img = o_img_new(view, cols*4, rows*4, o_img_RGBA);
    for (int r=0; r<img.rows; r++) {
        for (int c=0; c<img.cols; c++) {
            bool dark = r < img.rows / 2 ? (c < img.cols / 2 ? false : true) : (c < img.cols / 2 ? true : false);
            bool even_row = (r / 2) % 2 == 0;
            int dark_idx = dark ? 1 : 0;
            int idx;

            // swap each pixel row
            if (even_row)
                idx = c % 4;
            else
                idx = (c + 2) % 4;

            // bottom order of a pixel
            if (r % 2 == 1)
                idx = (int[]) {1, 0, 3, 2}[idx];
            obyte gray = CANVAS_PATTERN_COLORS[dark_idx][idx];
            bvec4 *pixel = (bvec4*) o_img_at(img, c, r);
            *pixel = bvec4_(gray, gray, gray, 255);
        }
    }

    o_del(C->pattern_tex);
    C->pattern_tex = RTex_new_img(view, &img);
    RTex_wrap_set(C->pattern_tex, RTex_wrap_REPEAT);
    o_img_free(&img);

}


enum mp_canvas_tool mp_canvas_tool(oobj view)
{
    assert(o_str_equals(OObj_name(view), CANVAS_VIEW_NAME));
    struct canvas_context *C = o_user(view);
    return C->tool;
}

enum mp_canvas_tool mp_canvas_tool_set(oobj view, enum mp_canvas_tool tool)
{
    assert(o_str_equals(OObj_name(view), CANVAS_VIEW_NAME));
    struct canvas_context *C = o_user(view);
    C->tool = tool;
    return C->tool;
}