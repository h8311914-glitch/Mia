#include "x/cursor.h"
#include "o/str.h"
#include "m/vec/vec2.h"
#include "m/mat/mat4.h"
#include "r/RObjBox.h"
#include "r/RCam.h"
#include "r/tex.h"
#include "a/AView.h"
#include "a/input.h"
#include "a/action.h"
#include "a/app.h"
#include "u/rect.h"

#undef O_LOG_LIB
#define O_LOG_LIB "x"

#include "o/log.h"

#define GAMEPAD_SPEED 100.0f

#define CURSOR_OFFSET 4.0f
#define CURSOR_LENGTH 16.0f
#define CURSOR_WIDTH 1.0f

enum cursor_bind_types {
    TYPE_UP,
    TYPE_LEFT,
    TYPE_DOWN,
    TYPE_RIGHT,
    TYPE_GESTURES_MAX,
    TYPE_ANALOG_X,
    TYPE_ANALOG_Y,
};

struct cursor_bind {
    char *action;
    enum cursor_bind_types type;
    float scale;
};

static struct {
    bool installed;

    oobj view;

    struct x_cursor_options options;

    vec2 multitouch_start_pos;
    vec2 multitouch_offset;
    vec2 multitouch_down_start;
    float multitouch_gesture_time;
    int multitouch_turn_off_defer;

    bool gamepad_active;
    bool gamepad_prev_down;

    oobj ro;

    bool active;
    bool down;
    vec2 pos;

    vec2 analog;

    // up, left, down, right, action
    bool gestures[TYPE_GESTURES_MAX];
    oobj bind_array;
} cursor_L;

