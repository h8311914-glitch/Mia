#ifdef MIA_OPTION_SDL2
#include <SDL2/SDL_events.h>
#define SDL_EVENT_MOUSE_BUTTON_DOWN SDL_MOUSEBUTTONDOWN
#define SDL_EVENT_MOUSE_MOTION SDL_MOUSEMOTION
#define SDL_EVENT_MOUSE_BUTTON_UP SDL_MOUSEBUTTONUP
#define SDL_EVENT_FINGER_DOWN SDL_FINGERDOWN
#define SDL_EVENT_FINGER_MOTION SDL_FINGERMOTION
#define SDL_EVENT_FINGER_UP SDL_FINGERUP
#define SDL_EVENT_MOUSE_WHEEL SDL_MOUSEWHEEL
#else
#include <SDL3/SDL.h>
#endif


#include "a/pointer.h"
#include "o/OEvent.h"
#include "r/common.h"
#include "a/app.h"
#include "a/AView.h"

#undef O_LOG_LIB
#define O_LOG_LIB "a"

#include "o/log.h"


// if no touch activity for 10 s, go back to cursor mode
#define TOUCH_FALLBACK_TIME 10.0

// We ignore touch and cursor until pen activity fades away + 0.25 s
//     The pen is also active while hovering...
#define PEN_FALLBACK_TIME 0.25


// sdl is left middle right
// mia is left right middle (cause its nicer for apps to use pointer(1) for right click like second finger)
#define SDL_BTN_TO_MIA_BTN_ID(id) ( (int[]) {0, 2, 1} [(id)] )

struct touch {
    int pointer_id;
    SDL_FingerID finger_id;
    vec4 pos_gl;
};

struct full_pointer {
    struct a_pointer pointer;
    struct a_pointer_pen pen;
    bool handled;
};


static struct {
    // last can't be set to handled
    struct full_pointer pointer_last[a_pointer_MAX];
    // current is a fresh copy of last on each frame (after events are handled)
    // can be set to handled for this frame
    struct full_pointer pointer_current[a_pointer_MAX];
    // history, if handled, remains handled
    struct full_pointer pointer_history[a_pointer_MAX][a_pointer_MAX_BACKTRACE];
    int pointer_history_current[a_pointer_MAX];
    int pointer_history_num[a_pointer_MAX];

    bool pointer_history_down[a_pointer_MAX];
    bool pointer_current_down[a_pointer_MAX];
    
    enum a_pointer_source source;

    obyte mouse_pressed[a_pointer_MAX_BUTTON_IDS];

    bool pen_has_been_used;
    float pen_deadzone;
    float pen_pressure;
    float pen_last_time;
    struct a_pointer_pen pen;

    struct touch touch_list[a_pointer_MAX];
    int touch_list_size;
    bool touch_used;
    float touch_last_time;

    ou64 timer;
} pointer_L;

static void clear_pen(void)
{
    pointer_L.pen = (struct a_pointer_pen){0};
    pointer_L.pen_pressure = 0;
}

static void set_pointer(int idx, bool down, vec4 pos, float pressure, enum a_pointer_source source)
{
    assert(0 <= idx && idx < a_pointer_MAX);
    
    pointer_L.source = source;

    pointer_L.pointer_history_num[idx] = o_min(pointer_L.pointer_history_num[idx] + 1, a_pointer_MAX_BACKTRACE);
    pointer_L.pointer_history_current[idx]++;
    pointer_L.pointer_history_current[idx] %= a_pointer_MAX_BACKTRACE;
    int bt = pointer_L.pointer_history_current[idx];

    pointer_L.pointer_history[idx][bt].pointer.down = down;
    pointer_L.pointer_history[idx][bt].pointer.prev_down = pointer_L.pointer_history_down[idx];
    pointer_L.pointer_history[idx][bt].pointer.pos = pos;
    pointer_L.pointer_history[idx][bt].pointer.pressure = down ? pressure : 0.0f;
    pointer_L.pointer_history[idx][bt].pen = pointer_L.pen;
    pointer_L.pointer_history[idx][bt].handled = false;

    pointer_L.pointer_history_down[idx] = down;

    pointer_L.pointer_last[idx] = pointer_L.pointer_history[idx][bt];

    pointer_L.timer = a_timer();
}

