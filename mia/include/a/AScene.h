#ifndef A_ASCENE_H
#define A_ASCENE_H

/**
 * @file AScene.h
 *
 * Object
 *
 * Manages one scene on the app scene stack.
 * Creating an AScene automatically registers it in the a_app module.
 *
 */


#include "o/OObj.h"
#include "m/types/int.h"
#include "AView.h"

// forward
struct o_img;

/** object id */
#define AScene_ID OObj_ID "AScene."

enum AScene_mode {
    AScene_SAFE,
    AScene_UNSAFE,
    AScene_CUSTOM,
    AScene_mode_ENUM_MAX
};

typedef struct {
    OObj super;

    AView *view;

    // AViewStage in which this scene is registered in
    oobj stage;

    bool opaque;

    // bounds left top width height in backbuffer (current tex) viewport coordinates
    ivec4 viewport;

    enum AScene_mode mode;

    OObj__event_fn escape_event;

    // Only != 0 if AScene_mode_FULL_UNSAFE
    // left top right back [%] of the viewport size to the safe zone
    vec4 safe_padding_rel;

    // true for a single frame if its resumed (calling update and render again, after an opaque scene was closed)
    bool resuming;

    // app title or NULL
    char *app_title;
    bool app_title_set;

    // sdl surface or NULL
    void *app_icon;
    bool app_icon_set;

} AScene;


/**
 * Initializes the object.
 * @param obj AScene object
 * @param parent to inherit from
 * @param stage AViewStage object to register in
 * @param view AView object for this scene, see AScene_view_set
 *             (NULL ok, but needs to be set with AScene_view_set immediately)
 * @param move_view if true, view is o_move_res'd into this object
 * @param mode as an example AScene_mode_SAFE for a safe to use view,
 *             or AScene_mode_UNSAFE which may extend beyond camera notches, etc.
 *                 Use AScene_safe_padding to shift interactive elements into the safe zone
 *             or AScene_mode_CUSTOM for a custom AScene_viewport_set()
 * @return obj casted as AScene
 * @note inits full_* options to true
 */
AScene *AScene_init_ex(oobj obj, oobj parent, oobj stage, oobj view, bool move_view, enum AScene_mode mode);

/**
 * Creates a new AScene object
 * @param parent to inherit from
 * @param stage AViewStage object to register in
 * @param view AView object for this scene, see AScene_view_set
 *             (NULL ok, but needs to be set with AScene_view_set immediately)
* @param move_view if true, view is o_move_res'd into this object
 * @param mode as an example AScene_mode_SAFE for a safe to use view,
 *             or AScene_mode_UNSAFE which may extend beyond camera notches, etc.
 *                 Use AScene_safe_padding to shift interactive elements into the safe zone
 *             or AScene_mode_CUSTOM for a custom AScene_viewport_set()
 * @return The new object
 */
o_inline AScene *AScene_new_ex(oobj parent, oobj stage, oobj view, bool move_view, enum AScene_mode mode)
{
    OObj_DECL_IMPL_NEW_SPECIAL(AScene, ex, parent, stage, view, move_view, mode);
}


/**
 * Creates a new AScene object.
 * Automatically registers in the current AViewStage of the app (a_app_stage())
 * @param obj AScene object
 * @param parent to inherit from
 * @param view AView object for this scene, see AScene_view_set
 *             (NULL ok, but needs to be set with AScene_view_set immediately)
 * @param move_view if true, view is o_move_res'd into this object
 * @param mode as an example AScene_mode_SAFE for a safe to use view,
 *             In the safe mode, the padding or unsafe border is clamped to be max. 10% of the display
 *                 width / height for each border.
 *                 So worst case is a safe zone with 80% width and 80% height in contrast to the unsafe zone.
 *             or AScene_mode_UNSAFE which may extend beyond camera notches, etc.
 *                 Use AScene_safe_padding to shift interactive elements into the safe zone
 *             or AScene_mode_CUSTOM for a custom AScene_viewport_set()
 * @return The new object
 */
AScene *AScene_new(oobj parent, oobj view, bool move_view, enum AScene_mode mode);


//
// object functions:
//

/**
 * @param obj AScene object
 * @return true (default) if this scene is opaque (false: transparent)
 */
OObj_DECL_GETSET(AScene, bool, opaque)

/**
 * @param obj AScene object
 * @return AView object of this scene
 */
OObj_DECL_GET(AScene, AView *, view)

/**
 * @param obj AScene object
 * @return AViewStage in which this scene is registered in
 */
OObj_DECL_GET(AScene, oobj, stage)

