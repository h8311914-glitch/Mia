#include "r/RObjText.h"
#include "o/OObj_builder.h"
#include "o/OArray.h"
#include "o/img.h"
#include "m/vec/vec2.h"
#include "m/mat/mat4.h"
#include "r/RObjBox.h"
#include "r/tex.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"


//
// public
//


RObjText *RObjText_init(oobj obj, oobj parent, oobj tex, bool move_tex, struct r_textlayout layout)
{
    RObj *super = obj;
    RObjText *self = obj;
    o_clear(self, sizeof *self, 1);

    RObj_init(obj, parent, RObjText__v_update, RObjText__v_render);
    OObj_id_set(self, RObjText_ID);

    self->pose = mat4_eye();
    self->layout = layout;
    self->boxes_ro = RObjBox_new_tex_rgba(parent, 0, tex, move_tex, 1, 1);

    return self;
}




RObjText *RObjText_new_font35(oobj parent, const char *opt_text)
{
    oobj tex = r_tex_font35();
    RObjText *self = RObjText_new(parent, tex, false, r_textlayout_new_font35());
    if(opt_text) {
        RObjText_text_set(self, opt_text, R_WHITE);
    }
    return self;
}

RObjText *RObjText_new_font35_shadow(oobj parent, const char *opt_text, bvec4 *opt_shadow_color)
{
    oobj tex;
    bool move_tex;
    if (opt_shadow_color) {
        tex = r_textlayout_font35_shadow_tex(parent, opt_shadow_color);
        move_tex = true;
    } else {
        tex = r_tex_font35_shadow();
        move_tex = false;
    }
    RObjText *self = RObjText_new(parent, tex, move_tex, r_textlayout_new_font35_shadow());
    if(opt_text) {
        RObjText_text_set(self, opt_text, R_WHITE);
    }
    return self;
}

RObjText *RObjText_new_font55(oobj parent, const char *opt_text)
{
    oobj tex = r_tex_font55();
    RObjText *self = RObjText_new(parent, tex, false, r_textlayout_new_font55());
    if(opt_text) {
        RObjText_text_set(self, opt_text, R_WHITE);
    }
    return self;
}

RObjText *RObjText_new_font55_shadow(oobj parent, const char *opt_text, bvec4 *opt_shadow_color)
{
    oobj tex;
    bool move_tex;
    if (opt_shadow_color) {
        tex = r_textlayout_font55_shadow_tex(parent, opt_shadow_color);
        move_tex = true;
    } else {
        tex = r_tex_font55_shadow();
        move_tex = false;
    }
    RObjText *self = RObjText_new(parent, tex, move_tex, r_textlayout_new_font55_shadow());
    if(opt_text) {
        RObjText_text_set(self, opt_text, R_WHITE);
    }
    return self;
}


RObjText *RObjText_new_font58(oobj parent, const char *opt_text)
{
    oobj tex = r_tex_font58();
    RObjText *self = RObjText_new(parent, tex, false, r_textlayout_new_font58());
    if(opt_text) {
        RObjText_text_set(self, opt_text, R_WHITE);
    }
    return self;
}



RObjText *RObjText_new_font58_shadow(oobj parent, const char *opt_text, bvec4 *opt_shadow_color)
{
    oobj tex;
    bool move_tex;
    if (opt_shadow_color) {
        tex = r_textlayout_font58_shadow_tex(parent, opt_shadow_color);
        move_tex = true;
    } else {
        tex = r_tex_font58_shadow();
        move_tex = false;
    }
    RObjText *self = RObjText_new(parent, tex, move_tex, r_textlayout_new_font58_shadow());
    if(opt_text) {
        RObjText_text_set(self, opt_text, R_WHITE);
    }
    return self;
}

//
// virtual implementations:
//

void RObjText__v_update(oobj obj)
{
    OObj_assert(obj, RObjText);
    RObjText *self = obj;
    if (self->queue_update) {
        RObj_update(self->boxes_ro);
        self->queue_update = false;
    }
}

void RObjText__v_render(oobj obj, oobj tex, const struct r_proj *proj)
{
    OObj_assert(obj, RObjText);
    RObjText *self = obj;
    
    mat4 pose = self->pose;
    
    // pose position on real pixel
    pose.m30 = m_round(pose.m30*proj->scale) / proj->scale;
    pose.m31 = m_round(pose.m31*proj->scale) / proj->scale;

    struct r_proj combined = *proj;
    combined.cam = mat4_mul_mat(proj->cam, pose);

    RObj_render_ex(self->boxes_ro, tex, &combined, false);
}

//
// object functions:
//

void RObjText_uv_size_set(oobj obj, vec2 size)
{
    OObj_assert(obj, RObjText);
    RObjText *self = obj;
    osize boxes_num = OArray_num(self->boxes_ro);
    struct r_box *boxes_v = OArray_data(self->boxes_ro, struct r_box);
    for(int i=0; i < boxes_num; i++) {
        struct r_box *b = &boxes_v[i];
        b->rect.zw = size;
        b->uv_rect.zw = size;
    }
    self->queue_update = true;
}

vec2 RObjText_text_set(oobj obj, const char *text, vec4 text_color)
{
    OObj_assert(obj, RObjText);
    RObjText *self = obj;
    oobj array = RObjBox_boxes(self->boxes_ro);
    OArray_clear(array);
    vec2 size = r_textlayout_set_boxes(&self->layout, array, text, text_color);
    self->queue_update = true;
    return size;
}

void RObjText_color_set(oobj obj, vec4 text_color)
{
    OObj_assert(obj, RObjText);
    RObjText *self = obj;
    oobj array = RObjBox_boxes(self->boxes_ro);
    osize boxes_num = OArray_num(array);
    struct r_box *boxes_v = OArray_data(array, struct r_box);
    for(osize i=0; i<boxes_num; i++) {
        struct r_box *b = &boxes_v[i];
        b->fx = text_color;
    }
    self->queue_update = true;
}

