#ifndef R_TEXTLAYOUT_H
#define R_TEXTLAYOUT_H

/**
 * @file textlayout.h
 *
 * Setup r_box'es or r_quads for text rendering
 */

#include "o/common.h"
#include "m/types/byte.h"
#include "m/types/flt.h"

#define R_TEXT_FONT35_FILE "$r/font35.png"
#define R_TEXT_FONT35_SHADOW_FILE "$r/font35_shadow.png"

#define R_TEXT_FONT55_FILE "$r/font55.png"
#define R_TEXT_FONT55_SHADOW_FILE "$r/font55_shadow.png"

#define R_TEXT_FONT58_FILE "$r/font58.png"
#define R_TEXT_FONT58_SHADOW_FILE "$r/font58_shadow.png"


// forward
struct r_textlayout;

typedef vec2 (*r_textlayout__char_uv_fn)(struct r_textlayout *self, char c);

typedef void (*r_textlayout__generic_fn)(struct r_textlayout *self, osize idx, vec2 xy, vec2 uv);

/**
 * A textlayout can be setup to always render upper case characters
 */
enum r_textlayout_casing {
    r_textlayout_casing_UNCHANGED,
    r_textlayout_casing_LOWER,
    r_textlayout_casing_UPPER,
    r_textlayout_casing_ENUM_MAX
};

/**
 * If max_size is set and wrap is != OFF, it breaks the lines at ANY char or at a word break (' ')
 */
enum r_textlayout_wrap {
    r_textlayout_wrap_OFF,
    r_textlayout_wrap_ANY,
    r_textlayout_wrap_WORD,
    r_textlayout_wrap_ENUM_MAX
};

/**
 * If r_textlayout_wrap_WORD is set, this defines how to use the remaining line room
 */
enum r_textlayout_align {
    r_textlayout_align_LEFT,
    r_textlayout_align_RIGHT,
    r_textlayout_align_CENTER,
    r_textlayout_align_BLOCK,
    r_textlayout_align_ENUM_MAX
};


struct r_textlayout {
    vec2 size;
    vec2 offset;

    // defaults are UNCHANGED, OFF, LEFT
    enum r_textlayout_casing casing;
    enum r_textlayout_wrap wrap;
    enum r_textlayout_align align;
    
    // defaults to -1 (ignored)
    // if .x>0, see wrap and align
    // if .y>0, remaining characters are hidden
    vec2 max_size;

    // virtual function to get the char's uv
    r_textlayout__char_uv_fn v_char_uv;

    // user data to access in char_uv
    void *user;

    // OArray may be set by r_textlayout_set_boxed or r_textlayout_set_quads
    oobj internal_generic_array;
    vec4 internal_generic_color;

    // resulting size of the current set text
    vec2 text_size;
    int cols, rows;
};


/**
 * Creates a new r_textlayout struct (no need to free...)
 * @param size of a font char
 * @param offset to next char (y for next line)
 * @param char_uv virtual function to get uv center pos for a character in tex
 * @return The new created struct
 */
struct r_textlayout r_textlayout_new(vec2 size, vec2 offset, r_textlayout__char_uv_fn char_uv);


/**
 * Creates a new r_textlayout struct (no need to free...)
 * Uses the font file R_TEXT_FONT35_FILE with 3x5 size (or see r/tex.h: r_tex_font35() )
 * @return The new created struct
 * @note inits textlayout_mode as UPPER, even if lower case is available
 */
struct r_textlayout r_textlayout_new_font35(void);

/**
 * Creates a new r_textlayout struct (no need to free...)
 * Uses the font file R_TEXT_FONT35_SHADOW_FILE with 3x5 size (or see r/tex.h: r_tex_font35_shadow() )
 * To create a custom shadow tex, see r_textlayout_font35_shadow_tex()
 * The default shadow is bvec4_(0, 0, 0, 128) and is equal to a contouring shader
 * with 2x2 kernel aligned bottom right
 * @return The new created struct
 * @note inits textlayout_mode as UPPER, even if lower case is available
 */
struct r_textlayout r_textlayout_new_font35_shadow(void);

/**
 * Creates a new r_textlayout struct (no need to free...)
 * Uses the font file R_TEXT_FONT55_FILE with 5x5 size (or see r/tex.h: r_tex_font55() )
 * @return The new created struct
 * @note inits textlayout_mode as UNCHANGED
 */