static void handle_multitouch(oobj view)
{
    // gestures
    o_clear(cursor_L.gestures, sizeof(bool), TYPE_GESTURES_MAX);
    cursor_L.multitouch_gesture_time += a_dt();

    if (!cursor_L.active) {
        struct a_pointer p = a_pointer(0, 0);
        if (a_pointer_pressed(p)) {
            oobj cam = AView_cam(view);
            vec2 cam_size = RCam_size(cam);
            if (p.pos.x <= cursor_L.options.multitouch_start_border) {
                cursor_L.multitouch_start_pos = p.pos.xy;
                cursor_L.multitouch_offset = vec2_(0, -cursor_L.options.multitouch_offset);
            } else if (p.pos.x >= cam_size.x - cursor_L.options.multitouch_start_border) {
                cursor_L.multitouch_start_pos = p.pos.xy;
                cursor_L.multitouch_offset = vec2_(0, -cursor_L.options.multitouch_offset);
            }
        }
        if (!p.down) {
            cursor_L.multitouch_start_pos.x = -1;
        } else if (cursor_L.multitouch_start_pos.x >= 0) {
            float dist = vec2_distance(cursor_L.multitouch_start_pos, p.pos.xy);
            if (dist >= cursor_L.options.multitouch_start_distance) {
                cursor_L.active = true;
                o_log_debug_s(__func__, "cursor on");
            }
        }
        
        cursor_L.multitouch_gesture_time = 999;
    }

    if (cursor_L.active && cursor_L.multitouch_turn_off_defer == 0) {
        // sequence front or back does not matter for overridding.
        mat4 to_back = mat4_mul_mat(*RCam_cam(AView_cam(view)), AView_display_pose(view));
        int p1_history_num = a_pointer_history_num(1);

        for (int h = 0; h <= a_pointer_history_num(0); h++) {
            struct a_pointer p = a_pointer(0, h);
            if (p.down) {
                struct a_pointer p1 = a_pointer(1, o_min(h, p1_history_num));

                // analog and gestures
                if (a_pointer_pressed(p1)) {
                    cursor_L.multitouch_down_start = p1.pos.xy;
                    cursor_L.multitouch_gesture_time = 0.0f;
                    
                } else if (p1.down) {
                    vec2 dist = vec2_sub_v(p1.pos.xy, cursor_L.multitouch_down_start);

                    cursor_L.analog = vec2_clamp(vec2_div(dist, cursor_L.options.multitouch_analog_radius), -1.0f, 1.0f);
                    cursor_L.analog.y *= -1.0f;

                    if (cursor_L.multitouch_gesture_time <= cursor_L.options.multitouch_gesture_time) {
                        const float gesture_dist = cursor_L.options.multitouch_gesture_dist;
                        if (dist.x <= -gesture_dist) {
                            // left
                            cursor_L.gestures[TYPE_LEFT] = true;
                            cursor_L.multitouch_gesture_time = 999;
                        } else if (dist.x >= +gesture_dist) {
                            // right
                            cursor_L.gestures[TYPE_RIGHT] = true;
                            cursor_L.multitouch_gesture_time = 999;
                        }
                        if (dist.y <= -gesture_dist) {
                            // up
                            cursor_L.gestures[TYPE_UP] = true;
                            cursor_L.multitouch_gesture_time = 999;
                        } else if (dist.y >= +gesture_dist) {
                            // down
                            cursor_L.gestures[TYPE_DOWN] = true;
                            cursor_L.multitouch_gesture_time = 999;
                        }
                    }
                }

                p.down = p1.down;
                p.prev_down = p1.prev_down;
                p.pressure = p1.down ? 1.0 : 0.0;
                vec2 pos = vec2_add_v(p.pos.xy, cursor_L.multitouch_offset);


                p.pos.xy = pos;
                p.pos = mat4_mul_vec(to_back, p.pos);

                a_pointer_override_back(p, 0, h);

                if (h == 0) {
                    cursor_L.down = p1.down;
                    cursor_L.pos = pos;
                }
            } else {
                // !p.down
                
                if(h>0 || !a_pointer(1, 0).down) {
                    // only for history and if the v cursor was NOT down
                    // if it was down we dont handle.
                    //     so app get released events from stopping with the first thumb
                    a_pointer_handled_ex(0, h, true);
                }
                if (h == 0) {
                    cursor_L.multitouch_turn_off_defer = cursor_L.down? 2 : 1;
                } 
            }
        }

        // handle all multi touch pointer
        for (int i=1; i < a_pointer_MAX; i++) {
            a_pointer_handled_ex(i, -1, true);
        }
    }


    if (cursor_L.multitouch_turn_off_defer > 1) {
        cursor_L.multitouch_turn_off_defer = 1;
        // if p1 currently down but cursor thumb got up first, create a release state before hard shutting down first
        o_log_debug_s(__func__, "cursor release");
        mat4 to_back = mat4_mul_mat(*RCam_cam(AView_cam(view)), AView_display_pose(view));
        struct a_pointer p = a_pointer(0, 0);
        p.pos.xy = cursor_L.pos;
        p.pos = mat4_mul_vec(to_back, p.pos);
        p.down = false;
        p.prev_down = cursor_L.down;
        a_pointer_override_back(p, 0, 0);

    } else if (cursor_L.multitouch_turn_off_defer == 1) {
        cursor_L.multitouch_turn_off_defer = 0;
        cursor_L.active = false;
        o_log_debug_s(__func__, "cursor off");

        // override the 0, 0 to be hidden.
        // its of course already up, but some apps use hovering
        // which may collide with source TOUCH not supporting hovering
        // and the last pointer being source OVERRIDE
        //     (and the hover cursor remains at the last thumb pos)
        struct a_pointer p = a_pointer(0, 0);
        p.pos.xy = vec2_(1e8);
        a_pointer_override_back(p, 0, 0);

        cursor_L.pos = vec2_(1e8);
        cursor_L.down = false;
    }
}

static void handle_gamepad(oobj view)
{
    if (!a_input_gamepad_available()) {
        cursor_L.gamepad_active = false;
        return;
    }
    oobj cam = AView_cam(view);
    vec2 cam_size = RCam_size(cam);

    if (a_input_pressed(A_INPUT_GAMEPAD_START)) {
        cursor_L.gamepad_active = !cursor_L.gamepad_active;
        o_log_s(__func__, "gamepad active: %d", cursor_L.gamepad_active);
        if (cursor_L.gamepad_active) {
            cursor_L.pos = vec2_div(cam_size, 2.0f);
        } else {
            cursor_L.pos = vec2_(1e8);
            cursor_L.down = false;
        }
    }
    if (!cursor_L.gamepad_active) {
        return;
    }

    cursor_L.active = true;
    cursor_L.down = a_input_down(A_INPUT_GAMEPAD_SOUTH);

    vec2 stick = a_input_analog_vec2(A_INPUT_ANALOG_GAMEPAD_STICK_LX);
    cursor_L.pos = vec2_add_scaled(cursor_L.pos, stick, GAMEPAD_SPEED);
    cursor_L.pos = vec2_clamp_v(cursor_L.pos, vec2_(0, 0), cam_size);

    a_pointer_handled_ex(-1, -1, true);

    mat4 to_back = mat4_mul_mat(*RCam_cam(cam), AView_display_pose(view));
    struct a_pointer p = {0};
    p.down = cursor_L.down;
    p.prev_down = cursor_L.gamepad_prev_down;
    cursor_L.gamepad_prev_down = cursor_L.down;
    p.pressure = cursor_L.down ? 1.0 : 0.0;
    p.pos = vec4_(cursor_L.pos.x, cursor_L.pos.y, 0, 1);
    p.pos = mat4_mul_vec(to_back, p.pos);

    for (int h = 0; h <= a_pointer_history_num(0); h++) {
        a_pointer_override_back(p, 0, h);
    }

    // handle all multi touch pointer
    for (int i = 1; i < a_pointer_MAX; i++) {
        a_pointer_handled_ex(i, -1, true);
    }

    // analog and gestures
    cursor_L.analog = a_input_analog_vec2(A_INPUT_ANALOG_GAMEPAD_STICK_RX);
    cursor_L.gestures[TYPE_UP] = a_input_pressed(A_INPUT_GAMEPAD_DPAD_UP);
    cursor_L.gestures[TYPE_LEFT] = a_input_pressed(A_INPUT_GAMEPAD_DPAD_LEFT);
    cursor_L.gestures[TYPE_DOWN] = a_input_pressed(A_INPUT_GAMEPAD_DPAD_DOWN);
    cursor_L.gestures[TYPE_RIGHT] = a_input_pressed(A_INPUT_GAMEPAD_DPAD_RIGHT);
}


