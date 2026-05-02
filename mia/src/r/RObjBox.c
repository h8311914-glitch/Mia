#include "r/RObjBox.h"
#include "o/OObj_builder.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/img.h"
#include "r/RBufferBox.h"
#include "r/RShaderBox.h"
#include "r/RShaderBoxIndexed.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"


RObjBox *RObjBox_init_ex(oobj obj, oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows,
                         oobj shader, bool move_shader)
{
    RObjBox *self = obj;
    o_clear(self, sizeof *self, 1);

    RObj_init(obj, parent, RObjBox__v_update, RObjBox__v_render);
    OObj_id_set(self, RObjBox_ID);

    num = o_max(0, num);
    self->num_rendered = oi32_MAX;

    self->boxes = OArray_new(self, NULL, sizeof(struct r_box), num);

    sprite_cols = o_max(1, sprite_cols);
    sprite_rows = o_max(1, sprite_rows);
    for (int i = 0; i < num; i++) {
        *RObjBox_at(self, i) = r_box_new_sprite(RTex_size(tex), sprite_cols, sprite_rows);
    }

    self->tex = tex;
    if (tex && move_tex) {
        o_move(tex, self);
    }

    if (move_shader) {
        o_move(shader, self);
    }
    self->shader_pipeline = OList_new(self, &shader, 1);

    self->buffer = RBufferBox_new(self);

    // vfuncs
    self->super.super.v_op_num = RObjBox__v_op_num;
    self->super.super.v_op_at = RObjBox__v_op_at;

    // update default values to gpu
    RObj_update(self);

    return self;
}

RObjBox *RObjBox_new_tex(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows)
{
    oobj shader = RShaderBox_new(parent, tex, false);
    return RObjBox_new_ex(parent, num, tex, move_tex, sprite_cols, sprite_rows, shader, true);
}

RObjBox *RObjBox_new(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows)
{
    oobj tex = RTex_new_file(parent, tex_file);
    return RObjBox_new_tex(parent, num, tex, true, sprite_cols, sprite_rows);
}

//
// Other shaders
//

RObjBox *RObjBox_new_tex_rgba(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows)
{
    oobj shader = RShaderBox_new_rgba(parent, tex, false);
    return RObjBox_new_ex(parent, num, tex, move_tex, sprite_cols, sprite_rows, shader, true);
}

RObjBox *RObjBox_new_rgba(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows)
{
    oobj tex = RTex_new_file(parent, tex_file);
    return RObjBox_new_tex_rgba(parent, num, tex, true, sprite_cols, sprite_rows);
}

RObjBox *RObjBox_new_tex_hsva(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows)
{
    oobj shader = RShaderBox_new_hsva(parent, tex, false);
    return RObjBox_new_ex(parent, num, tex, move_tex, sprite_cols, sprite_rows, shader, true);
}

RObjBox *RObjBox_new_hsva(oobj parent, int num, const char *tex_file, int sprite_cols, int sprite_rows)
{
    oobj tex = RTex_new_file(parent, tex_file);
    return RObjBox_new_tex_hsva(parent, num, tex, true, sprite_cols, sprite_rows);
}


RObjBox *RObjBox_new_indexed(oobj parent, int num, oobj tex, bool move_tex, int sprite_cols, int sprite_rows,
                             oobj palette, bool move_palette)
{
    oobj shader = RShaderBoxIndexed_new(parent, tex, false, palette);
    RObjBox *self = RObjBox_new_ex(parent, num, tex, move_tex, sprite_cols, sprite_rows, shader, true);
    // set new default fx
    struct r_box *boxes = OArray_data(self->boxes, struct r_box);
    for (int i = 0; i < num; i++) {
        boxes[i].fx = vec4_(0, 0, 0, 1);
    }
    return self;
}


//
// virtual implementations:
//

osize RObjBox__v_op_num(oobj obj)
{
    return RObjBox_num(obj);
}

void *RObjBox__v_op_at(oobj obj, osize idx)
{
    return RObjBox_at(obj, (int) idx);
}

void RObjBox__v_update(oobj obj)
{
    OObj_assert(obj, RObjBox);
    RObjBox *self = obj;

    RBuffer_update(self->buffer, OArray_data_void(self->boxes), OArray_num(self->boxes));
}

void RObjBox__v_render(oobj obj, oobj tex, const struct r_proj *proj)
{
    OObj_assert(obj, RObjBox);
    RObjBox *self = obj;

    for (osize i = 0; i < o_num(self->shader_pipeline); i++) {
        RShaderBox *shader = o_at(self->shader_pipeline, i);
        OObj_assert(shader, RShaderBox);

        // update tex reference
        shader->tex = self->tex;

        int num_render = o_min(OArray_num(self->boxes), self->num_rendered);
        RShader_render_ex(shader, self->buffer, tex, num_render, proj);
    }
}


//
// object functions:
//

oobj RObjBox_shader(oobj obj, int pipeline_idx)
{
    return o_at(RObjBox_shader_pipeline(obj), pipeline_idx);
}

int RObjBox_shader_set(oobj obj, int pipeline_idx, oobj shader, bool del_old)
{
    OList *pipeline = RObjBox_shader_pipeline(obj);
    if (pipeline_idx >= 0 && pipeline_idx < o_num(pipeline)) {
        oobj *ref = OList_at_ref(pipeline, pipeline_idx);
        if (del_old) {
            oobj old = *ref;
            o_del(old);
        }
        *ref = shader;
        return pipeline_idx;
    }

    OList_push(pipeline, shader);
    return o_num(pipeline) - 1;
}
