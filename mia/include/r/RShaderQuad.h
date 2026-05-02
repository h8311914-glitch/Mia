#ifndef R_RSHADERQUAD_H
#define R_RSHADERQUAD_H

/**
 * @file RShaderQuad.h
 *
 * Object
 *
 * Shader to render a batch of Quads, see RBufferQuad or RObjQuad
 */

#include "r/RShader.h"
#include "r/RTex.h"


/** object id */
#define RShaderQuad_ID RShader_ID "Quad."

typedef struct
{
    RShader super;

    // init as fx=vec4_(1), fy=fz=vec4_(0)
    vec4 fx, fy, fz;
    
    bool pixel_snap;

    // may be moved into this object
    RTex* tex;
} RShaderQuad;


/**
 * Initializes the object
 * @param obj RShaderQuad object
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return obj casted as RShaderQuad
 */
RShaderQuad* RShaderQuad_init(oobj obj, oobj parent, oobj tex, bool move_tex);

/**
 * Creates a new RShaderQuad object
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return The new object
 */
o_inline RShaderQuad* RShaderQuad_new(oobj parent, oobj tex, bool move_tex)
{
    OObj_DECL_IMPL_NEW(RShaderQuad, parent, tex, move_tex);
}

/**
 * Creates a new RShaderQuad object.
 * s -> rgba.
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return The new object
 */
RShaderQuad* RShaderQuad_new_rgba(oobj parent, oobj tex, bool move_tex);

/**
 * Creates a new RShaderQuad object.
 * s -> rgba.
 * t -> hsva shift.
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return The new object
 */
RShaderQuad* RShaderQuad_new_rgba_hsva(oobj parent, oobj tex, bool move_tex);

//
// virtual implementations:
//

void RShaderQuad__v_render(oobj obj, oobj bound_tex, oobj program, int num, const struct r_proj* proj);


//
// object functions:
//

/**
 * @param obj RShaderQuad object
 * @return reference fx uniforms
 */
o_inline vec4* RShaderQuad_fx(oobj obj)
{
    OObj_assert(obj, RShaderQuad);
    RShaderQuad* self = obj;
    return &self->fx;
}

/**
 * @param obj RShaderQuad object
 * @return reference fy uniforms
 */
o_inline vec4* RShaderQuad_fy(oobj obj)
{
    OObj_assert(obj, RShaderQuad);
    RShaderQuad* self = obj;
    return &self->fy;
}

/**
 * @param obj RShaderQuad object
 * @return reference fz uniforms
 */
o_inline vec4* RShaderQuad_fz(oobj obj)
{
    OObj_assert(obj, RShaderQuad);
    RShaderQuad* self = obj;
    return &self->fz;
}


/**
 * @param obj RShaderQuad object
 * @return true if pixel snapping / vertex rounding should be used (default)
 * @note for pixel art its better to round the vertices to pixels in the shaders.
 *       But if the sprite texture > render target, artefacts appear from rounding!
 */
OObj_DECL_GETSET(RShaderQuad, bool, pixel_snap)

/**
 * @param obj RShaderQuad object
 * @return The used RTex
 */
OObj_DECL_GET(RShaderQuad, RTex *, tex)

/**
 * @param obj RShaderQuad object
 * @param tex The new RTex
 * @param del_old if true, o_del old used RTex
 * @return == tex
 */
o_inline RTex* RShaderQuad_tex_set(oobj obj, oobj tex, bool del_old)
{
    OObj_assert(obj, RShaderQuad);
    RShaderQuad* self = obj;
    if (del_old) {
        o_del(self->tex);
    }
    self->tex = tex;
    return self->tex;
}


#endif //R_RSHADERQUAD_H
