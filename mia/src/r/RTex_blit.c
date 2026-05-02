#include "r/RTex_blit.h"
#include "RTex_L.h"
#include "m/vec/vec2.h"
#include "m/mat/mat4.h"
#include "r/tex.h"
#include "r/program.h"
#include "r/gl.h"

// shared from RTex.c module
extern struct RTex__L_t RTex__L;

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"


void RTex_blit_pose(oobj obj, oobj tex, mat4 pose, mat4 uv_pose)
{
    struct r_quad q = {0};
    q.pose = pose;
    q.uv_pose = uv_pose;

    RShaderQuad_tex_set(RTex__L.s_quad, tex, false);
    RShader_blend_set(RTex__L.s_quad, false);
    RShader_program_set(RTex__L.s_quad, r_program_Quad());
    RTex_quads(obj, RTex__L.s_quad, &q, 1);
}

void RTex_blit_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect)
{
    struct r_box b = {0};
    b.rect = rect;
    b.uv_rect = uv_rect;

    RShaderBox_tex_set(RTex__L.s_box, tex, false);
    RShader_blend_set(RTex__L.s_box, false);
    RShader_program_set(RTex__L.s_box, r_program_Box());
    RTex_boxes(obj, RTex__L.s_box, &b, 1);
}

void RTex_blit(oobj obj, oobj tex, float x, float y)
{
    vec4 rect = RTex_rect(tex, x, y);
    vec4 uv_rect = RTex_rect(tex, 0, 0);
    RTex_blit_rect(obj, tex, rect, uv_rect);
}

void RTex_blit_color_pose(oobj obj, oobj tex, mat4 pose, mat4 uv_pose, vec4 rgba, vec4 hsva)
{
    struct r_quad q = {0};
    q.pose = pose;
    q.uv_pose = uv_pose;
    q.fx = rgba;
    q.fy = hsva;

    RShaderQuad_tex_set(RTex__L.s_quad, tex, false);
    RShader_blend_set(RTex__L.s_quad, false);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_rgba_hsva());
    RTex_quads(obj, RTex__L.s_quad, &q, 1);
}

void RTex_blit_color_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect, vec4 rgba, vec4 hsva)
{
    mat4 pose = mat4_new(
        rect.width, 0, 0, 0,
        0, rect.height, 0, 0,
        0, 0, 1, 0,
        rect.x, rect.y, 0, 1
    );
    mat4 uv = mat4_new(
        uv_rect.width, 0, 0, 0,
        0, uv_rect.height, 0, 0,
        0, 0, 1, 0,
        uv_rect.x, uv_rect.y, 0, 1
    );
    RTex_blit_color_pose(obj, tex, pose, uv, rgba, hsva);
}

void RTex_blit_color(oobj obj, oobj tex, float x, float y, vec4 rgba, vec4 hsva)
{
    vec4 rect = RTex_rect(tex, x, y);
    vec4 uv_rect = RTex_rect(tex, 0, 0);
    RTex_blit_color_rect(obj, tex, rect, uv_rect, rgba, hsva);
}


void RTex_blit_sprite_color(oobj obj, oobj tex, float x, float y, vec2 sprite, ivec2 num_sprites, vec4 rgba, vec4 hsva)
{
    vec2 size = vec2_div_v(RTex_size(tex), vec2_cast_int(num_sprites.v));
    mat4 pose = mat4_new(
        size.x, 0, 0, 0,
        0, size.y, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    );
    mat4 uv = mat4_new(
        size.x, 0, 0, 0,
        0, size.y, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    struct r_quad q = {0};
    q.pose = pose;
    q.uv_pose = uv;
    q.sprite.xy = sprite;
    q.sprite.zw = size;
    q.fx = rgba;
    q.fy = hsva;

    RShaderQuad_tex_set(RTex__L.s_quad, tex, false);
    RShader_blend_set(RTex__L.s_quad, false);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_rgba_hsva());
    RTex_quads(obj, RTex__L.s_quad, &q, 1);
}


void RTex_blit_sprite(oobj obj, oobj tex, float x, float y, vec2 sprite, ivec2 num_sprites)
{
    RTex_blit_sprite_color(obj, tex, x, y, sprite, num_sprites, vec4_(1), vec4_(0));
}

