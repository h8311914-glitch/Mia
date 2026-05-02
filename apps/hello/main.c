/**
 * Welcome to Mia :)
 * This will be the starting point of the app
 *      (at the end of this file is the main function)
 * Feel free to tinker around.
 * Can be used as your own app template.
 * To use git, fork the template [MiaApp](https://github.com/renehorstmann/MiaApp) instead.
 *     And clone it into the apps/ directory of Mia.
 *     The MiaApp template is mostly the same app as this hello mini app.
 *
 * Or start the Examples App; Tea App or Mia Paint.
 *
 * Also have a look at the examples which act as a tutorial for coding in Mia ;)
 */

/** Only for the file containing the real main() function, before importing anything else */
#define MIA_MAIN

/**
 * Includes most of Mia
 * This header is located under "./include/mia.h";
 * The include directory "./include" may be used for your user code ( as well as this "./src" dir)
 */
#include "mia.h"


/**
 * Includes the Examples App and Tea App (which is part of the examples)
 * Located in ./apps/
 */
#include "ex/ex.h"

/** Includes Mia Paint (also located in ./apps/) */
#include <stdlib.h>

#include "mp/mp.h"


/**
 * Context Data for the hello world AView
 */
struct context {
    vec4 bg_color;

    oobj theme;
    oobj gui;
    oobj text;
};



/**
 * Called once at start of this AView
 * @param view AView object
 */
static void setup(oobj view)
{
    /** Get user context */
    struct context *C = o_user_new0(view, *C, 1);

    /** Logs an info string, works with formatting like printf */
    o_log("hello world");

    /** Setting some values in the context */
    C->bg_color = R_GRAY_X(0.33);

    /**
     * Creating a small w based gui
     */
    C->theme = WTheme_new_tiny(view);
    C->gui = WBox_new_v(view);
    C->text = WText_new(C->gui, "HELLO WORLD!");
    
}

/**
 * Called every frame before rendering to handle simulation, events, etc.
 * @param view AView object
 * @param tex RTex to render to (or NULL for the backbuffer...)
 *            Even if available, please render in the render function
 * @param dt delta time in seconds (time between this and the last frame)
 */
static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /** Update the gui */
    WTheme_update(C->theme, C->gui, vec2_(0), vec2_(0));

}


/**
 * Called every frame to render
 */
static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /** Clear the screen with the background color */
    RTex_clear_full(tex, C->bg_color);

    /** Renders the gui */
    WTheme_render(C->theme, tex);
}

/**
 * Starting point of the mia app with all systems initialized
 * @param root OObj to allocate on, acts as root parent for all objects
 */
static void app_main(oobj root)
{
    /**
     * Uncomment to install a virtual stage which may be resized or to take screenshots
     */
    // x_viewvirtual_scene_install_stage(root, ivec2_(180, 180));

    x_install();
    

    /**
     * Apps
     * Remove line comments to start
     */

    /** Examples */
    ex_main_splashed(root);  return;
    
    /** Tea Timer */
    // ex_tea_main_splashed(root);  return;

    /** Mia Paint */
    // mp_main_splashed(root, NULL);  return;

    /**
     * AScene with the AView of this main.c hello world file
     */
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    AScene_opaque_set(scene, false);
}


/**
 * Actual c main entry point.
 * This file needs "#define MIA_MAIN" before the includes...
 */
int main(int argc, char **argv)
{
    struct a_app_run_options options = a_app_run_options_default();
    options.mic_enable = true;
    options.log_level = O_LOG_INFO;

    a_app_run(app_main, &options);
    return 0;
}