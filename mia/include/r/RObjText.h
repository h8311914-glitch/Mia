#ifndef R_ROBJTEXT_H
#define R_ROBJTEXT_H

/**
 * @file RObjText.h
 *
 * Object
 *
 * Renders a text with an RObjBox (rect for each character; _color shader)
 *
 * update is a noop!
 * Call update on RObjText_boxes_ro() if you change these!
 */

#include "RObj.h"
#include "textlayout.h"
#include "RObjBox.h"

/** object id */
#define RObjText_ID RObj_ID "Text."

typedef struct {
    RObj super;

    // left top (see u/pose.h)
    mat4 pose;

    struct r_textlayout layout;

    // RObjBox
    oobj boxes_ro;

    bool queue_update;
} RObjText;


/**
 * Initializes the object
 * @param obj RObjText object
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @param itext interface, see "r/text.h"
 * @return obj casted as RObjText
 */
RObjText *RObjText_init(oobj obj, oobj parent, oobj tex, bool move_tex, struct r_textlayout layout);


/**
 * Creates a new RObjText object
 * @param parent to inherit from
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param itext interface, see "r/text.h"
 * @return The new object
 */
o_inline RObjText *RObjText_new(oobj parent, oobj tex, bool move_tex, struct r_textlayout layout)
{
    OObj_DECL_IMPL_NEW(RObjText, parent, tex, move_tex, layout);
}


/**
 * Creates a new RObjText object.
 * Loads font file "$r/font35.png" with 3x5 size
 * @param parent to inherit from
 * @param opt_text if not NULL -> RObjText_text_set
 * @return The new object
 * @note inits text_mode as UPPER, even if lower case is available
 */
RObjText *RObjText_new_font35(oobj parent, const char *opt_text);

/**
 * Creates a new RObjText object.
 * Loads font file "$r/font35_shadow.png" with 3x5 size
 * The default shadow is bvec4_(0, 0, 0, 128) and is equal to a contouring shader
 * with 2x2 kernel aligned bottom right
 * @param parent to inherit from
 * @param opt_text if not NULL -> RObjText_text_set
 * @param opt_shadow_color if NULL, bvec4_(0, 0, 0, 128) is used from the font sheet,
 *                         else replaced by the given color
 * @return The new object
 * @note inits text_mode as UPPER, even if lower case is available
 */
RObjText *RObjText_new_font35_shadow(oobj parent, const char *opt_text, bvec4 *opt_shadow_color);


/**
 * Creates a new RObjText object.
 * Loads font file "$r/font55.png" with 5x5 size
 * @param parent to inherit from
 * @param opt_text if not NULL -> RObjText_text_set
 * @return The new object
 * @note inits text_mode as UPPER, even if lower case is available
 */
RObjText *RObjText_new_font55(oobj parent, const char *opt_text);

/**
 * Creates a new RObjText object.
 * Loads font file "$r/font55_shadow.png" with 5x5 size
 * The default shadow is bvec4_(0, 0, 0, 128) and is equal to a contouring shader
 * with 2x2 kernel aligned bottom right
 * @param parent to inherit from
 * @param opt_text if not NULL -> RObjText_text_set
 * @param opt_shadow_color if NULL, bvec4_(0, 0, 0, 128) is used from the font sheet,
 *                         else replaced by the given color
 * @return The new object
 * @note inits text_mode as UPPER, even if lower case is available
 */
RObjText *RObjText_new_font55_shadow(oobj parent, const char *opt_text, bvec4 *opt_shadow_color);


/**
 * Creates a new RObjText object.
 * Loads font file "$r/font58.png" with 5x8 size
 * @param parent to inherit from
 * @param opt_text if not NULL -> RObjText_text_set
 * @return The new object
 * @note inits text_mode as UNCHANGED
 */
RObjText *RObjText_new_font58(oobj parent, const char *opt_text);

