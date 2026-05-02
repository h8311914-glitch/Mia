#ifdef MIA_OPTION_SDL2
#include <SDL2/SDL_hints.h>
#include <SDL2/SDL_video.h>
#include <SDL2/SDL_events.h>
#define SDL_DestroySurface SDL_FreeSurface
#define SDL_WINDOW_MAXIMIZED SDL_WINDOW_FULLSCREEN_DESKTOP
#define SDL_EVENT_QUIT SDL_QUIT
#define SDL_EVENT_MOUSE_BUTTON_DOWN SDL_MOUSEBUTTONDOWN
#define SDL_EVENT_MOUSE_MOTION SDL_MOUSEMOTION
#define SDL_EVENT_MOUSE_BUTTON_UP SDL_MOUSEBUTTONUP
#define SDL_EVENT_FINGER_DOWN SDL_FINGERDOWN
#define SDL_EVENT_FINGER_MOTION SDL_FINGERMOTION
#define SDL_EVENT_FINGER_UP SDL_FINGERUP
#define SDL_EVENT_KEY_DOWN SDL_KEYDOWN
#define SDL_EVENT_KEY_UP SDL_KEYUP
#define SDL_EVENT_MOUSE_WHEEL SDL_MOUSEWHEEL
#else
#include <SDL3/SDL.h>
#endif

#include "a/app.h"
#include "o/OObjRoot.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/OPtr.h"
#include "o/ODelcallback.h"
#include "o/OContext.h"
#include "o/OWeakjoin.h"
#include "o/OEvent.h"
#include "o/timer.h"
#include "o/file.h"
#include "o/img.h"
#include "o/str.h"
#include "m/vec/bvecn.h"
#include "m/vec/ivec2.h"
#include "m/mat/mat4.h"
#include "m/vec/vec4.h"
#include "s/common.h"
#include "r/common.h"
#include "r/RCam.h"
#include "r/RObjQuad.h"
#include "a/AScene.h"
#include "a/AViewStage.h"
#include "a/pointer.h"
#include <locale.h>


#undef O_LOG_LIB
#define O_LOG_LIB "a"
#include "o/log.h"

#ifdef MIA_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif

#ifdef MIA_OPTION_GLEW
#include "r/gl.h"
#endif


// if defined, the safe zone will get created from this pixel padding (left top right bottom)
// #define DEBUG_UNSAFE_PADDING ivec4_(0, 128, 0, 0)
// #define DEBUG_UNSAFE_PADDING ivec4_(32, 128, 64, 16)

#define MAX_DELTA_TIME 5.0 // seconds

// only for some platforms, see block_for_fps_limit
#define MAX_FPS 60

// scaled by dt (if fps>=10)
#define SMOOTH_ALPHA 1.5
#define SMOOTH_MIN_DT 0.1

#define METRICS_TIME 1.0


#ifndef MIA_OPTION_TITLE
#define MIA_OPTION_TITLE Mia
#endif


struct app_paused_event_info {
    a_app_paused_event event;
    oobj obj;
    oobj ptr;
    oobj opt_context;
};

struct app_update_event_info {
    OObj__event_fn event;
    oobj obj;
    oobj ptr;
    oobj opt_context;
};

static struct {
    bool init;
    SDL_Window *sdl_window;
    SDL_GLContext gl_context;
    bool running;
    bool paused;
    bool suspend_paused;
    bool suspend_force;
    OObj__event_fn opt_suspend_force_update_cb;
    oobj opt_suspend_force_update_obj;
    bool open_s_devices;
    bool open_audio_auto;
    bool open_mic_auto;
    bool audio_sound_auto_pause;

    enum a_app_screen_mode screen_mode;

    ivec2 back_size;
    vec2 sdl_window_coord_size;
    ivec4 viewport_unsafe;
    ivec4 viewport_safe;

    double limit_fps_time;

    // OArray of struct paused_event_info
    oobj paused_events;

    // OArray of struct update_event_info
    oobj update_events;


    // prevents recursion
    bool scene_exit_active;

    // OObjRoot with arena allocator
    struct o_allocator_i arena_allocator;
    oobj arena;
    osize arena_used;
    osize arena_empty_used;

    struct {
        bool enabled;
        double time;
    } metrics;


    // a/common.h
    oobj root;
    oobj root_threadsafe;
    oobj tmp;
    ou64 timer;
    double dt, time;
    float fps;
    float load, load_render;
    float load_ms, load_render_ms;
    bool is_touch;
    bool has_pen;
    bool using_pen;
    
    // AView for the app, defaults to an AStage
    oobj main_view;
    
    //
    // OContext shared:
    //
    oobj context_none;
    // Current AViewStage or NULL if not installed or running
    oobj stage;
    osize stage_context_idx;
    // set by AView
    oobj view;
    osize view_context_idx;
    // set by AViewStage
    oobj scene;
    osize scene_context_idx;


#ifdef MIA_PLATFORM_EMSCRIPTEN
    bool emscripten_file_init_done;
#endif
} app_L;