void RTex_blit_indexed_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect, oobj palette, vec2 palette_offset)
{
    struct r_box b = {0};
    b.rect = rect;
    b.uv_rect = uv_rect;
    b.fx = vec4_(palette_offset.x, palette_offset.y, 0, 1);

    RShaderBox_tex_set(RTex__L.s_box_indexed, tex, false);
    RShaderBoxIndexed_palette_set(RTex__L.s_box_indexed, palette);
    RShader_blend_set(RTex__L.s_box_indexed, false);
    RShader_program_set(RTex__L.s_box_indexed, r_program_BoxIndexed());
    RTex_boxes(obj, RTex__L.s_box_indexed, &b, 1);
}

void RTex_blit_indexed(oobj obj, oobj tex, float x, float y, oobj palette, vec2 palette_offset)
{
    vec4 rect = RTex_rect(tex, x, y);
    vec4 uv_rect = RTex_rect(tex, 0, 0);
    RTex_blit_indexed_rect(obj, tex, rect, uv_rect, palette, palette_offset);
}

void RTex_blit_back_ex(oobj obj, ivec4 viewport_obj, ivec4 viewport_back, enum RTex_filter_modes filter)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;

    RTex_use(self, 1);

    if (viewport_obj.x < 0) {
        viewport_obj = self->viewport;
    }
    viewport_obj.y = self->size.y - viewport_obj.y - viewport_obj.v3;

    if (viewport_back.x < 0) {
        viewport_back = RTex__L.back_gl_viewport;
    }

    glBindFramebuffer(GL_READ_FRAMEBUFFER, RTex__L.back_fbo);

    // cause we do trick opengl to use top row first, we need a mirror here
    glBlitFramebuffer(viewport_back.x, viewport_back.y,
                      viewport_back.x + viewport_back.v2, viewport_back.y + viewport_back.v3,
                      viewport_obj.x, viewport_obj.y + viewport_obj.v3,
                      viewport_obj.x + viewport_obj.v2, viewport_obj.y,
                      GL_COLOR_BUFFER_BIT,
                      filter == RTex_filter_LINEAR ? GL_LINEAR : GL_NEAREST);

    r_error_check("RTex_blit_fbo_ex");
}

void RTex_blend_pose(oobj obj, oobj tex, mat4 pose, mat4 uv_pose)
{
    struct r_quad q = {0};
    q.pose = pose;
    q.uv_pose = uv_pose;

    RShaderQuad_tex_set(RTex__L.s_quad, tex, false);
    RShader_blend_set(RTex__L.s_quad, true);
    RShader_program_set(RTex__L.s_quad, r_program_Quad());
    RTex_quads(obj, RTex__L.s_quad, &q, 1);
}

void RTex_blend_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect)
{
    struct r_box b = {0};
    b.rect = rect;
    b.uv_rect = uv_rect;

    RShaderBox_tex_set(RTex__L.s_box, tex, false);
    RShader_blend_set(RTex__L.s_box, true);
    RShader_program_set(RTex__L.s_box, r_program_Box());
    RTex_boxes(obj, RTex__L.s_box, &b, 1);
}

void RTex_blend(oobj obj, oobj tex, float x, float y)
{
    vec4 rect = RTex_rect(tex, x, y);
    vec4 uv_rect = RTex_rect(tex, 0, 0);
    RTex_blend_rect(obj, tex, rect, uv_rect);
}

void RTex_blend_color_pose(oobj obj, oobj tex, mat4 pose, mat4 uv_pose, vec4 rgba, vec4 hsva)
{
    struct r_quad q = {0};
    q.pose = pose;
    q.uv_pose = uv_pose;
    q.fx = rgba;
    q.fy = hsva;

    RShaderQuad_tex_set(RTex__L.s_quad, tex, false);
    RShader_blend_set(RTex__L.s_quad, true);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_rgba_hsva());
    RTex_quads(obj, RTex__L.s_quad, &q, 1);
}

void RTex_blend_color_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect, vec4 rgba, vec4 hsva)
{
    mat4 pose = mat4_new(
        rect.width, 0, 0, 0,
        0, rect.height, 0, 0,
        0, 0, 1, 0,
        rect.x, rect.y, 0, 1
    );
    mat4 uv = mat4_new(
        uv_rect.width, 0, 0, 0,
        0, uv_rect.height, 0, 0,
        0, 0, 1, 0,
        uv_rect.x, uv_rect.y, 0, 1
    );
    RTex_blend_color_pose(obj, tex, pose, uv, rgba, hsva);
}

