#ifndef R_RSHADERBOX_H
#define R_RSHADERBOX_H

/**
 * @file RShaderBox.h
 *
 * Object
 *
 * Shader to render a batch of box's, see RBufferBox or RObjBox
 */

#include "r/RShader.h"
#include "r/RTex.h"


/** object id */
#define RShaderBox_ID RShader_ID "Box."

typedef struct
{
    RShader super;

    // init as vec4_(1)
    vec4 fx;
    
    bool pixel_snap;

    // may be moved into this object
    RTex* tex;
} RShaderBox;


/**
 * Initializes the object
 * @param obj RShaderBox object
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return obj casted as RShaderBox
 */
RShaderBox* RShaderBox_init(oobj obj, oobj parent, oobj tex, bool move_tex);

/**
 * Creates a new RShaderBox object
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return The new object
 */
o_inline RShaderBox* RShaderBox_new(oobj parent, oobj tex, bool move_tex)
{
    OObj_DECL_IMPL_NEW(RShaderBox, parent, tex, move_tex);
}

/**
 * Creates a new RShaderBox object.
 * fx -> rgba.
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return The new object
 */
RShaderBox* RShaderBox_new_rgba(oobj parent, oobj tex, bool move_tex);

/**
 * Creates a new RShaderBox object.
 * fx -> hsva shift.
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return The new object
 */
RShaderBox* RShaderBox_new_hsva(oobj parent, oobj tex, bool move_tex);
//
// virtual implementations:
//

void RShaderBox__v_render(oobj obj, oobj bound_tex, oobj program, int num, const struct r_proj* proj);


//
// object functions:
//

/**
 * @param obj RShaderBox object
 * @return reference fx uniform
 */
o_inline vec4* RShaderBox_fx(oobj obj)
{
    OObj_assert(obj, RShaderBox);
    RShaderBox* self = obj;
    return &self->fx;
}


/**
 * @param obj RShaderBox object
 * @return true if pixel snapping / vertex rounding should be used (default)
 * @note for pixel art its better to round the vertices to pixels in the shaders.
 *       But if the sprite texture > render target, artefacts appear from rounding!
 */
OObj_DECL_GETSET(RShaderBox, bool, pixel_snap)

/**
 * @param obj RShaderBox object
 * @return The used RTex
 */
OObj_DECL_GET(RShaderBox, RTex *, tex)

/**
 * @param obj RShaderBox object
 * @param tex The new RTex
 * @param del_old if true, o_del old used RTex
 * @return == tex
 */
o_inline RTex* RShaderBox_tex_set(oobj obj, oobj tex, bool del_old)
{
    OObj_assert(obj, RShaderBox);
    RShaderBox* self = obj;
    if (del_old) {
        o_del(self->tex);
    }
    self->tex = tex;
    return self->tex;
}


#endif //R_RSHADERBOX_H