static void init(const struct a_app_run_options *opt_options)
{
    struct a_app_run_options options = {0};
    if (opt_options) {
        options = *opt_options;
    } else {
        options = a_app_run_options_default();
    }


    assert(!app_L.init);
    app_L.init = true;
    o_log_level_set(options.log_level);
    o_init();


    // protected
    void o__event_init(void);
    o__event_init();

    app_L.root = OObjRoot_new_pool("app");
    ODelcallback_new_assert(app_L.root, "app", "deleted!");
    app_L.root_threadsafe = OObjRoot_new_heap("app_threadsafe");
    OObj_threadsafe_set(app_L.root_threadsafe, true);
    ODelcallback_new_assert(app_L.root, "app_threadsafe", "deleted!");

    app_L.arena_allocator = o_allocator_arena_new(options.arena_size);
    app_L.arena = OObjRoot_new("a_arena", app_L.arena_allocator, false);
    app_L.arena_empty_used = o_allocator_arena_used(app_L.arena_allocator);
    o_arena_set(app_L.arena);

    app_L.tmp = OObj_new(app_L.root);
    OObj_name_set(app_L.tmp, "a_tmp");

    s_init(options.audio_enable, &options.audio_device_spec,
           options.mic_enable, &options.mic_device_spec);
    app_L.open_s_devices = options.audio_enable || options.mic_enable;
    app_L.open_audio_auto = options.audio_auto_open;
    app_L.open_mic_auto = options.mic_auto_open;

    app_L.paused_events = OArray_new_dyn(app_L.root, NULL, sizeof(struct app_paused_event_info), 0, 8);
    app_L.update_events = OArray_new_dyn(app_L.root, NULL, sizeof(struct app_update_event_info), 0, 8);

    if (options.create_view_stage) {
        app_L.main_view = AViewStage_new(app_L.root);
    }
    
    // swap context and create keys
    app_L.context_none = OContext_new(app_L.root, NULL);
    app_L.stage_context_idx = OContext_set_key(o_context(), "a_stage", NULL, NULL);    app_L.view_context_idx = OContext_set_key(o_context(), "a_view", NULL, NULL);
    app_L.scene_context_idx = OContext_set_key(o_context(), "a_scene", NULL, NULL);

    app_L.suspend_paused = true;
    app_L.audio_sound_auto_pause = true;

    // some start values for the smoothing
    app_L.fps = 60;
    app_L.load = app_L.load_render = 0.5;
    app_L.load_ms = app_L.load_render_ms = 5.0;

    // setup OpenGL usage
    o_log_debug_s(__func__,
                  "OpenGL minimal version: %d.%d", A_GL_MAJOR_VERSION, A_GL_MINOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, A_GL_MAJOR_VERSION);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, A_GL_MINOR_VERSION);
#ifdef MIA_OPTION_GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif

    // RGB 8 bit per channel
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);


    SDL_WindowFlags window_flags = SDL_WINDOW_OPENGL
                                   | SDL_WINDOW_RESIZABLE;
    if (options.mode == a_app_HEADLESS) {
        window_flags |= SDL_WINDOW_HIDDEN;
    }

#ifdef MIA_OPTION_SDL2
    // create window
    // SDL_WINDOW_ALLOW_HIGHDPI creates render bugs in web version?
    app_L.sdl_window = SDL_CreateWindow(options.title,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                    options.cols, options.rows,
                                    window_flags
    );
#else
    // SDL_WINDOW_HIGH_PIXEL_DENSITY creates render bugs in web version?
    window_flags |= SDL_WINDOW_HIGH_PIXEL_DENSITY;
    app_L.sdl_window = SDL_CreateWindow(options.title, options.cols, options.rows, window_flags);
#endif

    if (!app_L.sdl_window) {
        o_exit("failed creating a window: %s", SDL_GetError());
    }

    // Not necessary, but recommended to create a gl context:
    app_L.gl_context = SDL_GL_CreateContext(app_L.sdl_window);
    if (!app_L.gl_context) {
        o_exit("failed creating the OpenGL context: %s", SDL_GetError());
    }


#ifdef MIA_OPTION_GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        o_exit("glewInit failed: %s", glewGetErrorString(err));
    }
    o_log_debug_s(__func__,
                  "Using GLEW version: %s", glewGetString(GLEW_VERSION));
#endif

    // screen mode after OpenGL init, so vsync is set correctly
    SDL_SetWindowMinimumSize(app_L.sdl_window, 1, 1);
    a_app_screen_mode_set(options.mode);

    //
    // r init
    //
    r_init(app_L.root);

    // protected
    void a_pointer__init(void);
    a_pointer__init();

    // protected
    void a_input__init(void);
    a_input__init();
    
    // protected
    void a_rumble__init(void);
    a_rumble__init();


    // metrics
#ifdef NDEBUG
    app_L.metrics.enabled = false;
#else
    app_L.metrics.enabled = true;
#endif
    app_L.metrics.time = METRICS_TIME;


#ifdef MIA_OPTION_TESTS
    o_log_trace_s(__func__, "test...");

    // protected
    void o__test_main(void);
    o__test_main();
    o_log_debug_s(__func__, "test finished successfully");
#endif
}