void RTex_blend_color(oobj obj, oobj tex, float x, float y, vec4 rgba, vec4 hsva)
{
    vec4 rect = RTex_rect(tex, x, y);
    vec4 uv_rect = RTex_rect(tex, 0, 0);
    RTex_blend_color_rect(obj, tex, rect, uv_rect, rgba, hsva);
}


void RTex_blend_sprite_color(oobj obj, oobj tex, float x, float y, vec2 sprite, ivec2 num_sprites, vec4 rgba, vec4 hsva)
{
    vec2 size = vec2_div_v(RTex_size(tex), vec2_cast_int(num_sprites.v));
    mat4 pose = mat4_new(
        size.x, 0, 0, 0,
        0, size.y, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    );
    mat4 uv = mat4_new(
        size.x, 0, 0, 0,
        0, size.y, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    struct r_quad q = {0};
    q.pose = pose;
    q.uv_pose = uv;
    q.sprite.xy = sprite;
    q.sprite.zw = size;
    q.fx = rgba;
    q.fy = hsva;

    RShaderQuad_tex_set(RTex__L.s_quad, tex, false);
    RShader_blend_set(RTex__L.s_quad, true);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_rgba_hsva());
    RTex_quads(obj, RTex__L.s_quad, &q, 1);
}


void RTex_blend_sprite(oobj obj, oobj tex, float x, float y, vec2 sprite, ivec2 num_sprites)
{
    RTex_blend_sprite_color(obj, tex, x, y, sprite, num_sprites, vec4_(1), vec4_(0));
}

void RTex_blend_indexed_rect(oobj obj, oobj tex, vec4 rect, vec4 uv_rect, oobj palette, vec2 palette_offset)
{
    struct r_box b = {0};
    b.rect = rect;
    b.uv_rect = uv_rect;
    b.fx = vec4_(palette_offset.x, palette_offset.y, 0, 1);

    RShaderBox_tex_set(RTex__L.s_box_indexed, tex, false);
    RShaderBoxIndexed_palette_set(RTex__L.s_box_indexed, palette);
    RShader_blend_set(RTex__L.s_box_indexed, true);
    RShader_program_set(RTex__L.s_box_indexed, r_program_BoxIndexed());
    RTex_boxes(obj, RTex__L.s_box_indexed, &b, 1);
}

void RTex_blend_indexed(oobj obj, oobj tex, float x, float y, oobj palette, vec2 palette_offset)
{
    vec4 rect = RTex_rect(tex, x, y);
    vec4 uv_rect = RTex_rect(tex, 0, 0);
    RTex_blend_indexed_rect(obj, tex, rect, uv_rect, palette, palette_offset);
}




RTex *RTex_resize(oobj obj, int cols, int rows, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    RTex *res = RTex_new_ex(obj, NULL, cols, rows, format, format);
    vec4 res_rect = RTex_rect(res, 0, 0);
    vec4 obj_rect = RTex_rect(obj, 0, 0);
    RTex_blit_rect(res, obj, res_rect, obj_rect);
    return res;
}


void RTex_merge_into(oobj obj, oobj tex,
                     oobj opt_mask_a, oobj opt_mask_b, vec4 rgba, oobj into, enum RTex_merge_mode mode)
{
    struct r_quad q = r_quad_new(m_2(RTex_size(obj)));
    q.fx = rgba;

    RShaderQuad_tex_set(RTex__L.s_quad_merge, tex, false);
    RShaderQuadMerge_tex_old_set(RTex__L.s_quad_merge, obj);
    RShaderQuadMerge_mask_a_set(RTex__L.s_quad_merge, o_or(opt_mask_a, r_tex_white()));
    RShaderQuadMerge_mask_b_set(RTex__L.s_quad_merge, o_or(opt_mask_b, r_tex_white()));
    oobj program;
    switch (mode) {
        default:
        case RTex_merge_mode_DEFAULT:
            program = r_program_QuadMerge();
            break;
        case RTex_merge_mode_ADDITVE:
            program = r_program_QuadMerge_additive();
            break;
        case RTex_merge_mode_SUBTRACTIVE:
            program = r_program_QuadMerge_subtractive();
            break;
    }
    RShader_program_set(RTex__L.s_quad_merge, program);
    RShader_blend_set(RTex__L.s_quad_merge, false);
    RTex_quads(into, RTex__L.s_quad_merge, &q, 1);
}
