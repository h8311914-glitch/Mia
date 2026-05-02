#include "r/RShaderBox.h"
#include "o/OObj_builder.h"
#include "r/RProgram.h"
#include "r/program.h"
#include "r/gl.h"


// rare case where it's fine to use a global to speed up performance
extern int r__draw_calls;


RShaderBox *RShaderBox_init(oobj obj, oobj parent, oobj tex, bool move_tex)
{
    RShaderBox *self = obj;
    o_clear(self, sizeof *self, 1);

    RShader_init(obj, parent, r_program_Box(), RShaderBox__v_render);
    OObj_id_set(self, RShaderBox_ID);

    self->tex = tex;
    if (tex && move_tex) {
        o_move(tex, self);
    }

    self->fx = vec4_(1);
    self->pixel_snap = true;

    return self;
}


RShaderBox* RShaderBox_new_rgba(oobj parent, oobj tex, bool move_tex)
{
    RShaderBox *self = RShaderBox_new(parent, tex, move_tex);
    RShader_program_set(self, r_program_Box_rgba());
    return self;
}

RShaderBox* RShaderBox_new_hsva(oobj parent, oobj tex, bool move_tex)
{
    RShaderBox *self = RShaderBox_new(parent, tex, move_tex);
    RShader_program_set(self, r_program_Box_hsva());
    return self;
}

//
// virtual implementations:
//

void RShaderBox__v_render(oobj obj, oobj bound_tex, oobj program, int num, const struct r_proj *proj)
{
    OObj_assert(obj, RShaderBox);
    RShaderBox *self = obj;

    float mirror_y = bound_tex? -1.0 : +1.0;

    // common uniforms
    int u_c_pixel_snap = (self->pixel_snap&&proj->scale>=1)? 1 : 0;
    RProgram_uniform_int(program, "u_c_pixel_snap", &u_c_pixel_snap, 1);
    RProgram_uniform_float(program, "u_c_viewport_scale", &proj->scale, 1);
    RProgram_uniform_float(program, "u_c_viewport_mirror_y", &mirror_y, 1);
    RProgram_uniform_vec2(program, "u_c_viewport_size_half", &proj->vpsh, 1);
    RProgram_uniform_vec2(program, "u_c_viewport_even_offset", &proj->vpeo, 1);


    // fx
    RProgram_uniform_vec4(program, "u_fx", &self->fx, 1);

    // basic uniforms:
    RProgram_uniform_mat4(program, "u_vp", &proj->cam, 1);

    vec2 tex_scale = RTex_get_tex_scale(self->tex);
    RProgram_uniform_vec2(program, "u_tex_scale", &tex_scale, 1);

    RProgram_uniform_tex(program, "u_tex", 0, RTex_tex(self->tex));
    
    // draw call
    r__draw_calls++;
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, num);

    RProgram_uniform_tex_off(program, 0);
}