static void handle_window_event(const SDL_Event *event)
{
#ifdef MIA_OPTION_SDL2
    if(event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
        o_log_s(__func__, "resumed");
        app_L.paused = false;
    } else if(event->window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
        o_log_s(__func__, "paused");
        app_L.paused = true;
    }
#else
    // SDL3
    if (event->type == SDL_EVENT_WINDOW_FOCUS_GAINED) {
        o_log_s(__func__, "resumed");
        app_L.paused = false;
    } else if (event->type == SDL_EVENT_WINDOW_FOCUS_LOST) {
        o_log_s(__func__, "paused");
        app_L.paused = true;
    }
#endif

    // call events
    osize paused_events_num = o_num(app_L.paused_events);
    for (int i = 0; i < paused_events_num; i++) {
        struct app_paused_event_info *info = OArray_at_void(app_L.paused_events, i);
        if (!OPtr_available(info->ptr)) {
            a_app_unregister_paused_event(info->obj, info->event);
            continue;
        }
        if (info->opt_context) {
            OContext_apply(info->opt_context);
        }
        info->event(info->obj, app_L.paused);
        if (info->opt_context) {
            OContext_apply(app_L.context_none);
        }
    }
}


static void block_for_fps_limit(void)
{
    app_L.limit_fps_time += (1.0 / MAX_FPS);
    double diff = app_L.limit_fps_time - app_L.time;
    // <= 0 safe -> will yield than, but that's ok
    o_sleep((osize) (diff * 1000));
}

// float out. fps, load*
static float smooth_out_value(float old_value, float new_value)
{
    if (m_isnan(new_value) || m_isinf(new_value)) {
        return old_value;
    }
    if (app_L.dt > SMOOTH_MIN_DT) {
        return new_value;
    }
    return m_mix(old_value, new_value, SMOOTH_ALPHA*app_L.dt);
}

