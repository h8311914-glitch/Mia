#ifndef R_ROBJQUAD_H
#define R_ROBJQUAD_H

/**
 * @file RObjQuad.h
 *
 * Object
 *
 * Renders a batch of quads.
 *
 * Operators:
 * o_num -> RObjQuad_num
 * o_at -> RObjQuad_at
 */

#include "o/OArray.h"
#include "RObj.h"
#include "RTex.h"
#include "quad.h"

/** object id */
#define RObjQuad_ID RObj_ID "Quad"

typedef struct {
    RObj super;

    // OArray of "struct r_quad"
    OArray *quads;

    // can be set lower num, o_min(num, num_rendered) is used to render the shaders
    // init with oi32_MAX
    int num_rendered;
    
    // RBufferQuad
    oobj buffer;

    // may be moved into this object
    RTex *tex;
    
    // OList of RShaderQuad
    oobj shader_pipeline;

} RObjQuad;


/**
 * Initializes the object
 * @param obj RObjQuad object
 * @param parent to inherit from
 * @param num of quads
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @param shader RShaderQuad object
 * @param move_shader if true, shader is o_move'd into this object
 * @return obj casted as RObjQuad
 */
RObjQuad *RObjQuad_init_ex(oobj obj, oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows, oobj shader, bool move_shader);


/**
 * Creates a new RObjQuad object
 * @param parent to inherit from
 * @param num of quads
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @param shader RShaderQuad object
 * @param move_shader if true, shader is o_move'd into this object
 * @return The new object
 */
o_inline RObjQuad *RObjQuad_new_ex(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows, oobj shader, bool move_shader)
{
    OObj_DECL_IMPL_NEW_SPECIAL(RObjQuad, ex, parent, num, tex, move_tex, sprite_cols, sprite_rows, shader, move_shader);
}

/**
 * Creates a new RObjQuad object
 * @param parent to inherit from
 * @param num of quads
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjQuad *RObjQuad_new_tex(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows);

/**
 * Creates a new RObjQuad object.
 * With an RTex_new_file as texture.
 * @param parent to inherit from
 * @param num of quads
 * @param tex_file to load as used tex
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjQuad *RObjQuad_new(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows);


//
// Other shaders
//

/**
 * Creates a new RObjQuad object.
 * s -> rgba.
 * @param parent to inherit from
 * @param num of quads
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjQuad *RObjQuad_new_tex_rgba(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows);

/**
 * Creates a new RObjQuad object.
 * With an RTex_new_file as texture.
 * s -> rgba.
 * @param parent to inherit from
 * @param num of quads
 * @param tex_file to load as used tex
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjQuad *RObjQuad_new_rgba(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows);


/**
 * Creates a new RObjQuad object.
 * s -> rgba.
 * t -> hsva shift.
 * @param parent to inherit from
 * @param num of quads
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjQuad *RObjQuad_new_tex_rgba_hsva(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows);

/**
 * Creates a new RObjQuad object.
 * With an RTex_new_file as texture.
 * s -> rgba.
 * t -> hsva shift.
 * @param parent to inherit from
 * @param num of quads
 * @param tex_file to load as used tex
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjQuad *RObjQuad_new_rgba_hsva(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows);



//
// virtual implementations:
//

/**
 * virtual operator function
 * @param obj RObjQuad object
 * @return number of r_quad's
 */
osize RObjQuad__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj RObjQuad object
 * @return r_quad at given idx
 */
void *RObjQuad__v_op_at(oobj obj, osize idx);


void RObjQuad__v_update(oobj obj);

void RObjQuad__v_render(oobj obj, oobj tex, const struct r_proj *proj);


//
// object functions:
//

/**
 * @param obj RObjQuad object
 * @return internal OArray of struct r_quad
 */
OObj_DECL_GET(RObjQuad, OArray *, quads)

/**
 * @param obj RObjQuad object
 * @param quads The new OArray of struct r_quad
 * @param del_old if true, o_del old used OArray
 * @return == tex
 */
o_inline OArray *RObjQuad_quads_set(oobj obj, oobj quads, bool del_old)
{
    OObj_assert(obj, RObjQuad);
    RObjQuad *self = obj;
    if(del_old) {
        o_del(self->quads);
    }
    self->quads = quads;
    return self->quads;
}

/**
 * @param obj RObjQuad object
 * @return number of r_quad's
 */
o_inline osize RObjQuad_num(oobj obj)
{
    OObj_assert(obj, RObjQuad);
    RObjQuad *self = obj;
    return OArray_num(self->quads);
}

/**
 * @param obj RObjQuad object
 * @return number of rendered r_quad's (o_min(num, num_rendered))
 */
OObj_DECL_GETSET(RObjQuad, int, num_rendered)

/**
 * @param obj RObjQuad object
 * @return r_quad at given index
 * @note asserts bounds
 */
o_inline struct r_quad *RObjQuad_at(oobj obj, osize idx)
{
    OObj_assert(obj, RObjQuad);
    RObjQuad *self = obj;
    return OArray_at(self->quads, idx, struct r_quad);
}

/**
 * @param obj RObjQuad object
 * @return The used RTex
 */
OObj_DECL_GET(RObjQuad, RTex *, tex)

/**
 * @param obj RObjQuad object
 * @param tex The new RTex
 * @param del_old if true, o_del old used RTex
 * @return == tex
 */
o_inline RTex *RObjQuad_tex_set(oobj obj, oobj tex, bool del_old)
{
    OObj_assert(obj, RObjQuad);
    RObjQuad *self = obj;
    if(del_old) {
        o_del(self->tex);
    }
    self->tex = tex;
    return self->tex;
}

/**
 * @param obj RObjQuad object
 * @return OList of RShaderQuad's, which are rendered one be the other
 */
OObj_DECL_GET(RObjQuad, oobj, shader_pipeline)

/**
 * @param obj RObjQuad object
 * @param pipeline_idx index in the shader pipeline (pass 0 for the default / first shader)
 * @return RShaderQuad at the given index of the pipeline
 */
oobj RObjQuad_shader(oobj obj, int pipeline_idx);

/**
 * @param obj RObjQuad object
 * @param pipeline_idx index in the shader pipeline (pass 0 for the default / first shader).
 *                     if index <0 or >= pipeline_size, the shader is pushed back
 * @param shader RShaderQuad object to set
 * @param del_old o_del's the old shader, if true
 * @return created pipeline_idx
 */
int RObjQuad_shader_set(oobj obj, int pipeline_idx, oobj shader, bool del_old);

/**
 * Adds a RShaderQuadBloom at the end of the shader pipeline.
 * Kernel set as an gauss 9x9
 * @param obj RObjQuad object
 * @param kernel RTex object for the kernel (.r used), NULL safe
 * @param color to render with the kernel
 * @return created RShaderQuadBloom object
 * @note mask set to vec4_(1, 1, 1, 0.66)
 *       .a is threshold (so 0.66)
 */
oobj RObjQuad_shader_add_bloom(oobj obj, oobj kernel,vec4 color);



/**
 * Adds a RShaderQuadContour at the front of the shader pipeline.
 * @param obj RObjQuad object
 * @param kernel RTex object for the kernel (.r used), NULL safe
 * @param color to render with the kernel
 * @return created RShaderQuadContour object
 */
oobj RObjQuad_shader_add_contour(oobj obj, oobj kernel, vec4 color);




#endif //R_ROBJQUAD_H
