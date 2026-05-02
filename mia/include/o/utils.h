#ifndef O_UTILS_H
#define O_UTILS_H

/**
 * @file utils.h
 *
 * Some os utils.
 * Open an url in a browser.
 * Get and set clipboard strings (copy and paste).
 */

#include "common.h"

/**
 * Opens the given url in an external system browser.
 * Use `file:/// full/ path/ to/ file` for local files, if supported.
 * @return true if it worked (or at least did maybe launch smth)
 */
bool o_utils_open_url(const char *url);

/**
 * @return true on platforms, where this module may work
 * @note emscripten does not work, for example
 */
bool o_utils_clipboard_system_available(void);

/**
 * @return true if a string is available on the clipboard
 * @sa o_clipboard_get
 */
bool o_utils_clipboard_available(void);

/**
 * @param parent OObj to allocate on
 * @return clipboard string null terminated, or NULL if not available (paste)
 *         needs to be o_free'd
 */
char *o_utils_clipboard_get(oobj parent);

/**
 * @param set new null terminated string to set into the clipboard (copy)
 */
void o_utils_clipboard_set(const char *set);

#endif //O_UTILS_H