static void main_loop(void)
{

    // Prepare and set None context
    // protected
    void a__view_set(oobj opt_view);
    void a__scene_set(oobj opt_scene);

    // stage is only set in main_loop, if main_view is a stage, so as main stage
    a_app_stage_set(OObj_check(app_L.main_view, AViewStage) ? app_L.main_view : NULL);
    a__scene_set(NULL);
    a__view_set(NULL);
    OContext_clone(app_L.context_none, o_context());


#ifdef MIA_PLATFORM_EMSCRIPTEN
    bool o_file_route__idbfs_update(void);
    bool idbfs_loaded = o_file_route__idbfs_update();
    if (!app_L.emscripten_file_init_done) {
        app_L.emscripten_file_init_done = idbfs_loaded;
        if (!app_L.emscripten_file_init_done) {
            // skipping frame until file system is init
            return;
        }
        o_log_s("app", "emscripten file system initialized");
    }
    if (idbfs_loaded) {
        void o_file__route_sync_loaded();
        o_file__route_sync_loaded();
    }
#endif


    if (app_L.audio_sound_auto_pause) {
        if (a_app_suspending()) {
            s_audio_pause();
        } else {
            s_audio_play();
        }
    }

    if (a_app_suspending()) {
        a_app_events_poll();
        if (app_L.suspend_force && app_L.opt_suspend_force_update_cb) {
            app_L.opt_suspend_force_update_cb(app_L.opt_suspend_force_update_obj);
        }
        return;
    }

    // to use default predictable printing and reading
    setlocale(LC_ALL, "C");

    // check full frame load time
    ou64 load_timer = o_timer();

    app_L.dt = o_timer_reset_s(&app_L.timer);

    if (o_unlikely(app_L.dt < 0 || app_L.dt >= MAX_DELTA_TIME)) {
        o_log_trace_s(__func__, "dropped frame: %g sec", app_L.dt);
        return;
    }

    app_L.time += app_L.dt;
    app_L.fps = o_clamp(smooth_out_value(app_L.fps, (float) (1.0 / app_L.dt)), 0, 240);


    // events should not render...
    // protected
    void o__event_run_callbacks(oobj undo_context);
    o__event_run_callbacks(app_L.context_none);

    // check if the main view is of type AViewStage and if so, shut down if scenes are empty
    if (o_unlikely(OObj_check(app_L.main_view, AViewStage) && AViewStage_scenes_num(app_L.main_view) == 0)) {
        app_L.running = false;
        return;
    }

    // sdl events
    a_app_events_poll();
    if (o_unlikely(!app_L.running)) {
        return;
    }

    // open audio on pointer down, else browsers may block the sound (emscripten build)
    if (o_unlikely(app_L.open_s_devices)) {
        // we need to use _back, the default a_pointer function needs an active AView!
        struct a_pointer p = a_pointer_back(0, 0);
        if (p.down) {
            app_L.open_s_devices = false;
            if (app_L.open_audio_auto) {
                s_audio_device_open();
            }
            if (app_L.open_mic_auto) {
                s_mic_device_open();
            }
        }
    }

    // window size (updated by the sdl event system, so a_app_events_poll() first...)
    // window coord size may differ from the actual pixel size, but is used for cursors (see a/pointer.c)
    ivec2 window_size;
    ivec2 window_coord_size;

    SDL_GetWindowSize(app_L.sdl_window, &window_coord_size.x, &window_coord_size.y);


#ifdef MIA_OPTION_SDL2
    SDL_GL_GetDrawableSize(app_L.sdl_window, &window_size.x, &window_size.y);
#else
    SDL_GetWindowSizeInPixels(app_L.sdl_window, &window_size.x, &window_size.y);
#endif

    bool size_changed = !ivec2_equals_v(window_size, app_L.back_size);
    app_L.back_size = window_size;
    app_L.sdl_window_coord_size = vec2_(m_2(window_coord_size));


    // begin a new frame
    // update and render the main view, which defaults to an instance of an AViewStage (to handle scenes)
    // RTex_clear_full may block to the next frame for some reason? Maybe gl is flushing here?
    r_frame_begin(m_2(window_size));
    RTex_clear_full(NULL, R_BLACK);


    if (size_changed) {
        *r_back_proj() = r_proj_new(window_size, -1, vec2_(180), true);
        // setting back buffers viewport (after r_frame_begin...)
        app_L.viewport_unsafe = ivec4_(0, 0, m_2(window_size));
        RTex_viewport_set(NULL, app_L.viewport_unsafe);

        // viewport safe area
#ifdef MIA_OPTION_SDL2
        app_L.viewport_safe = app_L.viewport_unsafe;
#else
        SDL_Rect rect;
        bool valid = SDL_GetWindowSafeArea(app_L.sdl_window, &rect);
        if (!valid) {
            app_L.viewport_safe = app_L.viewport_unsafe;
        } else {
            vec4 padding = vec4_(
                (float) rect.x / app_L.sdl_window_coord_size.x,
                (float) rect.y / app_L.sdl_window_coord_size.y,
                (float) (app_L.sdl_window_coord_size.x-rect.w-rect.x) / app_L.sdl_window_coord_size.x,
                (float) (app_L.sdl_window_coord_size.y-rect.h-rect.y) / app_L.sdl_window_coord_size.y
            );

            // max. 10% each
            padding = vec4_clamp(padding, 0, 0.1);

            app_L.viewport_safe = ivec4_(
                padding.v0 * window_size.x,
                padding.v1 * window_size.y,
                (1.0f-padding.v2-padding.v0) * window_size.x,
                (1.0f-padding.v3-padding.v1) * window_size.y);
        }
#endif

#ifdef DEBUG_UNSAFE_PADDING
        app_L.viewport_safe = app_L.viewport_unsafe;
        app_L.viewport_safe.left += DEBUG_UNSAFE_PADDING.v0;
        app_L.viewport_safe.top += DEBUG_UNSAFE_PADDING.v1;
        app_L.viewport_safe.width -= DEBUG_UNSAFE_PADDING.v0 + DEBUG_UNSAFE_PADDING.v2;
        app_L.viewport_safe.height -= DEBUG_UNSAFE_PADDING.v1 + DEBUG_UNSAFE_PADDING.v3;
#endif


#ifdef MIA_OPTION_SDL2
        o_log_debug_s("main_loop",
                   "window size changed, "
                   "size: %i:%i; coord_size: %i:%i",
                   m_2(window_size), m_2(window_coord_size));
#else
        float pixel_density = SDL_GetWindowPixelDensity(app_L.sdl_window);
        o_log_debug_s("main_loop",
                      "window size changed, "
                      "size: %i:%i; coord_size: %i:%i; density: %f",
                      m_2(window_size), m_2(window_coord_size), pixel_density);
#endif
    }

    // call pre update events
    osize update_events_num = o_num(app_L.update_events);
    for (int i = 0; i < update_events_num; i++) {
        struct app_update_event_info *info = OArray_at_void(app_L.update_events, i);
        if (!OPtr_available(info->ptr)) {
            a_app_unregister_update_event(info->obj, info->event);
            continue;
        }
        if (info->opt_context) {
            OContext_apply(info->opt_context);
        }
        info->event(info->obj);
        if (info->opt_context) {
            OContext_apply(app_L.context_none);
        }
    }

    // actual update call chain
    AView_update(app_L.main_view, NULL, RTex_viewport(NULL));

    ou64 load_render_timer = o_timer();

    // actual render call chain
    AView_render(app_L.main_view, NULL);

    // delete tmp allocated stuff
    o_del(app_L.tmp);
    app_L.tmp = OObj_new(app_L.root);
    OObj_name_set(app_L.tmp, "a_tmp");

    // clear arena, if it was used
    app_L.arena_used = o_allocator_arena_used(app_L.arena_allocator);
    if (app_L.arena_used != app_L.arena_empty_used) {
        o_del(app_L.arena);
        o_allocator_arena_clear(app_L.arena_allocator);
        app_L.arena = OObjRoot_new("a_arena", app_L.arena_allocator, false);
        app_L.arena_empty_used = o_allocator_arena_used(app_L.arena_allocator);
    }
    o_arena_set(app_L.arena);


    // update sound stuff, mostly mic callback
    s_update();

    // finishes pending draw calls
    r_frame_end();

    // measure load before swapping window, which will block on some platforms for the next vsync
    ou64 load_end_timer = o_timer();

    double load_time = o_timer_diff_s(load_timer, load_end_timer);
    double load_render_time = o_timer_diff_s(load_render_timer, load_end_timer);

    app_L.load = m_clamp(smooth_out_value(app_L.load, load_time / app_L.dt), 0, 1);
    app_L.load_render = m_clamp(smooth_out_value(app_L.load_render, load_render_time / app_L.dt), 0, 1);

    app_L.load_ms = m_max(smooth_out_value(app_L.load_ms, load_time * 1000.0f), 0);
    app_L.load_render_ms = m_clamp(smooth_out_value(app_L.load_render_ms, load_render_time * 1000.0f), 0,
                                   app_L.load_ms);


#if !defined(NDEBUG) && defined(MIA_OPTION_LOG_DEBUG) && defined(MIA_OPTION_METRICS)
    if (app_L.metrics.enabled && o_log_level() >= O_LOG_DEBUG) {
        app_L.metrics.time -= app_L.dt;
        if (app_L.metrics.time <= 0) {
            app_L.metrics.time += METRICS_TIME;

#ifdef MIA_OPTION_METRICS_SYS
            a_app_log_metrics(NULL, O_LOG_DEBUG, true);
#else
            a_app_log_metrics(NULL, O_LOG_DEBUG, false);
#endif
        }
    }
#endif


    // end the frame
    // may block until next frame...
    SDL_GL_SwapWindow(app_L.sdl_window);

    r_error_check("frame finished");
}

