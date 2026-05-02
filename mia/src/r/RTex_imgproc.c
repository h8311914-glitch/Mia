#include "r/RTex_imgproc.h"
#include "r/RTex_blit.h"
#include "RTex_L.h"
#include "r/program.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"

// shared from RTex.c module
extern struct RTex__L_t RTex__L;


void RTex_conv_into(oobj obj, oobj kernel, ivec2 offset, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));

    RShaderQuad_tex_set(RTex__L.s_quad_kernel, obj, false);
    RShader_blend_set(RTex__L.s_quad_kernel, false);
    RShaderQuadKernel_kernel_set(RTex__L.s_quad_kernel, kernel, false);
    RShaderQuadKernel_offset_set(RTex__L.s_quad_kernel, offset);
    RShaderQuadKernel_color_set(RTex__L.s_quad_kernel, R_WHITE);
    RShader_program_set(RTex__L.s_quad_kernel, r_program_QuadKernel_conv());
    RTex_quads(into, RTex__L.s_quad_kernel, &q, 1);
}

void RTex_blur_into(oobj obj, ivec2 size, oobj into)
{
    oobj kernel = RTex_new_kernel(obj, m_2(size), 1.0f, false, 1.0f, R_FORMAT_R_F32);
    RTex_conv_into(obj, kernel, ivec2_(0), into);
    o_del(kernel);
}

void RTex_gauss_into(oobj obj, ivec2 size, vec2 sigma, oobj into)
{
    oobj kernel = RTex_new_kernel_gauss(obj, m_2(size), sigma, 1.0f, R_FORMAT_R_F32);
    RTex_conv_into(obj, kernel, ivec2_(0), into);
    o_del(kernel);
}

void RTex_dilate_into(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));

    RShaderQuad_tex_set(RTex__L.s_quad_kernel, obj, false);
    RShader_blend_set(RTex__L.s_quad_kernel, false);
    RShaderQuadKernel_kernel_set(RTex__L.s_quad_kernel, kernel, false);
    RShaderQuadKernel_offset_set(RTex__L.s_quad_kernel, offset);
    RShaderQuadKernel_color_set(RTex__L.s_quad_kernel, color);
    RShaderQuadKernel_mask_set(RTex__L.s_quad_kernel, mask);
    RShader_program_set(RTex__L.s_quad_kernel, r_program_QuadKernel_contour());
    RTex_quads(into, RTex__L.s_quad_kernel, &q, 1);
}

void RTex_erode_into(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));

    RShaderQuad_tex_set(RTex__L.s_quad_kernel, obj, false);
    RShader_blend_set(RTex__L.s_quad_kernel, false);
    RShaderQuadKernel_kernel_set(RTex__L.s_quad_kernel, kernel, false);
    RShaderQuadKernel_offset_set(RTex__L.s_quad_kernel, offset);
    RShaderQuadKernel_color_set(RTex__L.s_quad_kernel, color);
    RShaderQuadKernel_mask_set(RTex__L.s_quad_kernel, mask);
    RShader_program_set(RTex__L.s_quad_kernel, r_program_QuadKernel_contour_inv());
    RTex_quads(into, RTex__L.s_quad_kernel, &q, 1);
}

void RTex_contour_into(oobj obj, oobj kernel, ivec2 offset, vec4 mask, vec4 color, oobj into)
{
    RTex_dilate_into(obj, kernel, offset, mask, color, into);
    RTex_blend(into, obj, 0, 0);
}

