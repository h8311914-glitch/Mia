#ifndef R_RSHADERBOXINDEXED_H
#define R_RSHADERBOXINDEXED_H

/**
 * @file RShaderBoxIndexed.h
 *
 * Object
 *
 * box rendered using an index texture and a palette texture.
 * From the main index texture, .r is an index (as ou8)
 *     pointing to an rgba color column from the palette
 */

#include "r/RShaderBox.h"


/** object id */
#define RShaderBoxIndexed_ID RShaderBox_ID "Indexed."

typedef struct
{
    RShaderBox super;

    RTex* palette;
} RShaderBoxIndexed;


/**
 * Initializes the object.
 * fx.xy -> palette offset.
 * fx.a -> applied on alpha (blending).
 * so a default fx would be (0, 0, 0, 1).
 * @param obj RShaderBoxIndexed object
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @param palette RTex object, each value (as ou8) from tex points to a color column from this texture
 *                NULL safe
 * @return obj casted as RShaderBoxIndexed
 */
RShaderBoxIndexed* RShaderBoxIndexed_init(oobj obj, oobj parent, oobj tex, bool move_tex, oobj palette);

/**
 * Creates a new RShaderBoxIndexed object.
 * fx.xy -> palette offset.
 * fx.a -> applied on alpha (blending).
 * so a default fx would be (0, 0, 0, 1).
 * @param parent to inherit from
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @param palette RTex object, each value (as ou8) from tex points to a color column from this texture
 *                NULL safe
 * @return The new object
 */
o_inline RShaderBoxIndexed* RShaderBoxIndexed_new(oobj parent, oobj tex, bool move_tex, oobj palette)
{
    OObj_DECL_IMPL_NEW(RShaderBoxIndexed, parent, tex, move_tex, palette);
}

//
// virtual implementations:
//

void RShaderBoxIndexed__v_render(oobj obj, oobj bound_tex, oobj program, int num, const struct r_proj* proj);


//
// object functions:
//


/**
 * @param obj RShaderBoxIndexed object
 * @return palette texture
 */
OObj_DECL_GETSET(RShaderBoxIndexed, oobj, palette)


#endif //R_RSHADERBOXINDEXED_H
