#include "r/RShaderBoxIndexed.h"
#include "o/OObj_builder.h"
#include "r/RProgram.h"
#include "r/program.h"
#include "r/tex.h"
#include "m/vec/vec2.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"

//
// public
//


RShaderBoxIndexed* RShaderBoxIndexed_init(oobj obj, oobj parent, oobj tex, bool move_tex, oobj palette)
{
    RShader *super_shader = obj;
    RShaderBoxIndexed *self = obj;
    o_clear(self, sizeof *self, 1);

    RShaderBox_init(obj, parent, tex, move_tex);
    OObj_id_set(self, RShaderBoxIndexed_ID);

    self->palette = palette;

    // set new default fx
    *RShaderBox_fx(self) = vec4_(0, 0, 0, 1);

    // overwrite program
    super_shader->program = r_program_BoxIndexed();
    
    // update vfunc
    super_shader->v_render = RShaderBoxIndexed__v_render;

    return self;
}

//
// virtual implementations:
//


void RShaderBoxIndexed__v_render(oobj obj, oobj bound_tex, oobj program, int num, const struct r_proj *proj)
{
    OObj_assert(obj, RShaderBoxIndexed);
    RShaderBoxIndexed *self = obj;

    RProgram_uniform_tex(program, "u_palette", 1, RTex_tex(self->palette));

    RShaderBox__v_render(obj, bound_tex, program, num, proj);
}