struct r_textlayout r_textlayout_new_font55(void);

/**
 * Creates a new r_textlayout struct (no need to free...)
 * Uses the font file R_TEXT_FONT55_SHADOW_FILE with 5x5 size (or see r/tex.h: r_tex_font55_shadow() )
 * To create a custom shadow tex, see r_textlayout_font55_shadow_tex()
 * The default shadow is bvec4_(0, 0, 0, 128) and is equal to a contouring shader
 * with 2x2 kernel aligned bottom right
 * @return The new created struct
 * @note inits textlayout_mode as UNCHANGED
 */
struct r_textlayout r_textlayout_new_font55_shadow(void);

/**
 * Creates a new r_textlayout struct (no need to free...)
 * Uses the font file R_TEXT_FONT58_FILE with 5x8 size (or see r/tex.h: r_tex_font58() )
 * @return The new created struct
 * @note inits textlayout_mode as UNCHANGED
 */
struct r_textlayout r_textlayout_new_font58(void);

/**
 * Creates a new r_textlayout struct (no need to free...)
 * Uses the font file R_TEXT_FONT58_SHADOW_FILE with 5x8 size (or see r/tex.h: r_tex_font58_shadow() )
 * To create a custom shadow tex, see r_textlayout_font58_shadow_tex()
 * The default shadow is bvec4_(0, 0, 0, 128) and is equal to a contouring shader
 * with 2x2 kernel aligned bottom right
 * @return The new created struct
 * @note inits textlayout_mode as UNCHANGED
 */
struct r_textlayout r_textlayout_new_font58_shadow(void);

//
// struct functions
//

/**
 * Creates an RTex with a custom shadow color for the text
 * @param parent To allocate on
 * @param opt_shadow_color if NULL, bvec4_(0, 0, 0, 128) is used from the font sheet,
 *                         else replaced by the given color
 * @return RTex font35 shadow with custom color
 */
oobj r_textlayout_font35_shadow_tex(oobj parent, bvec4 *opt_shadow_color);

/**
 * Creates an RTex with a custom shadow color for the text
 * @param parent To allocate on
 * @param opt_shadow_color if NULL, bvec4_(0, 0, 0, 128) is used from the font sheet,
 *                         else replaced by the given color
 * @return RTex font35 shadow with custom color
 */
oobj r_textlayout_font55_shadow_tex(oobj parent, bvec4 *opt_shadow_color);

/**
 * Creates an RTex with a custom shadow color for the text
 * @param parent To allocate on
 * @param opt_shadow_color if NULL, bvec4_(0, 0, 0, 128) is used from the font sheet,
 *                         else replaced by the given color
 * @return RTex font58 shadow with custom color
 */
oobj r_textlayout_font58_shadow_tex(oobj parent, bvec4 *opt_shadow_color);

/**
 * Query the given textlayout into a generic callback for each char.
 * Used by r_textlayout_set_boxes|quads and r_textlayout_size
 */
vec2 r_textlayout_set_generic(struct r_textlayout *self, const char *text, r_textlayout__generic_fn generic_callback_fn);

/**
 * textlayout to struxt r_box OArray. Pushs each char.
 * @param boxes_array OArray of struct r_box.
 * @paeam textlayout_color for each char box
 */
vec2 r_textlayout_set_boxes(struct r_textlayout *self, oobj boxes_array, const char *text, vec4 textlayout_color);

/**
 * textlayout to struxt r_quad OArray. Pushs each char.
 * @param quads_array OArray of struct r_quad.
 * @paeam textlayout_color for each char quad
 */
vec2 r_textlayout_set_quads(struct r_textlayout *self, oobj quads_array, const char *text, vec4 textlayout_color);

/**
 * Queries the textlayout size.
 * Calls r_textlayout_set_generic internalls
 */
vec2 r_textlayout_size(struct r_textlayout *self, const char *text);

//
// virtual implementations:
//

vec2 r_textlayout__v_font35_char_uv(struct r_textlayout *self, char glyph);

vec2 r_textlayout__v_font55_char_uv(struct r_textlayout *self, char glyph);

vec2 r_textlayout__v_font58_char_uv(struct r_textlayout *self, char glyph);

#endif //R_TEXTLAYOUT_H
