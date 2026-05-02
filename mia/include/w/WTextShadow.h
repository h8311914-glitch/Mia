#ifndef W_WTEXTSHADOW_H
#define W_WTEXTSHADOW_H

/**
 * @file WTextShadow.h
 *
 * Object (derives WText)
 *
 * Text with background shadow.
 * Uses 2x boxes as compared to WText.
 * Each character has rect 0 as background shadow and rect 1 is the foreground
 */

#include "WText.h"

/** object id */
#define WTextShadow_ID WText_ID "Shadow."


typedef struct {
    WText super;

    vec4 shadow_color;

    // for internal use
    vec4 shadow_color_enabled_applied;
} WTextShadow;


/**
 * Initializes the object
 * @param obj WTextShadow object
 * @param parent to inherit from
 * @param text to render
 * @return obj casted as WTextShadow
 */
WTextShadow *WTextShadow_init(oobj obj, oobj parent, const char *text);


/**
 * Creates a new WTextShadow object
 * @param parent to inherit from
 * @param text to render
 * @return The new object
 */
o_inline WTextShadow *WTextShadow_new(oobj parent, const char *text)
{
    OObj_DECL_IMPL_NEW(WTextShadow, parent, text);
}

//
// virtual implementations:
//


/**
 * Virtual implementation that renders the text into the given WTheme object.
 * Calls the default WObj__v_update.
 * @note does >not< call WText_update, but that new update vfunc is mostly the same (with some changes...)
 */
vec2 WTextShadow__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);

/**
 * Virtual implementation that sets the text color
 */
void WTextShadow__v_style_apply(oobj obj);
  

//
// object functions:
//


/**
 * @param obj WTextShadow object
 * @return color for each char on an update call
 */
OObj_DECL_GETSET(WTextShadow, vec4, shadow_color)

#endif //W_WTEXTSHADOW_H
