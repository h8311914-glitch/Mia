#include "r/RTex_color.h"
#include "RTex_L.h"
#include "r/program.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"

// shared from RTex.c module
extern struct RTex__L_t RTex__L;

void RTex_color_into(oobj obj, vec4 rgba, vec4 hsva, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));
    q.fx = rgba;
    q.fy = hsva;

    RShaderQuad_tex_set(RTex__L.s_quad, obj, false);
    RShader_blend_set(RTex__L.s_quad, false);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_rgba_hsva());
    RTex_quads(into, RTex__L.s_quad, &q, 1);
}

void RTex_rgba_to_hsva_into(oobj obj, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));

    RShaderQuad_tex_set(RTex__L.s_quad, obj, false);
    RShader_blend_set(RTex__L.s_quad, false);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_hsva_from_rgba());
    RTex_quads(into, RTex__L.s_quad, &q, 1);
}

void RTex_hsva_rgba_into(oobj obj, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));

    RShaderQuad_tex_set(RTex__L.s_quad, obj, false);
    RShader_blend_set(RTex__L.s_quad, false);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_hsva_into_rgba());
    RTex_quads(into, RTex__L.s_quad, &q, 1);
}