//
// public
//

// runs main deferred in the main loop
static void main_deferred(oobj obj)
{
    a_app_main_fn main_fn = *((a_app_main_fn *) o_user(obj));
    main_fn(a_root());
}

void a_app_run(a_app_main_fn main_fn, const struct a_app_run_options *opt_options)
{
    init(opt_options);

    //
    // add main_fn as OEvent to run it in the main loop first
    //
    oobj main_event = OEvent_new(a_root(), main_deferred, NULL);
    // clone into a new alloacted buffer, because a function pointer != void * (sizeof...)
    o_user_set(main_event, o_new_clone(main_event, &main_fn, a_app_main_fn, 1));
    OEvent_post(main_event);

    //
    // start main_loop
    //

    o_log_s(__func__, "run");

    app_L.running = true;
    app_L.timer = o_timer();

#ifdef MIA_PLATFORM_EMSCRIPTEN
    emscripten_set_main_loop(main_loop, 0, true);
#else
    while (app_L.running) {
        main_loop();
#if defined(MIA_PLATFORM_LINUX) || defined(MIA_PLATFORM_CXXDROID)
        block_for_fps_limit();
#endif
    }
#endif

    //
    // app finished when this code is reached!
    //

    // protected
    void o__sanitizer_leak_check(const char *why, bool full);
    o__sanitizer_leak_check("App finished", true);

    SDL_DestroyWindow(app_L.sdl_window);

    // todo quit sdl and ports somewhere?
    /*
#ifdef OPTION_TTF
    TTF_Quit();
#endif
    IMG_Quit();
    SDL_Quit();
     */
}


struct a_app_run_options a_app_run_options_default(void)
{
    struct a_app_run_options options = {0};
    options.title = O_TO_STRING(MIA_OPTION_TITLE);
    options.cols = 2 * 180;
    options.rows = 2 * 180 * 16 / 9;

#ifdef MIA_OPTION_INIT_FULLSCREEN
    options.mode = a_app_FULLSCREEN;
#else
    options.mode = a_app_WINDOW;
#endif
    options.audio_enable = true;
    options.audio_auto_open = true;
    options.mic_enable = false;
    options.mic_auto_open = false;
    options.audio_device_spec = s_spec_default();
    options.mic_device_spec = options.audio_device_spec;
    options.mic_device_spec.channels = 1;
    options.create_view_stage = true;
    options.arena_size = 1024 * 1024;
    options.log_level = O_LOG_DEBUG;
    return options;
}


bool a_app_events_poll(void)
{
    // protected
    void a_pointer__update(void);
    void a_pointer__update_events_handled(void);
    void a_pointer__handle_event_mouse(SDL_Event *event);
    void a_pointer__handle_event_touch(SDL_Event *event);
    void a_pointer__handle_event_pen(SDL_Event *event);
    void a_input__update(void);
    void a_input__handle_event_key(SDL_Event *event);
    void a_input__handle_event_wheel(SDL_Event *event);
    void a_input__handle_event_gamepad(SDL_Event *event);
    void a_action__update(void);

    // resets before the events are handled
    a_action__update();
    a_pointer__update();
    a_input__update();


    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            app_L.running = false;
            return app_L.running;
        }

        switch (event.type) {
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
            case SDL_EVENT_MOUSE_MOTION:
            case SDL_EVENT_MOUSE_BUTTON_UP:
                a_pointer__handle_event_mouse(&event);
                break;
            case SDL_EVENT_FINGER_DOWN:
            case SDL_EVENT_FINGER_MOTION:
            case SDL_EVENT_FINGER_UP:
                a_pointer__handle_event_touch(&event);
                break;
#ifndef MIA_OPTION_SDL2
            case SDL_EVENT_PEN_DOWN:
            case SDL_EVENT_PEN_UP:
            case SDL_EVENT_PEN_MOTION:
            case SDL_EVENT_PEN_AXIS:
                a_pointer__handle_event_pen(&event);
                break;
#endif
            case SDL_EVENT_KEY_DOWN:
            case SDL_EVENT_KEY_UP:
                a_input__handle_event_key(&event);
                break;
            case SDL_EVENT_MOUSE_WHEEL:
                a_input__handle_event_wheel(&event);
                break;
#ifdef MIA_OPTION_SDL2
            case SDL_WINDOWEVENT:
#else
            case SDL_EVENT_WINDOW_FOCUS_GAINED:
            case SDL_EVENT_WINDOW_FOCUS_LOST:
#endif
                handle_window_event(&event);
                break;
#ifdef MIA_OPTION_GAMEPAD
            case SDL_EVENT_GAMEPAD_ADDED:
            case SDL_EVENT_GAMEPAD_REMOVED:
            case SDL_EVENT_GAMEPAD_REMAPPED:
                a_input__handle_event_gamepad(&event);
                break;
#endif
        }
    }

    a_pointer__update_events_handled();

    return app_L.running;
}

