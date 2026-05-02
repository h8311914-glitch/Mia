#ifndef X_TERMINAL_H
#define X_TERMINAL_H

/**
 * @file terminal.h
 *
 * Terminal that shows logs and terminal output + keyboard.
 * - XViewLogs
 * - XViewShell
 * - XViewKeys
 *
 * If installed, swipe up to open a transparent log panel on half the screen.
 * Next swipe shows the logs on top half and on bottom half are
 * terminal output shell and a keys for terminal input.
 * The shell allows for some built in commands + custom ones.
 */

#include "o/common.h"


#define X_TERMINAL_COLS 48
#define X_TERMINAL_ROWS 16


/**
 * Some options for the terminal.
 */
struct x_terminal_options {
    bool enable;

    float start_border;
    float start_distance;
};

/**
 * Default options (used if NULL is passed on install)
 */
struct x_terminal_options x_terminal_options_default(void);


/**
 * Install the terminal on a new a_app_view's layer.
 * So it works on top of the default scene stage.
 * @param opt_options: pass NULL for defaults.
 */
void x_terminal_install(const struct x_terminal_options *opt_options);

/**
 * @return true if already installed
 */
bool x_terminal_installed(void);

/**
 * @return a reference to some terminal options
 */
struct x_terminal_options *x_terminal_options_ref(void);

/**
 * Opens the terminal by code.
 * @param full if true, shows logs and shell, falso for logs only
 * @note Noop if not terminal is not installed.
 */
void x_terminal_open(bool full);

/**
 * Closes / Hides the terminal instantly.
 * Useful if a custom shell api starts a new scene.
 * @note Noop if not terminal is not installed.
 */
void x_terminal_close(void);


/**
 * @return XViewShell of the terminal (if installed)
 */
struct oobj_opt x_terminal_shell(void);


/**
 * @returns current string value (or NULL) for the given key.
 * @note env variables can be set by the env api command.
 *       returns NULL if the terminal is not installed.
 */
const char *x_terminal_env(const char *key);

/**
 * @returns true if the current string value for the given key matches equals
 * @note env variables can be set by the env api command.
 *       returns false if terminal is not installed.
 *       Just calls "o_str_equals(x_terminal_env(key), equals)"
 */
bool x_terminal_env_equals(const char *key, const char *equals);

/**
 * @param value to set for the key or NULL to rm the key
 * @returns current string value (or NULL) for the given key.
 * @note env variables can be set by the env api command
 *       is a noop and returns NULL if terminal is not installed
 */
const char *x_terminal_env_set(const char *key, const char *value);


#endif //X_TERMINAL_H
