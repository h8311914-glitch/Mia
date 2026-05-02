#ifndef R_RTEX_TEXT_H
#define R_RTEX_TEXT_H

/**
 * @file RTex_text.h
 *
 * Object extension
 *
 * RTex_ namespace
 *
 * Rendering text onto an RTex
 *
 * @sa RTex_ex.h to import all RTex object extensions
 */


#include "RTex.h"


enum RTex_font {
    RTex_font35,
    RTex_font35_shadow,
    RTex_font55,
    RTex_font55_shadow,
    RTex_font58,
    RTex_font58_shadow,
    RTex_font_ENUM_MAX
};

//
// text
//

/**
 * Renders a text with a given r_text interface and font RTex
 * @param obj RTex object
 * @param layout r_textlayout interface
 * @param font_tex RTex of the font
 * @param text to be rendered
 * @param pose pose for the left top text begin, width and height are scaling with 1.0 as default (2.0 is size doubled)
 * @param text_color rgba color for the rendered text
 * @return needed text_size
 * @note If you need more control, have a look at RObjText
 */
vec2 RTex_text_ex(oobj obj, struct r_textlayout *layout, oobj font_tex, const char *text, mat4 pose, vec4 text_color);

/**
 * Renders a text with a given font on the given texture
 * @param obj RTex object
 * @param text to be rendered
 * @param pose pose for the left top text begin, width and height are scaling with 1.0 as default (2.0 is size doubled)
 * @param font to use (textlayout casing is always UNCHANGED)
 * @param text_color rgba color for the rendered text
 * @return needed text_size
 * @note If you need more control, have a look at RObjText
 */
vec2 RTex_text_pose(oobj obj, const char *text, mat4 pose, enum RTex_font font, vec4 text_color);


/**
 * Renders a text with a given font on the given texture
 * @param obj RTex object
 * @param text to be rendered
 * @param rect rect for the left top text begin, width and height are scaling with 1.0 as default (2.0 is size doubled)
 * @param font to use (textlayout casing is always UNCHANGED)
 * @param text_color rgba color for the rendered text
 * @return needed text_size
 * @note If you need more control, have a look at RObjText
 */
vec2 RTex_text_rect(oobj obj, const char *text, vec4 rect, enum RTex_font font, vec4 text_color);

/**
 * Renders a text with a given font on the given texture
 * @param obj RTex object
 * @param text to be rendered
 * @param x, y for the left top text begin, width=height=1
 * @param font to use (textlayout casing is always UNCHANGED)
 * @param text_color rgba color for the rendered text
 * @return needed text_size
 * @note If you need more control, have a look at RObjText
 */
o_inline vec2 RTex_text(oobj obj, const char *text, float x, float y, enum RTex_font font, vec4 text_color)
{
    return RTex_text_rect(obj, text, vec4_(x, y, 1, 1), font, text_color);
}


#endif //R_RTEX_TEXT_H
