#ifndef U_SPLASH_H
#define U_SPLASH_H

/**
 * @file splash.h
 *
 * Creates an AScene as splashscreen.
 * After a few frames, the actual scene is created and their scene stack positions are switched.
 * So if the actual scene blocks during loadup, the (static) splash screen keeps showing
 */

#include "o/common.h"
#include "m/types/flt.h"
#include "a/AScene.h"

/**
 * Creates a splash scene, which involves the creation of the actual scene internally
 * @param parent to allocate on
 * @param splash_view AView for the splashscreen
 * @param move_splash_view if true, moved into the created AScene
 * @param actual_view AView to create an AScene from
 * @param move_actual_view_in_its_scene if true, actual_view is moved into the created actual scene
 * @param time_until_actual_scene_creation if this time has ended, the actual scene is created (0 would be next frame)
 * @return AScene of the splash screen
 */
oobj u_splash_new(oobj parent,
                  oobj splash_view, bool move_splash_view, enum AScene_mode splash_scene_mode,
                  oobj actual_view, bool move_actual_view_in_its_scene, enum AScene_mode actual_scene_mode,
                  float time_until_actual_scene_creation);
                  
/**
 * @param splash_view AScene object created with u_splash_new*
 * @return the AScene object to be created "actual", or NULL if not created yet
 */
struct oobj_opt u_splash_actual_scene(oobj splash_view);

/**
 * Sets the internal time over so it creates the scene on next update call
 * @param splash_view AScene object created with u_splash_new*
 */
void u_splash_invoke_creation(oobj splash_view);


/**
 * Creates a splash scene, which involves the creation of the actual scene internally.
 * Mia version which just displays a text for the author, like "HORSIMANN".
 * May be changed in the future...
 * @param parent to allocate on
 * @param actual_view AView to create an AScene from
 * @param move_actual_view_in_its_scene if true, actual_view is moved into the created actual scene
 * @param bg_color for the full background (RTex_clear_full)
 * @param shell_color, cmd_color, text_color for the text
 * @param author text to be displayed
 * @param app text to be displayed
 * @param min_time minimal time this splash is running (more if actual scene blocks while loading)
 * @param skipable if true a double click skips the splash
 * @return AScene of the splash screen
 */
oobj u_splash_new_mia_ex(oobj parent,
                         oobj actual_view, bool move_actual_view_in_its_scene, enum AScene_mode actual_scene_mode,
                         vec4 bg_color, vec4 shell_color, vec4 cmd_color, vec4 text_color,
                         const char *author, const char *app,
                         float min_time,
                         bool skipable);

/**
 * Creates a splash scene, which involves the creation of the actual scene internally.
 * Mia version which just displays a text for the author, like "HORSIMANN".
 * May be changed in the future...
 * @param parent to allocate on
 * @param actual_view AView to create an AScene from
 * @param move_actual_view_in_its_scene if true, actual_view is moved into the created actual scene
 * @param author text to be displayed
 * @param app text to be displayed
 * @param min_time minimal time this splash is running (more if actual scene blocks while loading)
 * @param skipable if true a double click skips the splash
 * @return AScene of the splash screen
 */
oobj u_splash_new_mia(oobj parent,
                      oobj actual_view, bool move_actual_view_in_its_scene, enum AScene_mode actual_scene_mode,
                      const char *author, const char *app,
                      float min_time,
                      bool skipable);


#endif //U_SPLASH_H
