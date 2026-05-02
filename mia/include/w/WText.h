#ifndef W_WTEXT_H
#define W_WTEXT_H

/**
 * @file WText.h
 *
 * Object (derives WObj)
 *
 * Widget that renders text with WTheme
 */

#include "WObj.h"
#include "m/types/int.h"
#include "r/textlayout.h"

/** object id */
#define WText_ID WObj_ID "Text."


/**
 * An WText can be setup to always render upper case characters.
 * Matches enum r_textlayout_casing
 */
enum WText_casing {
    WText_casing_UNCHANGED,
    WText_casing_LOWER,
    WText_casing_UPPER,
    WText_casing_ENUM_MAX
};

/**
 * If min_size or fixed_size is set and wrap is != OFF, it breaks the lines at ANY char or at a word break (' ').
 * Matches enum r_textlayout_wrap
 */
enum WText_wrap {
    WText_wrap_OFF,
    WText_wrap_ANY,
    WText_wrap_WORD,
    WText_wrap_ENUM_MAX
};

/**
 * If WText_wrap_WORD is set, this defines how to use the remaining line room.
 * Matches enum r_textlayout_align
 */
enum WText_align {
    WText_align_LEFT,
    WText_align_RIGHT,
    WText_align_CENTER,
    WText_align_BLOCK,
    WText_align_ENUM_MAX
};

typedef struct {
    WObj super;

    enum WText_casing casing;
    enum WText_wrap wrap;
    enum WText_align align;

    vec2 char_size;
    vec2 char_offset;
    vec2 char_scale;

    vec2 wrap_size;

    vec4 color;
    char *text;
    int num;

    vec2 text_size;
    ivec2 cols_rows;

    // for internal use
    vec4 color_enabled_applied;
} WText;



/**
 * Initializes the object
 * @param obj WText object
 * @param parent to inherit from
 * @param text to render
 * @return obj casted as WText
 */
WText *WText_init(oobj obj, oobj parent, const char *text);


/**
 * Creates a new WText object
 * @param parent to inherit from
 * @param text to render
 * @return The new object
 */
o_inline WText *WText_new(oobj parent, const char *text)
{
    OObj_DECL_IMPL_NEW(WText, parent, text);
}

//
// virtual implementations:
//


/**
 * Virtual implementation that renders the text into the given WTheme object.
 * Calls the default WObj__v_update.
 */
vec2 WText__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);

/**
 * Virtual implementation that sets the text color
 */
void WText__v_style_apply(oobj obj);
  

//
// object functions:
//


/**
 * @param obj WText object
 * @return text case used
 */
OObj_DECL_GETSET(WText, enum WText_casing, casing)

/**
 * @param obj WText object
 * @return text (line) wrapping used (uses min_size or fixed_size if != OFF)
 */
OObj_DECL_GETSET(WText, enum WText_wrap, wrap)

/**
 * @param obj WText object
 * @return text aligned for WText_wrap_WORD
 */
OObj_DECL_GETSET(WText, enum WText_align, align)

/**
 * Will update WText_char_size and WText_char_offset (if char_size.x<0)
 * @param obj WText object
 * @param theme WTheme object
 */
void WText_char_size_update(oobj obj, oobj theme);


/**
 * @param obj WText object
 * @return character size
 * @note inits with first update call using WTheme (char '0') (update if char_size.x<0)
 *       or call WText_char_size_update first
 */
OObj_DECL_GETSET(WText, vec2, char_size)

/**
 * @param obj WText object
 * @return character offset
 * @note inits with first update call using WTheme (char '0') (char_size + 1) (update if char_size.x<0)
 *       or call WText_char_size_update first
 */
OObj_DECL_GETSET(WText, vec2, char_offset)

/**
 * @param obj WText object
 * @return character scaling, defaults to vec2_(1)
 * @note if using pixel art, use integers to be pixel perfect
 */
OObj_DECL_GETSET(WText, vec2, char_scale)

/**
 * @param obj WText object
 * @return max size for the textlayout it wrap mode is != OFF, defaults to -1
 *         Only uses it if >= 0, else uses child_size / min_size
 */
OObj_DECL_GETSET(WText, vec2, wrap_size)

/**
 * @param obj WText object
 * @return color for each char on an update call
 */
OObj_DECL_GETSET(WText, vec4, color)

/**
 * @param obj WText object
 * @return used text
 */
OObj_DECL_GET(WText, const char*, text)

/**
 * @param obj WText object
 * @param text to set (will be cloned)
 * @return new allocated cloned text
 */
const char *WText_text_set(oobj obj, const char *text);

/**
 * @param obj WText object
 * @return used text length == o_strlen(text)
 */
OObj_DECL_GET(WText, int, num)

/**
 * @param obj WText object
 * @return used text_size (needs an update)
 */
OObj_DECL_GET(WText, vec2, text_size)


/**
 * @param obj WText object
 * @return text cols and rows
 */
OObj_DECL_GET(WText, ivec2, cols_rows)


#endif //W_WTEXT_H
