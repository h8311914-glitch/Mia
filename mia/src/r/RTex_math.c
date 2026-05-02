#include "r/RTex_math.h"
#include "r/RTex_channel.h"
#include "RTex_L.h"
#include "r/program.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"

// shared from RTex.c module
extern struct RTex__L_t RTex__L;

void RTex_add_scaled_into(oobj obj, vec4 add, vec4 scale, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));
    q.fx = add;
    q.fy = scale;

    RShaderQuad_tex_set(RTex__L.s_quad, obj, false);
    RShader_blend_set(RTex__L.s_quad, false);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_addscaled());
    RTex_quads(into, RTex__L.s_quad, &q, 1);
}


void RTex_inv_into(oobj obj, vec4 inv_mask, oobj into)
{
    mat4 mix;
    mix.col[0] = vec4_(inv_mask.r, 0, 0, 0);
    mix.col[1] = vec4_(0, inv_mask.g, 0, 0);
    mix.col[2] = vec4_(0, 0, inv_mask.b, 0);
    mix.col[3] = vec4_(0, 0, 0, inv_mask.a);
    RTex_mixer_into(obj, mix, into);;
}


void RTex_op_min_into(oobj obj, oobj tex_b, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));

    RShaderQuad_tex_set(RTex__L.s_quad_op, obj, false);
    RShader_blend_set(RTex__L.s_quad_op, false);
    RShaderQuadOp_tex_b_set(RTex__L.s_quad_op, tex_b);
    RShader_program_set(RTex__L.s_quad_op, r_program_QuadOp_min());
    RTex_quads(into, RTex__L.s_quad_op, &q, 1);
}
