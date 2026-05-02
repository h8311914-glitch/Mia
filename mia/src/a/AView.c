#include "a/AView.h"
#include "o/OObj_builder.h"
#include "o/OArray.h"
#include "o/OEvent.h"
#include "r/RCam.h"
#include "r/RTex_blit.h"
#include "a/app.h"
#include "m/mat/mat4.h"
#include "m/vec/ivec4.h"
#include "m/vec/vec4.h"
#include "u/pose.h"

#undef O_LOG_LIB
#define O_LOG_LIB "a"

#define COLOR_INJECT "color"

// protected
void a__view_set(oobj opt_view);


static void view_color_update(oobj view, oobj tex, float dt)
{
    // noop
}

static void view_color_render(oobj view, oobj tex, float dt)
{
    oobj inject = OObj_find(view, OObj, COLOR_INJECT, 0).o;
    assert(inject);
    vec4 *color = o_user(inject);
    RTex_clear(tex, *color);
}

//
// public
//

AView *AView_init(oobj obj, oobj parent,
                  OObj__event_fn opt_setup,
                  AView__fn update, AView__fn render)
{
    AView *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, AView_ID);

    self->cam = RCam_new(self, true, vec2_(180));
    self->cam_units_auto = true;

    self->scale = 1.0f;

    self->scale_auto = false;
    self->scale_auto_max = m_MAX;

    self->render_auto = true;
    self->render_colored = false;
    self->render_colored_rgba = vec4_(1);
    self->render_colored_hsva = vec4_(0);

    self->time = 0;

    self->tex = NULL;
    self->own_tex = NULL;
    self->use_own_tex = false;

    // v func lists
    struct AView_layer layer = {
        opt_setup, update, render
    };
    self->layers = OArray_new(self, &layer, sizeof(struct AView_layer), 1);
    self->current_layer = -1;


    // v funcs
    self->v_pointer_display = AView__v_pointer_display;

    return self;
}

AView *AView_new_color(oobj parent, vec4 color)
{
    oobj view = AView_new(parent, NULL, view_color_update, view_color_render);
    oobj inject = OObj_new(view);
    OObj_name_set(inject, COLOR_INJECT);
    vec4 *user = o_user_new0(inject, *user, 1);
    *user = color;
    return view;
}


struct a_pointer AView__v_pointer_display(oobj obj, int idx, int history)
{
    return a_pointer_back(idx, history);
}


void AView_update_ex(oobj obj, oobj tex, ivec4 viewport, mat4 display_pose)
{
    OObj_assert(obj, AView);
    AView *self = obj;

    self->viewport = viewport;
    self->display_pose = display_pose;
    self->display_pose_inv = mat4_inv(self->display_pose);

    ivec4 tex_viewport = RTex_viewport(tex);
    struct r_proj tex_proj = *RTex_proj(tex);
    float tex_scale = tex_proj.scale;

    if (self->scale_auto) {
        self->scale = o_min(tex_scale, self->scale_auto_max);
    }

    vec4 rect = r_proj_viewport_to_rect(&tex_proj, self->viewport);

    int cols, rows;

    if (self->use_own_tex) {
        cols = (int) m_floor(rect.v2 * self->scale);
        rows = (int) m_floor(rect.v3 * self->scale);

        if (!self->own_tex
            || RTex_size_int(self->own_tex).x != cols
            || RTex_size_int(self->own_tex).y != rows) {
            o_del(self->own_tex);
            self->own_tex = RTex_new(self, NULL, cols, rows);
        }

        self->tex = self->own_tex;
    } else {
        cols = self->viewport.v2;
        rows = self->viewport.v3;
        self->tex = tex;
    }

    if (self->cam_units_auto) {
        vec2 min_units_size = rect.zw;
        RCam_min_units_size_set(self->cam, min_units_size);
    }
    RCam_update_ex(self->cam, cols, rows);


    // update projection and viewport
    RCam_apply_proj(self->cam, self->tex);
    if (!self->own_tex) {
        RTex_viewport_set(tex, self->viewport);
    }

    // set current AView in app (for pointers, etc.)
    oobj opt_prev_view = a_view_try().o;
    a__view_set(self);

    //
    // call setup + update with configs set
    //

    double dt = a_dt();
    self->time += dt;

    self->in_update = true;

    // back to front
    for (osize i = o_num(self->layers) - 1; i >= 0; i--) {
        struct AView_layer *layer = o_at(self->layers, i);
        self->current_layer = i;
        if (layer->opt_setup) {
            layer->opt_setup(self);
            layer->opt_setup = NULL;
        }
        layer->update(self, self->tex, (float) dt);
    }
    self->current_layer = -1;
    self->in_update = false;

    //
    // reset
    //
    a__view_set(opt_prev_view);
    if (!self->own_tex) {
        RTex_viewport_set(tex, tex_viewport);
        *RTex_proj(self->tex) = tex_proj;
    }
}

