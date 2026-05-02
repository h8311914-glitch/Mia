#include "r/RTex_channel.h"
#include "RTex_L.h"
#include "r/program.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"

// shared from RTex.c module
extern struct RTex__L_t RTex__L;


void RTex_mixer_into(oobj obj, mat4 rgba_matrix, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));

    // mixer uses sprite for r
    // fx fy fz for g b a
    q.sprite = rgba_matrix.col[0];
    q.fx = rgba_matrix.col[1];
    q.fy = rgba_matrix.col[2];
    q.fz = rgba_matrix.col[3];

    RShaderQuad_tex_set(RTex__L.s_quad, obj, false);
    RShader_blend_set(RTex__L.s_quad, false);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_mixer());
    RTex_quads(into, RTex__L.s_quad, &q, 1);
}


void RTex_channels_into(oobj obj, ivec4 channel, oobj into)
{
    mat4 mix;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            mix.col[i].v[j] = (float) (channel.v[i] == j);
        }
    }
    RTex_mixer_into(obj, mix, into);
}

void RTex_channels_merge_into(oobj r, oobj g, oobj b, oobj a, oobj into)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(r)));

    RShaderQuadChannelmerge_r_set(RTex__L.s_quad_channelmerge, r);
    RShaderQuadChannelmerge_g_set(RTex__L.s_quad_channelmerge, g);
    RShaderQuadChannelmerge_b_set(RTex__L.s_quad_channelmerge, b);
    RShaderQuadChannelmerge_a_set(RTex__L.s_quad_channelmerge, a);
    RShader_blend_set(RTex__L.s_quad_channelmerge, false);
    RTex_quads(into, RTex__L.s_quad_channelmerge, &q, 1);
}