/**
 * Creates a new RObjText object.
 * Loads font file "$r/font58_shadow.png" with 5x8 size
 * The default shadow is bvec4_(0, 0, 0, 128) and is equal to a contouring shader
 * with 2x2 kernel aligned bottom right
 * @param parent to inherit from
 * @param opt_text if not NULL -> RObjText_text_set
 * @param opt_shadow_color if NULL, bvec4_(0, 0, 0, 128) is used from the font sheet,
 *                         else replaced by the given color
 * @return The new object
 * @note inits text_mode as UNCHANGED
 */
RObjText *RObjText_new_font58_shadow(oobj parent, const char *opt_text, bvec4 *opt_shadow_color);


//
// virtual implementations:
//

void RObjText__v_update(oobj obj);

void RObjText__v_render(oobj obj, oobj tex, const struct r_proj *proj);


//
// object functions:
//

/**
 * @param obj RObjText object
 * @return The used text pose (top left corner)
 * @note combined with used camera and passed to the quads as camera.
         Position will be set on a real pixel before combining.
         See u/pose.h
 */
OObj_DECL_GETSETREF(RObjText, mat4, pose)

/**
 * @param obj RObjText object
 * @return The used text interface, see "r/textlayout.h"
 */
OObj_DECL_GETSETREF(RObjText, struct r_textlayout, layout)

/**
 * @param obj RObjText object
 * @return RObjBoxes object used to render the characters
 * @note call RObj_update on the boxes, if changed (color, etc.)
 */
OObj_DECL_GET(RObjText, oobj, boxes_ro)


/**
 * @param obj RObjText object
 * @return internal OArray of struct r_box, from RObjText_boxes_ro
 * @note call RObj_update on the boxes, if changed (color, etc.)
 */
o_inline OArray *RObjText_boxes(oobj obj)
{
    return RObjBox_boxes(RObjText_boxes_ro(obj));
}

/**
 * @param obj RObjText object
 * @return True if the next update call of this ro, updates the underlying RObjBoxes
 */
OObj_DECL_GETSET(RObjText, bool, queue_update)

/**
 * Wrapper to the internal itext interface, text_size
 * @param obj RObjText object
 * @return The needed text_size for the given text string
 */
o_inline vec2 RObjText_text_size(oobj obj)
{
    OObj_assert(obj, RObjText);
    RObjText *self = obj;
    return self->layout.text_size;
}

/**
 * Wrapper to the internal itext interface, case
 * @param obj RObjText object
 * @return The needed used text case
 */
o_inline enum r_textlayout_casing RObjText_casing(oobj obj)
{
    OObj_assert(obj, RObjText);
    RObjText *self = obj;
    return self->layout.casing;
}

/**
 * Wrapper to the internal itext interface, case
 * @param obj RObjText object
 * @return The needed used text case
 * @note Sets RObjText_queue_update.
 */
o_inline enum r_textlayout_casing RObjText_casing_set(oobj obj, enum r_textlayout_casing set)
{
    OObj_assert(obj, RObjText);
    RObjText *self = obj;
    self->layout.casing = set;
    self->queue_update = true;
    return self->layout.casing;
}

/**
 * @param obj RObjText object
 * @param size for each character (quad uv + quad pose)0
 * @note useable to create shader effects on the text fonts, like shadow or outlining.
         The font files have typically transparent borders around each font for that.
         If you want to have a bigger printed size, see RObjText_pose.
         Sets RObjText_queue_update.
 */
void RObjText_uv_size_set(oobj obj, vec2 size);


/**
 * Resets the text to render.
 * A newline character results in a new line...
 * Text index matches the RObjText_quads index
 * rendered right down from the pose. So pose is top left of the text.
 * @param obj RObjText object
 * @param text to set
 * @param text_color rgba for the text
 * @return size if the full set text block
 * @note sets RObjQuad_num_rendered_set to the o_strlen(text).
         Sets RObjText_queue_update.
 */
vec2 RObjText_text_set(oobj obj, const char *text, vec4 text_color);

/**
 * Sets a (single) color for all used quads
 * @param obj RObjText object
 * @param text_color to set.
 * @note Sets RObjText_queue_update.
 */
void RObjText_color_set(oobj obj, vec4 text_color);


#endif //R_ROBJTEXT_H
