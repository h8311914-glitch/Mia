#ifndef R_ROBJBOX_H
#define R_ROBJBOX_H

/**
 * @file RObjBox.h
 *
 * Object
 *
 * Renders a batch of box's.
 *
 * Operators:
 * o_num -> RObjBox_num
 * o_at -> RObjBox_at
 */

#include "o/OArray.h"
#include "RObj.h"
#include "RTex.h"
#include "box.h"

/** object id */
#define RObjBox_ID RObj_ID "Box"

typedef struct {
    RObj super;

    // OArray of "struct r_box"
    OArray *boxes;

    // can be set lower num, o_min(num, num_rendered) is used to render the shaders
    // init with oi32_MAX
    int num_rendered;
    
    // RBufferBox
    oobj buffer;

    // may be moved into this object
    RTex *tex;
    
    // OList of RShaderBox
    oobj shader_pipeline;

} RObjBox;


/**
 * Initializes the object
 * @param obj RObjBox object
 * @param parent to inherit from
 * @param num of boxs
 * @param tex RTex object, NULL safe
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @param shader RShaderBox object
 * @param move_shader if true, shader is o_move'd into this object
 * @return obj casted as RObjBox
 */
RObjBox *RObjBox_init_ex(oobj obj, oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows, oobj shader, bool move_shader);

/**
 * Creates a new RObjBox object
 * @param parent to inherit from
 * @param num of boxs
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @param shader RShaderBox object
 * @param move_shader if true, shader is o_move'd into this object
 * @return The new object
 */
o_inline RObjBox *RObjBox_new_ex(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows, oobj shader, bool move_shader)
{
    OObj_DECL_IMPL_NEW_SPECIAL(RObjBox, ex, parent, num, tex, move_tex, sprite_cols, sprite_rows, shader, move_shader);
}


/**
 * Creates a new RObjBox object.
 * @param parent to inherit from
 * @param num of boxs
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjBox *RObjBox_new_tex(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows);

/**
 * Creates a new RObjBox object.
 * With an RTex_new_file as texture.
 * @param parent to inherit from
 * @param num of boxs
 * @param tex_file to load as used tex
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjBox *RObjBox_new(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows);


//
// Other shaders
//


/**
 * Creates a new RObjBox object.
 * fx -> rgba.
 * @param parent to inherit from
 * @param num of boxs
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjBox *RObjBox_new_tex_rgba(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows);

/**
 * Creates a new RObjBox object.
 * With an RTex_new_file as texture.
 * fx -> rgba.
 * @param parent to inherit from
 * @param num of boxs
 * @param tex_file to load as used tex
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjBox *RObjBox_new_rgba(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows);


/**
 * Creates a new RObjBox object.
 * fx -> hsva shift.
 * @param parent to inherit from
 * @param num of quads
 * @param tex RTex object, NULL safe;
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjBox *RObjBox_new_tex_hsva(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows);

/**
 * Creates a new RObjBox object.
 * With an RTex_new_file as texture.
 * fx -> hsva shift.
 * @param parent to inherit from
 * @param num of quads
 * @param tex_file to load as used tex
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 * @return The new object
 */
RObjBox *RObjBox_new_hsva(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows);

/**
 * Creates a new RObjBox object.
 * Uses RShaderBoxIndexed.
 * The main tex is read as index ou8 texture, 
 * each .r pixel value will be used to access .col from the color palette 
 * fx.xy -> palette offset.
 * fx.a -> applied on alpha (blending).
 * Default fx is set to (0, 0, 0, 1).
 * @param parent to inherit from
 * @param num of boxs
 * @param tex RTex object, NULL safe.
 * @param move_tex if true, tex is o_move'd into this object
 * @param sprite_cols, sprite_rows number of full grid sprites or <=1 for a single
 *  @param palette RTex of the palette, NULL safe.
 * @param move_palette if true, palette is o_move'd into this object
 * @return The new object
 */
RObjBox *RObjBox_new_indexed(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows,
                             oobj palette, bool move_palette);


//
// virtual implementations:
//

/**
 * virtual operator function
 * @param obj RObjBox object
 * @return number of r_box's
 */
osize RObjBox__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj RObjBox object
 * @return r_box at given idx
 */
void *RObjBox__v_op_at(oobj obj, osize idx);


void RObjBox__v_update(oobj obj);

void RObjBox__v_render(oobj obj, oobj tex, const struct r_proj *proj);


//
// object functions:
//

/**
 * @param obj RObjBox object
 * @return internal OArray of struct r_box
 */
OObj_DECL_GET(RObjBox, OArray *, boxes)

/**
 * @param obj RObjBox object
 * @param boxes The new OArray of struct r_box
 * @param del_old if true, o_del old used OArray
 * @return == tex
 */
o_inline OArray *RObjBox_boxes_set(oobj obj, oobj boxes, bool del_old)
{
    OObj_assert(obj, RObjBox);
    RObjBox *self = obj;
    if(del_old) {
        o_del(self->boxes);
    }
    self->boxes = boxes;
    return self->boxes;
}

/**
 * @param obj RObjBox object
 * @return number of r_box's
 */
o_inline osize RObjBox_num(oobj obj)
{
    OObj_assert(obj, RObjBox);
    RObjBox *self = obj;
    return OArray_num(self->boxes);
}

/**
 * @param obj RObjBox object
 * @return number of rendered r_box's (o_min(num, num_rendered))
 */
OObj_DECL_GETSET(RObjBox, int, num_rendered)

/**
 * @param obj RObjBox object
 * @return r_box at given index
 * @note asserts bounds
 */
o_inline struct r_box *RObjBox_at(oobj obj, osize idx)
{
    OObj_assert(obj, RObjBox);
    RObjBox *self = obj;
    return OArray_at(self->boxes, idx, struct r_box);
}

/**
 * @param obj RObjBox object
 * @return The used RTex
 */
OObj_DECL_GET(RObjBox, RTex *, tex)

/**
 * @param obj RObjBox object
 * @param tex The new RTex
 * @param del_old if true, o_del old used RTex
 * @return == tex
 */
o_inline RTex *RObjBox_tex_set(oobj obj, oobj tex, bool del_old)
{
    OObj_assert(obj, RObjBox);
    RObjBox *self = obj;
    if(del_old) {
        o_del(self->tex);
    }
    self->tex = tex;
    return self->tex;
}

/**
 * @param obj RObjBox object
 * @return OList of RShaderBox's, which are rendered one be the other
 */
OObj_DECL_GET(RObjBox, oobj, shader_pipeline)

/**
 * @param obj RObjBox object
 * @param pipeline_idx index in the shader pipeline (pass 0 for the default / first shader)
 * @return RShaderBox at the given index of the pipeline
 */
oobj RObjBox_shader(oobj obj, int pipeline_idx);

/**
 * @param obj RObjBox object
 * @param pipeline_idx index in the shader pipeline (pass 0 for the default / first shader).
 *                     if index <0 or >= pipeline_size, the shader is pushed back
 * @param shader RShaderBox object to set
 * @param del_old o_del's the old shader, if true
 * @return created pipeline_idx
 */
int RObjBox_shader_set(oobj obj, int pipeline_idx, oobj shader, bool del_old);





#endif //R_ROBJBOX_H
