#ifndef A_AVIEWSTAGE_H
#define A_AVIEWSTAGE_H

/**
 * @file AViewStage.h
 *
 * Object
 *
 */


#include "a/AView.h"
#include "a/AScene.h"

/** object id */
#define AViewStage_ID AView_ID "Stage."



typedef struct {
    AView super;

    // internal OArray (not of AScenes...)
    oobj scenes;

    int current_scene;
    int current_scene_start;
    bool scene_exit_active;

    // both may be NULL
    oobj app_title_scene;
    oobj app_icon_scene;
    
} AViewStage;


/**
 * Initializes the object.
 * @param obj AViewStage object
 * @param parent to inherit from
 * @return obj casted as AViewStage
 */
AViewStage *AViewStage_init(oobj obj, oobj parent);

/**
 * Creates a new AViewStage object.
 * @param parent to inherit from
 * @return The new object
 */
o_inline AViewStage *AViewStage_new(oobj parent) {
    OObj_DECL_IMPL_NEW(AViewStage, parent);
}



//
// object functions:
//

/**
 * @param obj AViewStage object
 * @return Current amount of scenes in the scene stack
 */
int AViewStage_scenes_num(oobj obj);

/**
 * Returns the AScene object for the given stack position / index
 * @param obj AViewStage object
 * @param index stack position / index (safe to pass outside bounds)
 * @return AScene object or NULL on failure
 */
struct oobj_opt AViewStage_scene_at(oobj obj, int index);

/**
 * Returns the stack position / index in from the given object
 * @param obj AViewStage object
 * @param scene AScene object (safe to pass stupid stuff...)
 * @return the index in the stack or -1 on failure
 */
int AViewStage_scene_index_for(oobj obj, oobj scene);


/**
 * @param obj AViewStage object
 * @return The current active scene index or -1 if none is active
 */
int AViewStage_scene_index_try(oobj obj);


/**
 * @param obj AViewStage object
 * @return The current active scene index.
 * @note asserts that a scene is currently active
 */
o_inline int AViewStage_scene_index(oobj obj)
{
    int current_scene_index = AViewStage_scene_index_try(obj);
    assert(current_scene_index>=0 && "no active AScene");
    return current_scene_index;
}

/**
 * @param obj AViewStage object
 * @return The current active AScene.
 * @note asserts that a scene is currently active
 */
o_inline oobj AViewStage_scene(oobj obj)
{
    return AViewStage_scene_at(obj, AViewStage_scene_index(obj)).o;
}


/**
 * Switches the position of two AScenes in the scene stack.
 * Only call this function if you know what you are doing.
 * Mia normally expects a working resource hierarchy and scenes may assume that they have specific parents.
 *      Mixing that could cause errors.
 * A use case of this function is u_splash:
 *      The splash scene is created first and rendered.
 *      In the following frame the actual scene is created and so its position is up the stack.
 *      The splash scene changes their stack positions and runs the actual scene to create it (may block during loadup)
 *      If the splash scene finishes, it deletes itself
 *
 * @param obj AViewStage object
 * @param scene_index_a, scene_index_b the scene indices to be changed in the scene stack
 * @note logs an error on failure
 *       asserts main thread
 */
void AViewStage_scene_switch_stack_position(oobj obj, int scene_index_a, int scene_index_b);

/**
 * Deletes the given scene index and all down the stack
 * @param obj AViewStage object
 * @param scene_index the index of the scenes (and all up the stack) to be deleted
 * @note logs an error on failure
 *       logs a warning, if the current active scene is >= scene_index.
 *       asserts main thread
 */
void AViewStage_scene_exit_instant_for(oobj obj, int scene_index);


/**
 * Deletes the given scene index and all down the stack.
 * Defers it to the next frame and calls AViewStage_scene_exit_instant then.
 * @param obj AViewStage object
 * @param scene_index the index of the scenes (and all up the stack) to be deleted
 *                    <0 to exit the current scene
 * @note logs an error on failure
 *       asserts main thread
 */
void AViewStage_scene_exit_for(oobj obj, int scene_index);


/**
 * Deletes the current scene index (AViewStage_scene_index) and all down the stack.
 * Defers it to the next frame and calls AViewStage_scene_exit_instant then
 * @param obj AViewStage object
 * @note logs an error on failure
 *       asserts main thread
 * @sa a_scene_exit(), AScene_exit()
 */
o_inline void AViewStage_scene_exit(oobj obj)
{
    AViewStage_scene_exit_for(obj, AViewStage_scene_index(obj));
}

/**
 * Deletes the given scene index while keeping all down the stack.
 * USE WITH CAUTION!
 * All scenes down should not have been some sort of parent to this scene nor should they depend on it.
 * Typical examples are intro scenes or other tmp inbetween scenes.
 * @param obj AViewStage object
 * @param scene_index the index of the scene to be deleted
 * @note logs an error on failure
 *       asserts main thread
 * @sa a_scene_exit_pop(), AScene_exit_pop()
 */
void AViewStage_scene_exit_pop_instant_for(oobj obj, int scene_index);

/**
 * Deletes the given scene index while keeping all down the stack.
 * USE WITH CAUTION!
 * All scenes down should not have been some sort of parent to this scene nor should they depend on it.
 * Typical examples are intro scenes or other tmp inbetween scenes.
 * Defers it to the next frame and calls AViewStage_scene_exit_pop_instant_for then.
 * @param obj AViewStage object
 * @param scene_index the index of the scene to be deleted
 * @note logs an error on failure
 *       asserts main thread
 * @sa a_scene_exit_pop(), AScene_exit_pop()
 */
void AViewStage_scene_exit_pop_for(oobj obj, int scene_index);

/**
 * Deletes the current scene index (AViewStage_scene_index) while keeping all down the stack.
 * USE WITH CAUTION!
 * All scenes down should not have been some sort of parent to this scene nor should they depend on it.
 * Typical examples are intro scenes or other tmp inbetween scenes.
 * Defers it to the next frame and calls AViewStage_scene_exit_pop_instant_for then.
 * @param obj AViewStage object
 * @note logs an error on failure
 *       asserts main thread
 * @sa a_scene_exit_pop(), AScene_exit_pop()
 */
o_inline void AViewStage_scene_exit_pop(oobj obj)
{
    AViewStage_scene_exit_pop_for(obj, AViewStage_scene_index(obj));
}



#endif //A_AVIEWSTAGE_H
