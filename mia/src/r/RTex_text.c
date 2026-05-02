#include "r/RTex_text.h"
#include "RTex_L.h"
#include "o/OArray.h"
#include "m/mat/mat4.h"
#include "r/program.h"
#include "r/tex.h"
#include "r/textlayout.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"

// shared from RTex.c module
extern struct RTex__L_t RTex__L;



o_inline mat4 rtex_rect2pose(vec4 rect)
{
    mat4 pose = (mat4){
            {
                rect.v2, 0, 0, 0,
                0, rect.v3, 0, 0,
                0, 0, 1, 0,
                rect.x, rect.y, 0, 1
            }
    };
    return pose;
}


vec2 RTex_text_ex(oobj obj, struct r_textlayout *layout, oobj font_tex, const char *text, mat4 pose, vec4 text_color)
{
    osize len = o_strlen(text);
    oobj boxes = OArray_new_dyn(r_root(), NULL, sizeof(struct r_box), 0, len);
    vec2 text_size = r_textlayout_set_boxes(layout, boxes, text, text_color);

    const struct r_proj *proj = RTex_proj(obj);

    // pose position on real pixel
    pose.m30 = m_round(pose.m30 * proj->scale) / proj->scale;
    pose.m31 = m_round(pose.m31 * proj->scale) / proj->scale;

    struct r_proj combined = *proj;
    combined.cam = mat4_mul_mat(proj->cam, pose);

    RShaderBox_tex_set(RTex__L.s_box, font_tex, false);
    RShader_blend_set(RTex__L.s_box, true);
    RShader_program_set(RTex__L.s_box, r_program_Box_rgba());
    RBuffer_update(RTex__L.b_box, o_at(boxes, 0), o_num(boxes));
    RShader_render_ex(RTex__L.s_box, RTex__L.b_box, obj, -1, &combined);

    o_del(boxes);
    return text_size;
}

vec2 RTex_text_pose(oobj obj, const char *text, mat4 pose, enum RTex_font font, vec4 text_color)
{
    struct r_textlayout layout;
    oobj tex;

    switch (font) {
        default:
        case RTex_font35:
            layout = r_textlayout_new_font35();
            tex = r_tex_font35();
            break;
        case RTex_font35_shadow:
            layout = r_textlayout_new_font35_shadow();
            tex = r_tex_font35_shadow();
            break;
        case RTex_font55:
            layout = r_textlayout_new_font55();
            tex = r_tex_font55();
            break;
        case RTex_font55_shadow:
            layout = r_textlayout_new_font55_shadow();
            tex = r_tex_font55_shadow();
            break;
        case RTex_font58:
            layout = r_textlayout_new_font58();
            tex = r_tex_font58();
            break;
        case RTex_font58_shadow:
            layout = r_textlayout_new_font58_shadow();
            tex = r_tex_font58_shadow();
            break;
    }
    layout.casing = r_textlayout_casing_UNCHANGED;

    return RTex_text_ex(obj, &layout, tex, text, pose, text_color);
}

vec2 RTex_text_rect(oobj obj, const char *text, vec4 rect, enum RTex_font font, vec4 text_color)
{
    return RTex_text_pose(obj, text, rtex_rect2pose(rect), font, text_color);
}
