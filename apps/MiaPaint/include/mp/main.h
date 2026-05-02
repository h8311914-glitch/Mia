#ifndef MP_MAIN_H
#define MP_MAIN_H

/**
 * @file main.h
 *
 * Mia Paint main module to start the app
 */

#include "o/OObj.h"
#include "m/types/int.h"
#include "m/types/flt.h"

struct mp_options {
    ivec2 canvas_size;
    const vec4 *palette;
    int palette_num;

    oobj opt_canvas_init_tex;
    char *opt_save_path;

    bool show_exit_btn;

    // AScene from mp_main is passed
    OObj__event_fn opt_on_update;
};

/**
 * @return Default options to start if NULL is passed in mp_main
 */
struct mp_options mp_options_default(void);

/**
 * Starts the Mia Paint app
 * @param root object to allocate on
 * @param opt_options optional options for startup, pass NULL for mp_options_default
 * @return AScene object on which all of mia paint is built upon
 * @note o_user stuff on the AScene is not used internally and free to use.
 */
oobj mp_main(oobj root, struct mp_options *opt_options);

/**
 * Starts the Mia Paint app
 * @param root object to allocate on
 * @param opt_options optional options for startup, pass NULL for mp_options_default
 * @return AScene object of the used u_splash, which creates the main scene
 * @note o_user stuff on the AScene is not used internally and free to use.
 */
oobj mp_main_splashed(oobj root, struct mp_options *opt_options);


#endif //MP_MAIN_H
