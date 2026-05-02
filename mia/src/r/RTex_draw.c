#include "r/RTex_draw.h"
#include "r/RTex_blit.h"
#include "r/RTex_imgproc.h"
#include "RTex_L.h"
#include "o/OArray.h"
#include "m/vec/ivec2.h"
#include "m/vec/vec2.h"
#include "m/discr.h"
#include "r/tex.h"
#include "r/program.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"

// shared from RTex.c module
extern struct RTex__L_t RTex__L;



void RTex_outline_into(oobj obj, ivec2 size, ivec2 offset, vec4 mask, vec4 color, oobj into)
{
    oobj kernel = RTex_new_kernel(obj, m_2(size), 1.0f, true, -1, R_FORMAT_R_F32);
    RTex_contour_into(obj, kernel, offset, mask, color, into);
    o_del(kernel);
}

RTex *RTex_collage(const oobj *srcs, int n, int cols, ivec2 margin, vec4 bg_color, ou32 format)
{
    assert(n>=1);
    assert(margin.x>=0 && margin.y>=0);
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(srcs[0]);
    }

    ivec2 grid_size = ivec2_(0);
    for (int i = 0; i < n; i++) {
        ivec2 size = RTex_size_int(srcs[i]);
        grid_size = ivec2_max_v(grid_size, size);
    }
    grid_size = ivec2_add_v(grid_size, ivec2_scale(margin, 2));

    int rows = (int) m_ceil((float) n / (float) cols);

    ivec2 full_size = ivec2_scale_v(grid_size, ivec2_(cols, rows));

    RTex *res = RTex_new_ex(srcs[0], NULL, m_2(full_size), format, format);
    RTex_clear_full(res, bg_color);

    for (int i = 0; i < n; i++) {
        int c = i % cols;
        int r = i / cols;

        vec2 pos = vec2_((grid_size.x-full_size.x)/2.0f, (full_size.y-grid_size.y)/2.0f);
        pos.x += c * grid_size.x;
        pos.y -= r * grid_size.y;

        RTex_blend(res, srcs[i], m_2(pos));
    }

    return res;
}



RTex *RTex_border(oobj obj, ivec4 lrbt, vec4 color, ou32 format)
{
    if (format == R_FORMAT_KEEP) {
        format = RTex_format(obj);
    }
    vec2 size = RTex_size(obj);
    size.x += lrbt.v0 + lrbt.v1;
    size.y += lrbt.v2 + lrbt.v3;

    RTex *res = RTex_new_ex(obj, NULL, m_2(size), format, format);
    RTex_clear_full(res, color);
    RTex_blit(res, obj, 0, 0);
    return res;
}




//
// draw functions
//


void RTex_draw_dabs(oobj obj, vec4 color, const vec2 *dabs, int num, oobj opt_brush)
{
    opt_brush = o_or(opt_brush, r_tex_white());

    oobj boxes_array = RTex__L.draw_boxes_array;
    OArray_resize(boxes_array, num);
    struct r_box *boxes = OArray_data(boxes_array, struct r_box);

    vec2 brush_size = RTex_size(opt_brush);
    vec2 offset = vec2_sub(brush_size, 0.1);
    offset = vec2_floor(vec2_div(offset, 2));
    for (int i = 0; i < num; i++) {
        boxes[i] = r_box_new(m_2(brush_size));
        boxes[i].fx = color;
        boxes[i].rect = vec4_(
            dabs[i].x - offset.x,
            dabs[i].y - offset.y,
            brush_size.x, brush_size.y
        );
    }

    RShaderBox_tex_set(RTex__L.s_box, opt_brush, false);
    RShader_blend_set(RTex__L.s_box, true);
    RShader_program_set(RTex__L.s_box, r_program_Box_rgba());
    RTex_boxes(obj, RTex__L.s_box, boxes, num);
}

void RTex_draw_line(oobj obj, vec4 color, vec2 pos_a, vec2 pos_b, oobj opt_brush)
{
    oobj dab_array = RTex__L.draw_dab_array;
    OArray_clear(dab_array);
    m_discr_grid_point(dab_array, pos_a);
    m_discr_grid_line(dab_array, pos_a, pos_b);
    RTex_draw_dabs(obj, color, OArray_data(dab_array, vec2), OArray_num(dab_array), opt_brush);
}

void RTex_draw_box(oobj obj, vec4 color, vec2 pos_a, vec2 pos_b, oobj opt_brush)
{
    oobj dab_array = RTex__L.draw_dab_array;
    OArray_clear(dab_array);
    m_discr_grid_box(dab_array, pos_a, pos_b);
    RTex_draw_dabs(obj, color, OArray_data(dab_array, vec2), OArray_num(dab_array), opt_brush);
}

void RTex_draw_circle(oobj obj, vec4 color, vec2 center, float radius, oobj opt_brush)
{
    oobj dab_array = RTex__L.draw_dab_array;
    OArray_clear(dab_array);
    m_discr_grid_circle(dab_array, center, radius);
    RTex_draw_dabs(obj, color, OArray_data(dab_array, vec2), OArray_num(dab_array), opt_brush);
}

void RTex_draw_circle_bow(oobj obj, vec4 color, vec2 center, float radius, float a_rad, float b_rad, oobj opt_brush)
{
    oobj dab_array = RTex__L.draw_dab_array;
    OArray_clear(dab_array);
    m_discr_grid_circle_bow(dab_array, center, radius, a_rad, a_rad);
    m_discr_grid_circle_bow(dab_array, center, radius, a_rad, b_rad);
    RTex_draw_dabs(obj, color, OArray_data(dab_array, vec2), OArray_num(dab_array), opt_brush);
}
