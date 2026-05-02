#ifndef MP_CANVAS_H
#define MP_CANVAS_H

/**
 * @file canvas.h
 *
 * Implements an AView to be used to show the image and draw on
 */

#include "o/OObj.h"

#include "surface.h"
#include "selection.h"
#include "history.h"

enum mp_canvas_tool {
    mp_canvas_TOOL_PICK,
    mp_canvas_TOOL_FREE,
    mp_canvas_TOOL_LINE,
    mp_canvas_TOOL_BOX,
    mp_canvas_TOOL_CIRCLE,
    mp_canvas_TOOL_FILL4,
    mp_canvas_TOOL_FILL8,
    mp_canvas_TOOL_REPLACE,
    mp_canvas_TOOL_ENUM_MAX
};

/**
 * @param parent to inherit from
 * @param surface surface to draw on
 * @return AView that renders and handles the canvas
 */
oobj mp_canvas_new(oobj parent, struct mp_surface *surface, struct mp_selection *selection, struct mp_history *history);

/**
 * Sets the camera to home / show the full texture with a border around it
 * @param view AView created from mp_canvas_new
 * @param selection if true, fit to selection (if available), else full canvas
 */
void mp_canvas_cam_home(oobj view, bool selection);

/**
 * @param view AView created from mp_canvas_new
 * @param cols, rows pattern repeats after these, use (one of em) <= 0 to turn off bg pattern
 */
void mp_canvas_pattern_set(oobj view, int cols, int rows);

/**
 * @param view AView created from mp_canvas_new
 * @return current set draw tool
 */
enum mp_canvas_tool mp_canvas_tool(oobj view);

/**
 * @param view AView created from mp_canvas_new
 * @return current set draw tool
 */
enum mp_canvas_tool mp_canvas_tool_set(oobj view, enum mp_canvas_tool tool);


#endif //MP_CANVAS_H
