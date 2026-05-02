#ifndef A_APP_H
#define A_APP_H

/**
 * @file app.h
 *
 * Starts up the app, creating contexts, etc.
 * Handles the scene stack.
 *
 * Each scene may have some options in other modules.
 * Instead of passing a scene object or something,
 * the current scene registers in the app and other
 * modules check grab the current scene in their functions.
 *
 * This file contains functions in either the namespace a_app or simply the library namespace a
 */


#include "o/OObj.h"
#include "a/common.h"
#include "m/types/int.h"

// forwarded from o/img.h
struct o_img;

/**
 * Different screen_modes, not all are available on some platforms
 */
enum a_app_screen_mode {
    a_app_WINDOW,
    a_app_MAXIMIZED,
    a_app_FULLSCREEN,
    a_app_HEADLESS,
    a_app_sceen_mode_ENUM_MAX
};

struct a_app_run_options {
    const char *title;
    int cols, rows;
    enum a_app_screen_mode mode;
    bool audio_enable, audio_auto_open;
    bool mic_enable, mic_auto_open;
    struct s_spec audio_device_spec;
    struct s_spec mic_device_spec;
    bool create_view_stage;
    osize arena_size;
    int log_level;
};

/**
 * main function to create scenes, etc.
 * the app runs until all scenes are deleted, so if no scene is created, the main loop will get stopped.
 * @param root object == a_app_root
 */
typedef void (*a_app_main_fn)(oobj root);

/**
 * Event that may be registered with a_app_register_paused_event
 * @param obj OObj that was passed at event registration
 * @param paused true if pause begins, false if pause ends
 */
typedef void (*a_app_paused_event)(oobj obj, bool paused);


/**
 * Runs the main loop until an exit event is received, or all scenes are deleted (a_app_scene_exit(0))
 * @param main_fn runs with a, r, m, o init called first (deferred in the main loop)
 * @param opt_options custom start options, if not NULL
 */
void a_app_run(a_app_main_fn main_fn, const struct a_app_run_options *opt_options);

/**
 * @return default run options
 *         will enable audio, but disable mic
 */
struct a_app_run_options a_app_run_options_default(void);


/**
 * Automatically called on each frame.
 * If you use the headless mode, call it here and then to poll the events and polls em.
 * @return true if the app is still running (may get false if quit event was polled)
 */
bool a_app_events_poll(void);

/**
 * Sets all events / inputs that may be handled to be handled.
 * At first just calls a_pointer_handled(-1, -1) to handle all pointers
 */
void a_app_events_handle(void);


/**
 * Logs app metrics.
 * Is not affected by the MIA_OPTION_METRICS (etc.) options for auto metrics
 * @param tag smth like __func__ or NULL for "a_app_log_metrics"
 * @param log_level O_LOG_INFO, etc.
 * @param log_sys true to also log system roots (like sound)
 */
void a_app_log_metrics(const char *tag, int log_level, bool log_sys);

/**
 * @return true if log level debug will print metrics
 */
bool a_app_auto_log_metrics(void);

/**
 * @return true if log level debug will print metrics
 */
bool a_app_auto_log_metrics_set(bool set);


/**
 * @return the current screen mode like a_app_MODE_FULLSCREEN
 */
enum a_app_screen_mode a_app_screen_mode(void);


/**
 * @param set the current screen mode like a_app_MODE_FULLSCREEN
 * @note not all platforms support all modes, the nearest is choosen than, for example:
 *       MIA_PLATFORM_EMSCRIPTEN is always a_app_MODE_FULLSCREEN
 */
enum a_app_screen_mode a_app_screen_mode_set(enum a_app_screen_mode set);


/**
 * @return If true (default), the sound will get paused while suspending and played when regained focus
 */
bool a_app_audio_sound_auto_pause(void);

/**
 * @return If true (default), the sound will get paused while suspending and played when regained focus
 */
bool a_app_audio_sound_auto_pause_set(bool set);


/**
 * The unsafe zone contains the full display viewport.
 * @return ltwh position of the back buffers display
 * @sa a_app_viewport_safe
 */
ivec4 a_app_viewport_unsafe(void);


/**
 * Modern devices offer a safe zone, outside from it are for example the top camera notch / holes, etc.
 * In the safe zone, the hud is safe to interact with and should be visible.
 * Outside the safe zone, the gameplay may still be rendered, such as the background or level, etc.
 * By default, AView's disable the usage of unsafe viewports and set the AView's viewport within the safe zone
 * To enable it, see AView_allow_unsafe()
 * Also have a look at AView_safe_padding() if its allowed
 * The padding or unsafe border is clamped to be max. 10% of the display width / height for each border.
 *     So worst case is a safe zone with 80% width and 80% height in contrast to the unsafe zone.
 * @return ltwh position of a safe zone for the back buffers display
 */
ivec4 a_app_viewport_safe(void);



/**
 * @return true if focus is lost and the app is in pause mode.
 */
bool a_app_paused(void);

/**
 * @return true (default) if the app should skip working during pause
 */
bool a_app_suspend_paused(void);

/**
 * @return true (default) if the app should skip working during pause
 */
bool a_app_suspend_paused_set(bool suspend_during_pause);


/**
 * @return true if currently forced to suspend (see a_app_suspend_force_set)
 */
bool a_app_suspend_force(void);

/**
 * Forces an app suspend.
 * Calls opt_update_cb if not NULL on each update call while active (passing opt_update_obj)
 */
bool a_app_suspend_force_set(bool suspend_force, OObj__event_fn opt_update_cb, oobj opt_update_obj);

/**
 * @return true if the app is in pause mode and should suspend
 */
o_inline bool a_app_suspending(void)
{
    // protected
    bool o__ignore_pause(void);

    return (a_app_paused() && a_app_suspend_paused() && !o__ignore_pause()) || a_app_suspend_force();
}

/**
 * Registers a paused event that is called when the app goes into pause, or resumes from there
 * @param obj OObj to be passed with the event
 * @param event called with obj
 * @param opt_context optional OContext object to be applied before, will be clones, like o_context()
 * @note asserts main thread
 */
void a_app_register_paused_event(oobj obj, a_app_paused_event event, oobj opt_context);

/**
 * Unregisters a previous registered event on that object, logs on failure
 * @note asserts main thread
 */
void a_app_unregister_paused_event(oobj obj, a_app_paused_event event);

/**
 * Registers an update event called before the app updates the main_view
 * @param obj OObj to be passed with the event
 * @param opt_context optional OContext object to be applied before, will be clones, like o_context()
 * @note asserts main thread
 */
void a_app_register_update_event(oobj obj, OObj__event_fn event, oobj opt_context);

/**
 * Unregisters a previous registered event on that object, logs on failure
 * @note asserts main thread
 */
void a_app_unregister_update_event(oobj obj, OObj__event_fn event);


/**
 * @return AView object as main view, normally it's the default AViewStage ( a_stage() )
 */
oobj a_app_main_view(void);

/**
 * @param set AView object as main view, normally it's the default AViewStage ( a_stage() )
 */
void a_app_main_view_set(oobj set);


/**
 * Sets the current active stage, so that AScene's get correctly installed ( a_stage() )
 * @param stage AViewStage object or NULL
 */
void a_app_stage_set(oobj stage);

#endif //A_APP_H
