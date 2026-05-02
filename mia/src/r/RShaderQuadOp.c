#include "r/RShaderQuadOp.h"
#include "o/OObj_builder.h"
#include "r/RProgram.h"
#include "r/program.h"
#include "r/tex.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"

//
// public
//


RShaderQuadOp* RShaderQuadOp_init(oobj obj, oobj parent, oobj tex, bool move_tex)
{
    RShader *super_shader = obj;
    RShaderQuadOp *self = obj;
    o_clear(self, sizeof *self, 1);

    RShaderQuad_init(obj, parent, tex, move_tex);
    OObj_id_set(self, RShaderQuadOp_ID);

    self->tex_b = r_tex_white();

    // overwrite program
    super_shader->program = r_program_QuadOp_min();

    // default is not blended
    RShader_blend_set(self, false);
    
    // update vfunc
    super_shader->v_render = RShaderQuadOp__v_render;

    return self;
}

//
// virtual implementations:
//


void RShaderQuadOp__v_render(oobj obj, oobj bound_tex, oobj program, int num, const struct r_proj *proj)
{
    OObj_assert(obj, RShaderQuadOp);
    RShaderQuadOp *self = obj;

    RProgram_uniform_tex(program, "u_tex_b", 1, RTex_tex(self->tex_b));

    RShaderQuad__v_render(obj, bound_tex, program, num, proj);
}
