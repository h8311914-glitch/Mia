#ifndef R_RSHADERQUADOP_H
#define R_RSHADERQUADOP_H

/**
 * @file RShaderQuadOp.h
 *
 * Object
 *
 * Operations with two images. Makes only sense for blend=off, which is the default for this shader
 */

#include "RShaderQuad.h"

/** object id */
#define RShaderQuadOp_ID RShaderQuad_ID "Op."

typedef struct
{
    RShaderQuad super;

    // RTex b (together with main tex) for the operations
    oobj tex_b;


} RShaderQuadOp;


/**
 * Initializes the object.
 * @param obj RShaderQuadOp object
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return obj casted as RShaderQuadOp
 */
RShaderQuadOp* RShaderQuadOp_init(oobj obj, oobj parent, oobj tex, bool move_tex);


/**
 * Creates a new RShaderQuadOp object.
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @return The new object
 */
o_inline RShaderQuadOp* RShaderQuadOp_new(oobj parent, oobj tex, bool move_tex)

{
    OObj_DECL_IMPL_NEW(RShaderQuadOp, parent, tex, move_tex);
}


//
// virtual implementations:
//

void RShaderQuadOp__v_render(oobj obj, oobj bound_tex, oobj program, int num, const struct r_proj* proj);


//
// object functions:
//

/**
 * @param obj RShaderQuadOp object
 * @return "b"" RTex to operate with
 */
OObj_DECL_GETSET(RShaderQuadOp, oobj, tex_b)


#endif //R_RSHADERQUADOP_H