// protected: reset touch on scene exit...
void a_pointer__reset_touch_instantly(void)
{
    if (o_likely(pointer_L.touch_list_size > 0)) {
        return;
    }
    o_log_trace_s("a_app_reset_touch", "resetting touch pointers");
    for (int i = 0; i < pointer_L.touch_list_size; i++) {
        set_pointer(i, false, pointer_L.touch_list[i].pos_gl, 0.0f, a_pointer_source_TOUCH);
        pointer_L.pointer_history_num[i] = 0;
    }
    pointer_L.touch_list_size = 0;
    pointer_L.touch_last_time = TOUCH_FALLBACK_TIME;
}

static bool touch_pointer_id_already_exists(int pointer_id)
{
    for (int i = 0; i < pointer_L.touch_list_size; i++) {
        if (pointer_L.touch_list[i].pointer_id == pointer_id)
            return true;
    }
    return false;
}

static void touch_id_remove(int id)
{
    if (o_unlikely(id < 0 || id >= pointer_L.touch_list_size)) {
        o_log_error_s("handle_event_touch", "invalid id: %i/%i", id, pointer_L.touch_list_size);
        return;
    }
    for (int i = id; i < pointer_L.touch_list_size - 1; i++) {
        pointer_L.touch_list[i] = pointer_L.touch_list[i + 1];
    }
    pointer_L.touch_list_size--;
}

static vec4 coord_pos_gl(float x, float y)
{
    // protected
    vec2 a_app__sdl_window_coord_size(void);

    vec4 pos_gl;
    vec2 wnd_size = a_app__sdl_window_coord_size();
    pos_gl.x = (2.0f * x) / wnd_size.x - 1.0f;
    pos_gl.y = 1.0f - (2.0f * y) / wnd_size.y;
    pos_gl.z = 0.0f;
    pos_gl.w = 1.0f;
    return pos_gl;
}

//
// protected
//

// protected
void a_pointer__init(void)
{
    // init hover pos out of screen, m_MAX is to far and may overflow
    const vec4 pos_init = {{1e8, 1e8, 0, 1}};
    
    for (int i = 0; i < a_pointer_MAX; i++) {
        for (int bt = 0; bt < a_pointer_MAX_BACKTRACE; bt++) {
            pointer_L.pointer_history[i][bt].pointer = (struct a_pointer){
                .idx = i, 
                .pos = pos_init
                };
        }
        pointer_L.pointer_current[i].pointer = (struct a_pointer){
                .idx = 0, 
                .pos = pos_init
                };
        pointer_L.pointer_last[i] = pointer_L.pointer_current[i];
    }
    pointer_L.touch_last_time = TOUCH_FALLBACK_TIME;
    pointer_L.pen_last_time = PEN_FALLBACK_TIME;
    pointer_L.pen_deadzone = 0.001;
}

// protected
void a_pointer__update(void)
{
    for (int i = 0; i < a_pointer_MAX; i++) {
        // clear history
        pointer_L.pointer_history_num[i] = 0;
    }
    pointer_L.touch_last_time += a_dt();
    pointer_L.pen_last_time += a_dt();
}

// protected
void a_pointer__update_events_handled(void)
{
    for (int i = 0; i < a_pointer_MAX; i++) {
        pointer_L.pointer_current[i] = pointer_L.pointer_last[i];
        pointer_L.pointer_current[i].pointer.prev_down = pointer_L.pointer_current_down[i];
        pointer_L.pointer_current_down[i] = pointer_L.pointer_current[i].pointer.down;
    }
}