void a_app_events_handle(void)
{
    a_pointer_handled_ex(-1, -1, true);
}

void a_app_log_metrics(const char *tag, int log_level, bool log_sys)
{
    tag = o_or(tag, "metrics");

    osize arena_size = o_allocator_arena_size(app_L.arena_allocator);

    // fps and load
    o_log_base(log_level, "a", NULL, 0, tag,
               "fps: %.1f | cpu: %.1f%%(%.1fms) + gpu: %.1f%%(%.1fms) = total: %.1f%%(%.1fms)",
               app_L.fps,
               (app_L.load - app_L.load_render) * 100.0f,
               (app_L.load_ms - app_L.load_render_ms),
               app_L.load_render * 100.0f,
               app_L.load_render_ms,
               app_L.load * 100.0f,
               app_L.load_ms);
    o_log_base(log_level, "a", NULL, 0, tag,
              "drawcalls: %i | arena: %.2f%%(%"osize_PRI"B)",
              r_draw_calls(),
              100.0f * app_L.arena_used / arena_size,
              app_L.arena_used
              );

    OObjRoot__list_lock_block {
        osize roots = OObjRoot__list_num();
        for (osize i = 0; i < roots; i++) {
            oobj r = OObjRoot__list_at(i).o;
            assert(r);

            if (!log_sys && o_str_begins(OObj_name(r), "[mia]")) {
                continue;
            }

            osize oobjs, datas, depths;
            OObj_tree_metrics(r, &oobjs, &datas, &depths);

            // clangs format strings may crash if using osize multiple times with just %i/%d
            if (OObj_allocator(r).type == O_ALLOCATOR_TYPE_POOL) {
                int blocks = o_allocator_pool_blocks_used(OObj_allocator(r));
                o_log_base(log_level, "a", NULL, 0, tag,
                           "[%i/%i] <%s> (pool): oobjs: %i | datas: %i | depths: %i | blocks: %i",
                           (int) i, (int) roots, OObj_name(r), (int) oobjs, (int) datas, (int) depths, (int) blocks);
            } else {
                o_log_base(log_level, "a", NULL, 0, tag,
                           "[%i/%i] <%s> (heap): oobjs: %i | datas: %i | depths: %i",
                           (int) i, (int) roots, OObj_name(r), (int) oobjs, (int) datas, (int) depths);
            }
        }
    }
}

bool a_app_auto_log_metrics(void)
{
    return app_L.metrics.enabled;
}

bool a_app_auto_log_metrics_set(bool set)
{
#ifndef NDEBUG
    app_L.metrics.enabled = set;
#endif
    return app_L.metrics.enabled;
}

enum a_app_screen_mode a_app_screen_mode(void)
{
    return app_L.screen_mode;
}

enum a_app_screen_mode a_app_screen_mode_set(enum a_app_screen_mode set)
{
#ifdef MIA_PLATFORM_EMSCRIPTEN
    set = a_app_WINDOW;
#endif
#ifdef MIA_PLATFORM_ANDROID
    set = a_app_FULLSCREEN;
#endif
#ifdef MIA_PLATFORM_CXXDROID
    if(set == a_app_MAXIMIZED || set == a_app_HEADLESS) {
        set = a_app_WINDOW;
    }
#endif

    if (set == a_app_HEADLESS) {
        o_log_debug_s(__func__, "headless window mode");
        SDL_HideWindow(app_L.sdl_window);

        // vsync off
        SDL_GL_SetSwapInterval(0);
    } else {
        // not headless, so in a window or maximized
        Uint32 sdl_mode;
        if (set == a_app_MAXIMIZED) {
            sdl_mode = SDL_WINDOW_MAXIMIZED;
        } else if (set == a_app_FULLSCREEN) {
            sdl_mode = SDL_WINDOW_FULLSCREEN;
        } else {
            sdl_mode = 0;
            set = a_app_WINDOW;
        }
        o_log_debug_s(__func__, "window mode = %i", set);
        SDL_ShowWindow(app_L.sdl_window);
        SDL_SetWindowFullscreen(app_L.sdl_window, sdl_mode);

        // vsync on
        if (SDL_GL_SetSwapInterval(-1) != O_SDL_OK) {
            o_log_s(__func__, "Adaptive vsync failed, trying normal vsync");
            if (SDL_GL_SetSwapInterval(1) != O_SDL_OK) {
                o_log_s(__func__, "Normal vsync also failed...");
            }
        }
    }
    app_L.screen_mode = set;
    return app_L.screen_mode;
}