void AView_update(oobj obj, oobj tex, ivec4 viewport)
{
    mat4 super_pose;
    ivec4 super_viewport;
    oobj super_view = a_view_try().o;
    if (super_view) {
        super_pose = ((AView*) super_view)->display_pose;
        super_viewport = AView_viewport(super_view);
    } else {
        super_pose = mat4_eye();
        super_viewport = a_app_viewport_unsafe();
    }

    vec2 center = vec2_(viewport.v0 + viewport.v2*0.5f,
                        viewport.v1 + (float) viewport.v3*0.5f);

    vec2 super_center = vec2_(super_viewport.v0 + super_viewport.v2*0.5f,
                              super_viewport.v1 + (float) super_viewport.v3*0.5f);

    mat4 rel_pose = mat4_eye();
    rel_pose.m00 = (float) viewport.v2 / (float) super_viewport.v2;
    rel_pose.m11 = (float) viewport.v3 / (float) super_viewport.v3;

    // center offset
    rel_pose.m30 = +2.0f * (center.x - super_center.x) / (float) super_viewport.v2;
    rel_pose.m31 = -2.0f * (center.y - super_center.y) / (float) super_viewport.v3;

    mat4 display_pose = mat4_mul_mat(super_pose, rel_pose);
    AView_update_ex(obj, tex, viewport, display_pose);
}

void AView_render(oobj obj, oobj tex)
{
    OObj_assert(obj, AView);
    AView *self = obj;

    ivec4 tex_viewport = RTex_viewport(tex);
    struct r_proj tex_proj = *RTex_proj(tex);

    // update projection and viewport
    RCam_apply_proj(self->cam, self->tex);
    if (!self->own_tex) {
        RTex_viewport_set(tex, self->viewport);
    }

    // set current AView in app (for pointers, etc.)
    oobj opt_prev_view = a_view_try().o;
    a__view_set(self);

    //
    // call render with configs set
    //

    double dt = a_dt();

    self->in_render = true;

    // front to back
    for (osize i = 0; i < o_num(self->layers); i++) {
        struct AView_layer *layer = o_at(self->layers, i);
        self->current_layer = i;
        layer->render(self, self->tex, (float) dt);
    }
    self->current_layer = -1;
    self->in_render = false;

    //
    // reset
    //
    a__view_set(opt_prev_view);
    if (!self->own_tex) {
        RTex_viewport_set(tex, tex_viewport);
        *RTex_proj(self->tex) = tex_proj;
    }

    //
    // render own_tex to tex
    //
    if (self->render_auto && self->tex) {
        // render the view onto the scene
        AView_render_tex(self, tex);
    }
}

void AView_render_tex(oobj obj, oobj tex)
{
    OObj_assert(obj, AView);
    AView *self = obj;
    if (self->use_own_tex) {
        ivec4 tex_viewport = RTex_viewport(tex);
        struct r_proj tex_proj = *RTex_proj(tex);

        RTex_viewport_set(tex, self->viewport);
        // size of proj may differ, to enable different scalings
        *RTex_proj(tex) = r_proj_new(RTex_size_int(self->own_tex), -1, vec2_(-1), true);

        if (self->render_colored) {
            RTex_blend_color(tex, self->own_tex, 0, 0, self->render_colored_rgba, self->render_colored_hsva);
        } else {
            RTex_blend(tex, self->own_tex, 0, 0);
        }


        RTex_viewport_set(tex, tex_viewport);
        *RTex_proj(tex) = tex_proj;
    }
}

struct event_data {
    oobj view;
    struct AView_layer layer;
    bool back;
};

static void event_fn(oobj event)
{
    struct event_data *data = o_user(event);
    assert(data);
    if (data->back) {
        OArray_push(AView_layers(data->view), &data->layer);
    } else {
        OArray_push_front(AView_layers(data->view), &data->layer);
    }
}

void AView_cam_min_units_set(oobj obj, vec2 min_units_size)
{
    OObj_assert(obj, AView);
    AView *self = obj;
    self->cam_units_auto = false;
    RCam_min_units_size_set(self->cam, min_units_size);
    RCam_update(self->cam);
}

void AView_push_layer_deferred(oobj obj, struct AView_layer layer, bool back)
{
    OObj_assert(obj, AView);
    AView *self = obj;

    oobj event = OEvent_new(self, event_fn, NULL);
    struct event_data *data = o_new0(event, *data, 1);
    data->view = self;
    data->layer = layer;
    data->back = back;
    o_user_set(event, data);
    OEvent_post(event);
}


struct a_pointer AView_pointer_raw(oobj obj, int idx, int history)
{
    OObj_assert(obj, AView);
    AView *self = obj;

    // display pointer to raw [-1.0 : +1.0]
    struct a_pointer p = AView_pointer_display(obj, idx, history);
    p.pos = mat4_mul_vec(self->display_pose_inv, p.pos);

    // bounds check
    bool in_bounds = -1.0f <= p.pos.x && p.pos.x <= +1.0f && -1.0f <= p.pos.y && p.pos.y <= +1.0f;
    p.down &= in_bounds;
    p.prev_down &= in_bounds;

    return p;
}

struct a_pointer AView_pointer_p(oobj obj, int idx, int history)
{
    struct a_pointer p = AView_pointer_raw(obj, idx, history);
    p.pos = mat4_mul_vec(*RCam_cam_p_inv(AView_cam(obj)), p.pos);
    return p;
}

struct a_pointer AView_pointer(oobj obj, int idx, int history)
{
    struct a_pointer p = AView_pointer_raw(obj, idx, history);
    p.pos = mat4_mul_vec(*RCam_cam_inv(AView_cam(obj)), p.pos);
    return p;
}