// protected
void a_pointer__handle_event_mouse(SDL_Event *event)
{
    if (pointer_L.touch_last_time < TOUCH_FALLBACK_TIME
        || pointer_L.pen_last_time < PEN_FALLBACK_TIME) {
        // if we're touching or pen'ning, don't use mouse cursor
        return;
    }

    pointer_L.touch_used = false;

    int btn_sdl = event->button.button;
    int id = btn_sdl - 1;
    vec4 pos_gl;
    switch (event->type) {
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if (id < 0 || id >= a_pointer_MAX_BUTTON_IDS)
                break;
            pos_gl = coord_pos_gl(event->button.x, event->button.y);
            set_pointer(id, true, pos_gl, 1.0f, a_pointer_source_MOUSE);
            pointer_L.mouse_pressed[SDL_BTN_TO_MIA_BTN_ID(id)] = true;
            break;
        case SDL_EVENT_MOUSE_MOTION:
            pos_gl = coord_pos_gl(event->motion.x, event->motion.y);
            for (int i = 0; i < a_pointer_MAX_BUTTON_IDS; i++) {
                set_pointer(i, pointer_L.mouse_pressed[i], pos_gl, 1.0f, a_pointer_source_MOUSE);
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if (id < 0 || id >= a_pointer_MAX_BUTTON_IDS)
                break;
            pos_gl = coord_pos_gl(event->button.x, event->button.y);
            set_pointer(id, false, pos_gl, 0.0f, a_pointer_source_MOUSE);
            pointer_L.mouse_pressed[SDL_BTN_TO_MIA_BTN_ID(id)] = false;
            break;
    }
}

// protected
void a_pointer__handle_event_touch(SDL_Event *event)
{
    if (pointer_L.pen_last_time < PEN_FALLBACK_TIME) {
        // if we're pen'ning, don't use touch
        return;
    }

    float x = event->tfinger.x;
    float y = event->tfinger.y;
#ifdef MIA_OPTION_SDL2
    SDL_FingerID finger_id = event->tfinger.fingerId;
#else
    SDL_FingerID finger_id = event->tfinger.fingerID;
#endif
    bool active = event->type == SDL_EVENT_FINGER_DOWN || event->type == SDL_EVENT_FINGER_MOTION;
    if (!active && event->type != SDL_EVENT_FINGER_UP) {
        return;
    }


    pointer_L.touch_used = true;
    pointer_L.touch_last_time = 0;

    vec4 pos_gl;
    pos_gl.x = 2.0f * x - 1.0f;
    pos_gl.y = 1.0f - 2.0f * y;
    pos_gl.z = 0;
    pos_gl.w = 1;

    // check finger ids to cast to touch id
    int id = -1;
    for (int i = 0; i < pointer_L.touch_list_size; i++) {
        if (pointer_L.touch_list[i].finger_id == finger_id) {
            id = i;
            break;
        }
    }

    if (o_unlikely(id >= 0 && event->type == SDL_EVENT_FINGER_DOWN)) {
        o_log_trace_s("handle_event_touch", "got down, but also an id");
        touch_id_remove(id);
        id = -1;
    }

    if (id < 0) {
        if (o_unlikely(event->type == SDL_EVENT_FINGER_UP)) {
            o_log_trace_s("handle_event_touch",
                          "got an up without an active touch id, ignoring...");
            return;
        }
        if (pointer_L.touch_list_size >= a_pointer_MAX) {
            a_pointer__reset_touch_instantly();
        }

        // create new id
        int pointer_id = 0;
        while (touch_pointer_id_already_exists(pointer_id)) {
            pointer_id++;
        }

        id = pointer_L.touch_list_size++;
        pointer_L.touch_list[id].pointer_id = pointer_id;
        pointer_L.touch_list[id].finger_id = finger_id;

        if (o_unlikely(event->type == SDL_EVENT_FINGER_MOTION)) {
            o_log_trace_s("handle_event_touch",
                          "e_input_update: got move, but not an id");
        }
    }

    // update pos
    pointer_L.touch_list[id].pos_gl = pos_gl;

    // stylus if available
    float pressure = event->tfinger.pressure;
    if (pressure < 0 || pressure > 1.0) {
        // if garbage:
        pressure = 1.0f;
    }

    int pointer_id = pointer_L.touch_list[id].pointer_id;

    // remove ups
    if (!active) {
        touch_id_remove(id);
    }

    set_pointer(pointer_id, active, pos_gl, pressure, a_pointer_source_TOUCH);
}

// protected
void a_pointer__handle_event_pen(SDL_Event *event)
{
#ifndef MIA_OPTION_SDL2

    // event->type is either:
    // SDL_EVENT_PEN_DOWN
    // SDL_EVENT_PEN_UP
    // SDL_EVENT_PEN_MOTION
    // SDL_EVENT_PEN_AXIS

    pointer_L.pen_last_time = 0;
    pointer_L.pen_has_been_used = true;
    a_pointer__reset_touch_instantly();

    // SDL_EVENT_PEN_PROXIMITY_IN | OUT is not safe enough...
    if (event->type == SDL_EVENT_PEN_AXIS) {
        pointer_L.pen.state = event->paxis.pen_state;
        switch (event->paxis.axis) {
            case SDL_PEN_AXIS_PRESSURE:
                pointer_L.pen_pressure = event->paxis.value;
                break;
            case SDL_PEN_AXIS_XTILT:
                pointer_L.pen.tilt.x = event->paxis.value;
            break;
            case SDL_PEN_AXIS_YTILT:
                pointer_L.pen.tilt.y = event->paxis.value;
            break;
            case SDL_PEN_AXIS_DISTANCE:
                pointer_L.pen.distance = event->paxis.value;
            break;
            case SDL_PEN_AXIS_ROTATION:
                pointer_L.pen.rotation = event->paxis.value;
            break;
            case SDL_PEN_AXIS_SLIDER:
                pointer_L.pen.slider = event->paxis.value;
            break;
            case SDL_PEN_AXIS_TANGENTIAL_PRESSURE:
                pointer_L.pen.tangential_pressure = event->paxis.value;
            break;
            default:
                // nothing to do
                    break;
        }
        return;
    }

    // remaining event->type's:
    // SDL_EVENT_PEN_DOWN
    // SDL_EVENT_PEN_UP
    // SDL_EVENT_PEN_MOTION

    bool active = pointer_L.pen_pressure >= pointer_L.pen_deadzone;
    vec4 pos_gl = coord_pos_gl(event->pmotion.x, event->pmotion.y);
    set_pointer(0, active, pos_gl, pointer_L.pen_pressure, a_pointer_source_PEN);

#endif
}


static struct full_pointer *full_back(int idx, int history)
{
    assert(idx >= 0 && idx < a_pointer_MAX);
    assert(history >= 0 && history <= a_pointer_MAX_BACKTRACE);
    if (history <= 0) {
        return &pointer_L.pointer_current[idx];
    }
    if (history > pointer_L.pointer_history_num[idx]) {
        o_log_warn_s(__func__, "history invalid, using last");
        history = pointer_L.pointer_history_num[idx];
    }
    history--;
    int bt = pointer_L.pointer_history_current[idx] - history;
    bt = o_mod(bt, a_pointer_MAX_BACKTRACE);
    return &pointer_L.pointer_history[idx][bt];
}

//
// public
//

struct a_pointer a_pointer_back(int idx, int history)
{
    return full_back(idx, history)->pointer;
}

//
// AView's delegations
//

struct a_pointer a_pointer_raw(int idx, int history)
{
    return AView_pointer_raw(a_view(), idx, history);
}


struct a_pointer a_pointer_p(int idx, int history)
{
    return AView_pointer_p(a_view(), idx, history);
}

struct a_pointer a_pointer(int idx, int history)
{
    return AView_pointer(a_view(), idx, history);
}

struct a_pointer_pen a_pointer_pen(int idx, int history)
{
    return full_back(idx, history)->pen;
}


//
// utility functions
//

enum a_pointer_source a_pointer_source(void)
{
    return pointer_L.source;
}

int a_pointer_history_num(int idx)
{
    assert(idx >= 0 && idx < a_pointer_MAX);
    return pointer_L.pointer_history_num[idx];
}

void a_pointer_handled_ex(int idx, int history, bool full)
{
    if (idx < 0) {
        for (int i = 0; i < a_pointer_MAX; i++) {
            a_pointer_handled_ex(i, history, full);
        }
        return;
    }
    assert(idx >= 0 && idx < a_pointer_MAX);
    if (history < 0) {
        for (int bt = 0; bt <= a_pointer_history_num(idx); bt++) {
            a_pointer_handled_ex(idx, bt, full);
        }
        return;
    }

    // check if its in the current a_view() bounds
    if (!full) {
        struct a_pointer p = a_pointer_raw(idx, history);
        if (p.pos.x < -1.0f || p.pos.x > +1.0f || p.pos.y < -1.0f || p.pos.y > +1.0f) {
            return;
        }
    }

    if (history == 0) {
        pointer_L.pointer_current[idx].pointer.down = false;
        pointer_L.pointer_current[idx].pointer.prev_down = false;
        pointer_L.pointer_current[idx].handled = true;
    } else {
        history--;
        assert(history >= 0 && history < a_pointer_MAX_BACKTRACE);

        int bt = pointer_L.pointer_history_current[idx] - history;
        bt = o_mod(bt, a_pointer_MAX_BACKTRACE);
        pointer_L.pointer_history[idx][bt].pointer.down = false;
        pointer_L.pointer_history[idx][bt].pointer.prev_down = false;
        pointer_L.pointer_history[idx][bt].handled = true;
    }
}

bool a_pointer_was_handled(int idx, int history)
{
    return full_back(idx, history)->handled;
}

void a_pointer_handled_forward_history(int idx)
{
    if (idx < 0) {
        for (int i = 0; i < a_pointer_MAX; i++) {
            a_pointer_handled_forward_history(i);
        }
        return;
    }
    assert(idx >= 0 && idx < a_pointer_MAX);
    if(a_pointer_was_handled(idx, 0)) {
        a_pointer_handled(idx, -1);
    }
}


static void defer_reset_touch(oobj defer)
{
    a_pointer__reset_touch_instantly();
}
void a_pointer_reset_touch(void)
{
    OEvent_new_post(a_root_threadsafe(), defer_reset_touch, NULL, NULL);

}

bool a_pointer_touch_used(void)
{
    return pointer_L.touch_used;
}

ou64 a_pointer_timer(void)
{
    return pointer_L.timer;
}

bool a_pointer_pen_available(void)
{
    return pointer_L.pen_has_been_used;
}

bool a_pointer_pen_used(void)
{
    return pointer_L.pen_last_time < PEN_FALLBACK_TIME;
}

float a_pointer_pen_deadzone(void)
{
    return pointer_L.pen_deadzone;
}

float a_pointer_pen_deadzone_set(float set)
{
    pointer_L.pen_deadzone = set;
    return pointer_L.pen_deadzone;
}

void a_pointer_override_back(struct a_pointer pointer, int idx, int history)
{
    full_back(idx, history)->pointer = pointer;
    pointer_L.source = a_pointer_source_OVERRIDE;
}

void a_pointer_override_pen(struct a_pointer_pen pen, int idx, int history)
{
    full_back(idx, history)->pen = pen;
    pointer_L.source = a_pointer_source_OVERRIDE;
}
