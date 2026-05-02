#ifndef A_COMMON_H
#define A_COMMON_H

/**
 * @file common.h
 *
 * Common used app stuff
 */


#include "s/common.h"
#include "m/types/flt.h"

// GLES and WebGL use GLES3.0
#ifdef MIA_OPTION_GLES
#define A_GL_MAJOR_VERSION 3
#define A_GL_MINOR_VERSION 0

// GL uses GL3.3
#else
#define A_GL_MAJOR_VERSION 3
#define A_GL_MINOR_VERSION 3
#endif


/**
 * @return the main app object root to allocate on.
 *         Use scene objects to allocate for scenes as default...
 *         Typically uses a pool allocator (not threadsafe).
 * @note Will init a_app on first call, which will be done in AScene_new as an example.
 */
oobj a_root(void);

/**
 * @return An additional root for a, which uses a threadsafe heap allocator.
 */
oobj a_root_threadsafe(void);


/**
 * @return OObj that gets auto deleted after the frame.
 *         Use this to allocate tmp stuff during a frame.
 * @note asserts main thread.
*/
oobj a_tmp(void);

/**
 * @return an arena allocator OObjRoot to allocate on. Automatically deleted / cleared each frame.
 * @note will also be set as o_arena for the arena str versions like o_strf_a.
 *       asserts main thread.
 * @warning An arena has a fixed allcoation size, should be used for small stuff and not full images, etc.
 *          Only cleared / o_del'ed if used.
 */
oobj a_arena(void);



/**
 * @return current delta time, may differ in update calls.
 * @note dt is updated in (and after) update calls.
 *       so event function has the old time.
 */
double a_dt(void);

/**
 * o_timer() time of the frame begin.
 */
ou64 a_timer(void);

/**
 * @return current app time, may differ in update calls.
 * @note dt is updated in (and after) update calls.
 *       so event function has the old time.
 */
double a_time(void);

/**
 * @return current fps.
 * @note smoothed.
 */
float a_fps(void);

/**
 * Load is the amount of time spend in ratio to the full frame time.
 * @return current load (all stuff except for waiting for next frame of course) [%].
 * @note smoothed.
 */
float a_load(void);

/**
 * Load [ms] is the amount of time spend in millis.
 * @return current load (all stuff except for waiting for next frame of course) [ms].
 * @note smoothed.
 */
float a_load_ms(void);

/**
 * Load is the amount of time spend in ratio to the full frame time.
 * Here only for the time spend in render functions.
 * @return current load of render calls [%].
 * @note smoothed.
 */
float a_load_render(void);

/**
 * Load [ms] is the amount of time spend in millis.
 * Here only for the time spend in render functions.
 * @return current load of render calls [ms]
 * @note smoothed.
 */
float a_load_render_ms(void);

/**
 * Load is the amount of time spend in ratio to the full frame time.
 * Here only for the time spend in NOT render functions.
 * @return current load of NOT render calls [%].
 * @note smoothed.
 */
o_inline float a_load_update(void)
{
    return a_load() - a_load_render();
}

/**
 * Load [ms] is the amount of time spend in millis.
 * Here only for the time spend in NOT render functions.
 * @return current load of NOT render calls [ms].
 * @note smoothed.
 */
o_inline float a_load_update_ms(void)
{
    return a_load_ms() - a_load_render_ms();
}

/**
 * @return the total number of allocated pool blocks.
 */
int a_pool_blocks_num(void);

/**
 * @return the number of used pool blocks.
 * @note if this grows each frame, you may have a memory leak.
 */
int a_pool_blocks_used(void);

/**
 * @return The current active AView.
 * @note asserts that a view is currently active.
 *       asserts main thread.
 */
struct oobj_opt a_view_try(void);

/**
 * @return The current active AView.
 * @note asserts that a view is currently active.
 *       asserts main thread.
 */
o_inline oobj a_view(void)
{
    oobj current_view = a_view_try().o;
    assert(current_view && "no active AView");
    return current_view;
}

/**
 * @return The current active AScene.
 * @note asserts that a scene is currently active.
 *       asserts main thread.
 */
struct oobj_opt a_scene_try(void);

/**
 * @return The current active AScene.
 * @note asserts that a scene is currently active.
 *       asserts main thread.
 */
o_inline oobj a_scene(void)
{
    oobj current_scene = a_scene_try().o;
    assert(current_scene && "no active AScene");
    return current_scene;
}

/**
 * Calls AViewStage_scene_exit() with a_stage().
 * Deletes the current scene index (AViewStage_scene_index) and all down the stack.
 * Defers it to the next frame and calls AViewStage_scene_exit_instant then.
 * @note logs an error on failure.
 *       asserts main thread.
 * @sa AViewStage.h, a_stage(), AScene_exit()
 */
void a_scene_exit(void);

/**
 * Calls AViewStage_scene_exit_pop() with a_stage().
 * Deletes the given scene index while keeping all down the stack.
 * USE WITH CAUTION!
 * All scenes down should not have been some sort of parent to this scene nor should they depend on it.
 * Typical examples are intro scenes or other tmp inbetween scenes.
 * Defers it to the next frame and calls AViewStage_scene_exit_pop_instant_for then.
 * @note logs an error on failure.
 *       asserts main thread.
 * @sa AViewStage.h, a_stage(), AScene_exit_pop()
 */
void a_scene_exit_pop(void);


/**
 * @return AViewStage of the current running active stage.
 */
struct oobj_opt a_stage_try(void);


/**
 * @return AViewStage of the current running stage.
 * @note asserts that a stage is currently active.
 */
o_inline oobj a_stage(void)
{
    oobj stage = a_stage_try().o;
    assert(stage && "no active AViewStage");
    return stage;
}

/**
 * @return The current active AView's RCam.
 * @note asserts that a view is currently active.
 *       Just calls AView_cam(a_app_view());
 *       asserts main thread.
 */
oobj a_cam(void);


#endif //A_COMMON_H