static void cursor_setup(oobj view)
{
    // invalid if <0
    cursor_L.multitouch_start_pos.x = -1;

    cursor_L.ro = RObjBox_new_tex_rgba(view, 4, r_tex_white(), false, 1, 1);
}

static void cursor_update(oobj view, oobj tex, float dt)
{
#ifdef MIA_OPTION_GAMEPAD
    if (cursor_L.options.enable_gamepad) {
        handle_gamepad(view);
    }
#endif

    if (!cursor_L.gamepad_active && cursor_L.options.enable_multitouch) {
        handle_multitouch(view);
    }

    if (!cursor_L.active) {
        o_clear(cursor_L.gestures, sizeof(bool), 4);
    }

    osize bindings_num = OArray_num(cursor_L.bind_array);
    for (osize i=0; i<bindings_num; i++) {
        struct cursor_bind *bind = OArray_at_void(cursor_L.bind_array, i);
        if (bind->type < TYPE_GESTURES_MAX && cursor_L.gestures[bind->type]) {
            a_action_set(bind->action, bind->scale);
        } else if (bind->type == TYPE_ANALOG_X) {
            a_action_set(bind->action, cursor_L.analog.x * bind->scale);
        } else if (bind->type == TYPE_ANALOG_Y) {
            a_action_set(bind->action, cursor_L.analog.y * bind->scale);
        }
    }

}

static void cursor_render(oobj view, oobj tex, float dt)
{
    if (!cursor_L.active) {
        return;
    }

    if (cursor_L.options.opt_render_fn) {
        cursor_L.options.opt_render_fn(view, tex, dt);
    }

    if (!cursor_L.options.render_cursor) {
        return;
    }

    vec2 pos = cursor_L.pos;
    struct r_box *b = RObjBox_at(cursor_L.ro, 0);


    // left, top, right, bottom
    b[0].rect = u_rect_new(pos.x - CURSOR_OFFSET - CURSOR_LENGTH, pos.y - CURSOR_WIDTH / 2.0f, CURSOR_LENGTH,
                           CURSOR_WIDTH);
    b[1].rect = u_rect_new(pos.x - CURSOR_WIDTH / 2.0f, pos.y - CURSOR_OFFSET - CURSOR_LENGTH, CURSOR_WIDTH,
                           CURSOR_LENGTH);
    b[2].rect = u_rect_new(pos.x + CURSOR_OFFSET, pos.y - CURSOR_WIDTH / 2.0f, CURSOR_LENGTH, CURSOR_WIDTH);
    b[3].rect = u_rect_new(pos.x - CURSOR_WIDTH / 2.0f, pos.y + CURSOR_OFFSET, CURSOR_WIDTH, CURSOR_LENGTH);

    b[0].fx = b[1].fx = b[2].fx = b[3].fx = cursor_L.down ? cursor_L.options.color_down : cursor_L.options.color_hover;

    RObj_render(cursor_L.ro, tex);
}

//
// public
//