/**
 * @param obj AScene object
 * @param view AView object for this scene.
 * @param del_old if true, the olf view is o_del'ed
 */
AView *AScene_view_set(oobj obj, oobj view, bool del_old);

/**
 * @param obj AScene object
 * @return bounds left bottom width height
 */
OObj_DECL_GETSET(AScene, ivec4, viewport)

/**
 * @param obj AScene object
 * @return viewport mode, defaults to AScene_mode_FULL_SAFE which sets the viewport to fullscreen in safe zone
 * @note Modern devices offer a safe zone, outside from it are for example the top camera notch / holes, etc.
 *       In the safe zone, the hud is safe to interact with and should be visible.
 *       Outside the safe zone, the gameplay may still be rendered, such as the background or level, etc.
 * In the safe mode, the padding or unsafe border is clamped to be max. 10% of the display width / height for each border.
 *     So worst case is a safe zone with 80% width and 80% height in contrast to the unsafe zone.
 * @warning AScene_mode_FULL_UNSAFE only works if the current AViewStage's tex is in fact full screen
 */
OObj_DECL_GETSET(AScene, enum AScene_mode, mode)

/**
 * @param obj AScene object
 * @return called if ESCAPE or BACK is pressed, may be NULL (default)
 *         (if it's the last scene in the stack)
 */
OObj_DECL_SET(AScene, OObj__event_fn, escape_event)


/**
 * @param obj AScene object
 * @return if AScene_mode() == AScene_mode_FULL_UNSAFE: relative [%] padding to the safe zone as ltrb
 *                                                      clamped to max. 10%
 *         else: 0
 * @note Modern devices offer a safe zone, outside from it are for example the top camera notch / holes, etc.
 *       In the safe zone, the hud is safe to interact with and should be visible.
 *       Outside the safe zone, the gameplay may still be rendered, such as the background or level, etc.
 */
OObj_DECL_GET(AScene, vec4, safe_padding_rel)


/**
 * @param obj AScene object
 * @return true for a single frame if its resumed
 *         (calling update and render again, after an opaque scene was closed)
 */
OObj_DECL_GET(AScene, bool, resuming)


/**
 * @param obj AScene object
 * @return if AScene_mode() == AScene_mode_FULL_UNSAFE:  padding to the safe zone as ltrb in the AView's camera coords
 *                                                      clamped to max. 10% of the display width / height
 *         else: 0
 * @note Modern devices offer a safe zone, outside from it are for example the top camera notch / holes, etc.
 *       In the safe zone, the hud is safe to interact with and should be visible.
 *       Outside the safe zone, the gameplay may still be rendered, such as the background or level, etc.
 */
vec4 AScene_safe_padding(oobj obj);


/**
 * Returns the stack position index from a_app.
 *      Calls simply a_app_scene_object_index(obj)
 * @param obj AScene object
 * @return the stack position from a_app [0 : a_app_scenes_num())
 */
int AScene_index(oobj obj);


/**
 * Exits this scene (and all up the stack) deferred in the next frame
 * @param obj AScene object
 * @sa a_scene_exit(), AViewStage.h
 */
void AScene_exit(oobj obj);

/**
 * Exits this scene while keeping all down the stack.
 * USE WITH CAUTION!
 * All scenes down should not have been some sort of parent to this scene nor should they depend on it.
 * Typical examples are intro scenes or other tmp inbetween scenes.
 * @param obj AScene object
 * @sa a_scene_exit_pop(), AViewStage.h
 */
void AScene_exit_pop(oobj obj);


/**
 * Registers an escape event that will exit this scene
 *         (if it's the last scene in the stack)
 * @param obj AScene object
 */
void AScene_escape_event_set_exit(oobj obj);


/**
 * @param obj AScene object
 * @param opt_title for this AScene, the app title will be the last set title of scenes in the stack, NULL to remove
 * @note works on MIA_PLATFORM_DESKTOP + MIA_PLATFORM_EMSCRIPTEN
 */
void AScene_title_set(oobj obj, const char *opt_title);

/**
 * @param obj AScene object
 * @param opt_icon for this AScene, the app icon will be the last set icon of scenes in the stack, NULL to remove
 * @note works on MIA_PLATFORM_DESKTOP
 */
void AScene_icon_set(oobj obj, struct o_img *opt_icon);

/**
 * @param obj AScene object
 * @param file to load the icon from, calls AScene_icon_set, or NULL to remove the scene icon
 * @note works on MIA_PLATFORM_DESKTOP
 */
void AScene_icon_set_file(oobj obj, const char *opt_file);



#endif //A_ASCENE_H
