#include "u/USplit.h"
#include "o/OObj_builder.h"
#include "a/app.h"
#include "a/AScene.h"
#include "a/AView.h"
#include "r/RCam.h"
#include "r/RTex.h"
#include "o/OList.h"

#undef O_LOG_LIB
#define O_LOG_LIB "u"
#include "o/log.h"


o_inline float even(USplit *self, float v)
{
    if(self->even_sizes) {
        return m_floor(v/2.0f) * 2.0f;
    }
    return v;
}


static void update(oobj child, oobj tex, ivec4 viewport)
{
    if (OObj_check(child, USplit)) {
        USplit_update_ex(child, tex, viewport);
    } else if (OObj_check(child, AView)) {
        AView_update(child, tex, viewport);
    } else {
        o_log_warn_s("USplit_update", "child invalid, should have been USplit or AView");
    }
}

static void split_render(oobj child, oobj tex)
{
    if (OObj_check(child, USplit)) {
        USplit_render(child, tex);
    } else if (OObj_check(child, AView)) {
        AView_render(child, tex);
    } else {
        o_log_warn_s("USplit_render", "child invalid, should have been USplit or AView");
    }
}


//
// public
//


USplit *USplit_init(oobj obj, oobj parent, oobj child_a, oobj child_b,
                    enum USplit_mode mode, float seperator, float spacing)
{
    USplit *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, USplit_ID);

    self->child_a = child_a;
    self->child_b = child_b;
    self->mode = mode;
    self->seperator = seperator;
    self->spacing = spacing;
    self->even_sizes = false;

    return self;
}


void USplit_update_ex(oobj obj, oobj tex, ivec4 viewport)
{
    OObj_assert(obj, USplit);
    USplit *self = obj;

    // in units
    vec4 ltwh = r_proj_viewport_to_rect(RTex_proj(tex), viewport);
    float l = ltwh.v0;
    float t = ltwh.v1;
    float w = ltwh.v2;
    float h = ltwh.v3;

    if (w <= 0 || h <= 0) {
        return;
    }

    // ltwh
    float a_left, a_top, a_width, a_height;
    float b_left, b_top, b_width, b_height;

    if(self->mode == USplit_H_LEFT) {
        // full height
        a_top = b_top = t;
        a_height = b_height = even(self, h);

        // a left until seperator
        a_left = l;
        if(self->seperator>0) {
            a_width = self->seperator;
        } else {
            // relative mode
            a_width = w*-self->seperator-self->spacing/2;
        }
        a_width = o_clamp(a_width, 0, w-self->spacing);
        a_width = even(self, a_width);

        // b behind a
        b_left = a_left + a_width + self->spacing;
        b_width = l + w - b_left;
        b_width = o_clamp(b_width, 0, w-self->spacing);
        float b_width_even = even(self, b_width);
        float even_diff = b_width - b_width_even;
        b_width = b_width_even;
        b_left += even_diff;
    } 
    
    else if(self->mode == USplit_H_RIGHT) {
        // full height
        a_top = b_top = t;
        a_height = b_height = even(self, h);

        // a left = width - seperator - spacing
        a_left = l;
        
        if(self->seperator>0) {
            a_width = w - self->seperator - self->spacing;
        } else {
            // relative mode
            a_width = w*(1+self->seperator) - self->spacing/2;
        }
        a_width = o_clamp(a_width, 0, w-self->spacing);
        a_width = even(self, a_width);

        // b behind a
        b_left = a_left + a_width + self->spacing;
        b_width = l + w - b_left;
        b_width = o_clamp(b_width, 0, w-self->spacing);
        float b_width_even = even(self, b_width);
        float even_diff = b_width - b_width_even;
        b_width = b_width_even;
        b_left += even_diff;
    }
    
    else if(self->mode == USplit_V_TOP) {
        // full width
        a_left = b_left = l;
        a_width = b_width = even(self, w);

        // a top until seperator
        a_top = t;
        if(self->seperator>0) {
            a_height = self->seperator;
        } else {
            // relative mode
            a_height = h*-self->seperator-self->spacing/2;
        }
        a_height = o_clamp(a_height, 0, h-self->spacing);
        a_height = even(self, a_height);

        // b behind a
        b_top = a_top + a_height + self->spacing;
        b_height = t + h - b_top;
        b_height = o_clamp(b_height, 0, h-self->spacing);
        float b_height_even = even(self, b_height);
        float even_diff = b_height - b_height_even;
        b_height = b_height_even;
        b_top += even_diff;
    }
    
    else {
        // full width
        a_left = b_left = l;
        a_width = b_width = even(self, w);

        // a top = height - seperator - spacing
        a_top = t;
        if(self->seperator>0) {
            a_height = h - self->seperator - self->spacing;
        } else {
            // relative mode
            a_height = h*(1+self->seperator) - self->spacing/2;
        }
        a_height = o_clamp(a_height, 0, h-self->spacing);
        a_height = even(self, a_height);

        // b behind a
        b_top = a_top + a_height + self->spacing;
        b_height = t + h - b_top;
        b_height = o_clamp(b_height, 0, h-self->spacing);
        float b_height_even = even(self, b_height);
        float even_diff = b_height - b_height_even;
        b_height = b_height_even;
        b_top += even_diff;
    }


    ivec4 vp_a = r_proj_rect_to_viewport(RTex_proj(tex), vec4_(a_left, a_top, a_width, a_height));
    ivec4 vp_b = r_proj_rect_to_viewport(RTex_proj(tex), vec4_(b_left, b_top, b_width, b_height));

    update(self->child_a, tex, vp_a);
    update(self->child_b, tex, vp_b);
}

void USplit_update(oobj obj, oobj tex)
{
    USplit_update_ex(obj, tex, RTex_viewport(tex));
}

void USplit_render(oobj obj, oobj tex)
{
    OObj_assert(obj, USplit);
    USplit *self = obj;

    split_render(self->child_a, tex);
    split_render(self->child_b, tex);
}

static void collect_views_r(oobj obj, oobj list)
{
    if(OObj_check(obj, USplit)) {
        USplit *split = obj;
        collect_views_r(split->child_a, list);
        collect_views_r(split->child_b, list);
    } else if (OObj_check(obj, AView)) {
        OList_push(list, obj);
    } else {
        o_log_warn_s("USplit_views", "child invalid, should have been USplit or AView");
    }
}

AView **USplit_views(oobj obj)
{
    oobj list = OList_new(obj, NULL, 0);
    collect_views_r(obj, list);
    AView **res = (AView**) OList_list(list);
    OList_move(list, obj);
    o_del(list);
    return res;
}