struct x_cursor_options x_cursor_options_default(void)
{
    struct x_cursor_options opt = {0};
    opt.enable_multitouch = true;
    opt.enable_gamepad = true;
    opt.multitouch_start_border = 8.0f;
    opt.multitouch_start_distance = 32.0f;
    opt.multitouch_offset = 48.0f;
    opt.multitouch_analog_radius = 32.0f;
    opt.multitouch_gesture_dist = 64.0f;
    opt.multitouch_gesture_time = 0.33f;
    opt.render_cursor = true;
    opt.opt_render_fn = NULL;
    opt.color_hover = vec4_(0.87, 0.87, 0.89, 0.66);
    opt.color_down = vec4_(0.91, 0.93, 0.91, 0.80);
    return opt;
}

void x_cursor_install(const struct x_cursor_options *opt_options)
{
    if (cursor_L.installed) {
        o_log_s(__func__, "already installed");
        return;
    }
    cursor_L.installed = true;
    if (opt_options) {
        cursor_L.options = *opt_options;
    } else {
        cursor_L.options = x_cursor_options_default();
    }


    cursor_L.view = a_app_main_view();
    struct AView_layer layer = {cursor_setup, cursor_update, cursor_render};
    AView_push_layer_deferred(cursor_L.view, layer, true);

    cursor_L.bind_array = OArray_new_dyn(cursor_L.view, NULL, sizeof(struct cursor_bind), 0, 8);
}

bool x_cursor_installed(void)
{
    return cursor_L.installed;
}

struct x_cursor_options *x_cursor_options_ref(void)
{
    assert(cursor_L.installed && "not installed?");
    return &cursor_L.options;
}

bool x_cursor_active(void)
{
    return cursor_L.active;
}

bool x_cursor_gamepad_active(void)
{
    return cursor_L.gamepad_active;
}

void x_cursor_start_abort(void)
{
    if(!cursor_L.active) {
        cursor_L.multitouch_start_pos.x = -1;
    }
}

vec2 x_cursor_pos(void)
{
    return cursor_L.pos;
}

bool x_cursor_down(void)
{
    return cursor_L.down;
}

//
// gestures
//

bool x_cursor_gesture_up(void)
{
    return cursor_L.gestures[TYPE_UP];
}

bool x_cursor_gesture_left(void)
{
    return cursor_L.gestures[TYPE_LEFT];
}

bool x_cursor_gesture_down(void)
{
    return cursor_L.gestures[TYPE_DOWN];
}

bool x_cursor_gesture_right(void)
{
    return cursor_L.gestures[TYPE_RIGHT];
}

vec2 x_cursor_analog(void)
{
    return cursor_L.analog;
}

void x_cursor_analog_bind_action(const char *opt_action_x, const char *opt_action_y)
{
    o_thread_assert_main();
    assert(cursor_L.installed && "not installed?");
    if (opt_action_x) {
        a_action_install(opt_action_x);
        struct cursor_bind bind = {o_str_clone(cursor_L.bind_array, opt_action_x), TYPE_ANALOG_X, 1.0f};
        OArray_push(cursor_L.bind_array, &bind);
    }
    if (opt_action_y) {
        a_action_install(opt_action_y);
        struct cursor_bind bind = {o_str_clone(cursor_L.bind_array, opt_action_y), TYPE_ANALOG_Y, 1.0f};
        OArray_push(cursor_L.bind_array, &bind);
    }
}

static void cursor_gesture_bind(const char *action, int gesture, float scale)
{
    o_thread_assert_main();
    assert(action != NULL);
    assert(cursor_L.installed && "not installed?");
    a_action_install(action);
    struct cursor_bind bind = {o_str_clone(cursor_L.bind_array, action), gesture, scale};
    OArray_push(cursor_L.bind_array, &bind);
}

void x_cursor_gesture_up_bind_action(const char *action)
{
    cursor_gesture_bind(action, TYPE_UP, +1.0f);
}

void x_cursor_gesture_left_bind_action(const char *action)
{
    cursor_gesture_bind(action, TYPE_LEFT, -1.0f);
}


void x_cursor_gesture_down_bind_action(const char *action)
{
    cursor_gesture_bind(action, TYPE_DOWN, -1.0f);
}


void x_cursor_gesture_right_bind_action(const char *action)
{
    cursor_gesture_bind(action, TYPE_RIGHT, +1.0f);
}


void x_cursor_gesture_bind_remove_all(void)
{
    o_thread_assert_main();
    assert(cursor_L.installed && "not installed?");
    o_del(cursor_L.bind_array);
    cursor_L.bind_array = OArray_new_dyn(cursor_L.view, NULL, sizeof(struct cursor_bind), 0, 8);
}