// protected
void a_app__title_set(const char *set)
{
    set = o_or(set, O_TO_STRING(MIA_OPTION_TITLE));
    SDL_SetWindowTitle(app_L.sdl_window, set);
}

// protected
void *a_app__icon_create(const struct o_img *set)
{
    SDL_Surface *surface = o_img_as_sdl_surface(*set);
    if (!surface) {
        o_log_error_s(__func__, "invalid image");
        return NULL;
    }

    // we use sdl's cpu resizing here, cause for these small images its around 6x faster

    // resize to 32x32 if smaller
    if (surface->w < 32 || surface->h < 32) {
#ifdef MIA_OPTION_SDL2
        SDL_Surface *tmp = SDL_CreateRGBSurfaceWithFormat(0, 32, 32, 32, SDL_PIXELFORMAT_RGBA32);
        SDL_BlitScaled(surface, NULL, tmp, NULL);
#else
        SDL_Surface *tmp = SDL_CreateSurface(32, 32, SDL_PIXELFORMAT_RGBA32);
        SDL_BlitSurfaceScaled(surface, NULL, tmp, NULL, SDL_SCALEMODE_NEAREST);
#endif
        SDL_DestroySurface(surface);
        surface = tmp;
    }

#ifndef MIA_OPTION_SDL2
    // SDL3 allows for alternative resolutions
    SDL_Surface *surface_64 = SDL_CreateSurface(64, 64, SDL_PIXELFORMAT_RGBA32);
    SDL_Surface *surface_128 = SDL_CreateSurface(128, 128, SDL_PIXELFORMAT_RGBA32);
    SDL_BlitSurfaceScaled(surface, NULL, surface_64, NULL, SDL_SCALEMODE_NEAREST);
    SDL_BlitSurfaceScaled(surface, NULL, surface_128, NULL, SDL_SCALEMODE_NEAREST);
    SDL_AddSurfaceAlternateImage(surface, surface_64);
    SDL_AddSurfaceAlternateImage(surface, surface_128);
    SDL_DestroySurface(surface_64);
    SDL_DestroySurface(surface_128);
#endif

    return surface;
}

// protected
void a_app__icon_free(void *surface_icon)
{
    SDL_DestroySurface(surface_icon);
}

// protected
void a_app__icon_set(void *surface_icon)
{
    SDL_SetWindowIcon(app_L.sdl_window, surface_icon);
}

bool a_app_audio_sound_auto_pause(void)
{
    return app_L.audio_sound_auto_pause;
}

bool a_app_audio_sound_auto_pause_set(bool set)
{
    app_L.audio_sound_auto_pause = set;
    return app_L.audio_sound_auto_pause;
}

ivec4 a_app_viewport_unsafe(void)
{
    return app_L.viewport_unsafe;
}

ivec4 a_app_viewport_safe(void)
{
    return app_L.viewport_safe;
}

bool a_app_paused(void)
{
    return app_L.paused;
}

bool a_app_suspend_paused(void)
{
    return app_L.suspend_paused;
}

bool a_app_suspend_paused_set(bool suspend_during_pause)
{
    app_L.suspend_paused = suspend_during_pause;
    return suspend_during_pause;
}

bool a_app_suspend_force(void)
{
    return app_L.suspend_force;
}

bool a_app_suspend_force_set(bool suspend_force, OObj__event_fn opt_update_cb, oobj opt_update_obj)
{
    app_L.suspend_force = suspend_force;
    app_L.opt_suspend_force_update_cb = opt_update_cb;
    app_L.opt_suspend_force_update_obj = opt_update_obj;
    return app_L.suspend_force;
}

void a_app_register_paused_event(oobj obj, a_app_paused_event event, oobj opt_context)
{
    OObj_assert(obj, OObj);
    o_thread_assert_main();

    // check if already available
    for (osize i = 0; i < OArray_num(app_L.paused_events); i++) {
        struct app_paused_event_info *info = o_at(app_L.paused_events, i);
        if (info->obj == obj && info->event == event) {
            o_log_warn_s(__func__, "already registered");
            return;
        }
    }
    struct app_paused_event_info add = {0};
    add.event = event;
    add.obj = obj;
    add.ptr = OPtr_new(app_L.root, obj);
    if (opt_context) {
        add.opt_context = OContext_new(add.ptr, opt_context);
    }
    OArray_push(app_L.paused_events, &add);
    o_log_debug_s(__func__, "registered: %p", obj);
}

