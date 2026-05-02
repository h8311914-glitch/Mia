#ifndef X_COMMON_H
#define X_COMMON_H

/**
 * @file install.h
 *
 * Common used x stuff
 *
 */

#include "o/common.h"
#include "x/cursor.h"
#include "x/terminal.h"
#include "x/toast.h"

/**
 * just a simple helper function to install all optional x modules (in the correct order :D)
 * Such as:
 * - terminal
 * - toast
 * - cursor
 */
o_inline void x_install(void)
{
    x_terminal_install(NULL);
    x_toast_install();
    x_cursor_install(NULL);
}

#endif //X_COMMON_H
