#ifndef MS_MAIN_H
#define MS_MAIN_H

/**
 * @file main.h
 *
 * Mia Studio main module to start the app
 */

#include "o/common.h"


/**
 * Starts the Mia Studio app
 * @param root object to allocate on
 * @note o_user stuff on the AScene is not used internally and free to use.
 */
oobj ms_main(oobj root);

/**
 * Starts the Mia Studio app
 * @param root object to allocate on
 * @return AScene object of the used u_splash, which creates the main scene
 * @note o_user stuff on the AScene is not used internally and free to use.
 */
oobj ms_main_splashed(oobj root);

#endif //MS_MAIN_H