void a_app_unregister_paused_event(oobj obj, a_app_paused_event event)
{
    OObj_assert(obj, OObj);
    o_thread_assert_main();

    int found = 0;
    for (osize i = 0; i < OArray_num(app_L.paused_events); i++) {
        struct app_paused_event_info *info = o_at(app_L.paused_events, i);
        if (info->obj == obj && info->event == event) {
            o_del(info->ptr);
            OArray_pop_at(app_L.paused_events, i, NULL);
            i--;
            found++;
            o_log_debug_s(__func__, "unregistered: %p", obj);
        }
    }
    if (found == 0) {
        o_log_warn_s(__func__, "not registered");
    } else if (found > 1) {
        o_log_warn_s(__func__, "multiple entries found");
    }
}

void a_app_register_update_event(oobj obj, OObj__event_fn event, oobj opt_context)
{
    OObj_assert(obj, OObj);
    o_thread_assert_main();

    // check if already available
    for (osize i = 0; i < OArray_num(app_L.update_events); i++) {
        struct app_update_event_info *info = o_at(app_L.update_events, i);
        if (info->obj == obj && info->event == event) {
            o_log_warn_s(__func__, "already registered");
            return;
        }
    }
    struct app_update_event_info add = {0};
    add.event = event;
    add.obj = obj;
    add.ptr = OPtr_new(app_L.root, obj);
    if (opt_context) {
        add.opt_context = OContext_new(add.ptr, opt_context);
    }
    OArray_push(app_L.update_events, &add);
    o_log_debug_s(__func__, "registered: %p", obj);
}

/**
 * Unregisters a previous registered event on that object, logs on failure
 */
void a_app_unregister_update_event(oobj obj, OObj__event_fn event)
{
    OObj_assert(obj, OObj);
    o_thread_assert_main();

    int found = 0;
    for (osize i = 0; i < OArray_num(app_L.update_events); i++) {
        struct app_update_event_info *info = o_at(app_L.update_events, i);
        if (info->obj == obj && info->event == event) {
            o_del(info->ptr);
            OArray_pop_at(app_L.update_events, i, NULL);
            i--;
            found++;
            o_log_debug_s(__func__, "unregistered: %p", obj);
        }
    }
    if (found == 0) {
        o_log_warn_s(__func__, "not registered");
    } else if (found > 1) {
        o_log_warn_s(__func__, "multiple entries found");
    }
}


oobj a_app_main_view(void)
{
    return app_L.main_view;
}

void a_app_main_view_set(oobj set)
{
    OObj_assert(set, AView);
    app_L.main_view = set;
}

void a_app_stage_set(oobj stage)
{
    if (stage) {
        OObj_assert(stage, AViewStage);
    }
    app_L.stage = stage;
    OContext_set_at(o_context(), app_L.stage_context_idx, a_app_stage_set, stage);
}


vec2 a_app__sdl_window_coord_size(void)
{
    return app_L.sdl_window_coord_size;
}


//
// a/common.h
//


oobj a_root(void)
{
    assert(app_L.root && "call a_app_init first");
    return app_L.root;
}

oobj a_root_threadsafe(void)
{
    assert(app_L.root_threadsafe && "call a_app_init first");
    return app_L.root_threadsafe;
}

oobj a_tmp(void)
{
    o_thread_assert_main();
    assert(app_L.tmp && "call a_app_init first");
    return app_L.tmp;
}

oobj a_arena(void)
{
    o_thread_assert_main();
    assert(app_L.arena && "call a_app_init first");
    return app_L.arena;
}

double a_dt(void)
{
    return app_L.dt;
}

ou64 a_timer(void)
{
    return app_L.timer;
}

double a_time(void)
{
    return app_L.time;
}

float a_fps(void)
{
    return app_L.fps;
}

float a_load(void)
{
    return app_L.load;
}

float a_load_ms(void)
{
    return app_L.load_ms;
}

float a_load_render(void)
{
    return app_L.load_render;
}

float a_load_render_ms(void)
{
    return app_L.load_render_ms;
}


int a_pool_blocks_num(void)
{
    return o_allocator_pool_blocks_num(
        OObj_allocator(app_L.root));
}


int a_pool_blocks_used(void)
{
    return o_allocator_pool_blocks_used(
        OObj_allocator(app_L.root));
}

void a__view_set(oobj opt_view)
{
    app_L.view = opt_view;
    OContext_set_at(o_context(), app_L.view_context_idx, a__view_set, opt_view);

}

struct oobj_opt a_view_try(void)
{
    o_thread_assert_main();
    return oobj_opt(app_L.view);
}

void a__scene_set(oobj opt_scene)
{
    app_L.scene = opt_scene;
    OContext_set_at(o_context(), app_L.scene_context_idx, a__scene_set, opt_scene);
}

struct oobj_opt a_scene_try(void)
{
    o_thread_assert_main();
    return oobj_opt(app_L.scene);
}

struct oobj_opt a_stage_try(void)
{
    return oobj_opt(app_L.stage);
}

void a_scene_exit(void)
{
    AViewStage_scene_exit(a_stage());
}

void a_scene_exit_pop(void)
{
    AViewStage_scene_exit_pop(a_stage());
}

oobj a_cam(void)
{
    return AView_cam(a_view());
}
