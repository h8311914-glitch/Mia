#include "r/RCam.h"
#include "o/OObj_builder.h"
#include "m/mat/mat4.h"
#include "m/vec/ivec2.h"
#include "m/vec/vec2.h"
#include "r/RTex.h"


// new pose with a rotation
static mat4 pose_new_angle(float x, float y, float w, float h, float angle_rad)
{
    // mat4 has column major order
    return mat4_new(
        cosf(angle_rad) * w, sinf(angle_rad) * w, 0, 0,
        -sinf(angle_rad) * h, cosf(angle_rad) * h, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    );
}

static mat4 pose_new_3(vec2 lt, vec2 rt, vec2 lb)
{
    return mat4_new(
        rt.x - lt.x, rt.y - lt.y, 0, 0,
        lb.x - lt.x, lb.y - lt.y, 0, 0,
        0, 0, 1, 0,
        lt.x, lt.y, 0, 1);
}

//
// public
//


void RCam_matrices_init(struct RCam_matrices* self)
{
    self->p = mat4_eye();
    self->p_inv = mat4_eye();

    self->v = mat4_eye();
    self->v_inv = mat4_eye();

    self->vp = mat4_eye();
    self->v_p_inv = mat4_eye();
}

void RCam_matrices_update(struct RCam_matrices* self)
{
    self->p_inv = mat4_inv(self->p);
    self->v_inv = mat4_inv(self->v);
    self->vp = mat4_mul_mat(self->p, self->v_inv);
    self->v_p_inv = mat4_mul_mat(self->v, self->p_inv);
}


RCam* RCam_init(oobj obj, oobj parent, bool pixelperfect, vec2 min_units_size)
{
    RCam* self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, RCam_ID);

    self->pixelperfect_scale = self->pixelperfect_view = pixelperfect;
    self->fixed_scale = -1;
    self->min_units_size = min_units_size;
    self->proj.scale = 1;
    self->zoom = 1;

    RCam_matrices_init(&self->cam);

    // vfuncs
    self->v_update = RCam__v_update;

    return self;
}


void RCam__v_update(oobj obj, int back_cols, int back_rows)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;

    self->back_size = ivec2_(back_cols, back_rows);


    self->proj = r_proj_new(self->back_size, self->fixed_scale, self->min_units_size, self->pixelperfect_scale);
    self->cam.p = self->proj.cam;


    self->cam.v = pose_new_angle(m_2(self->pos), self->zoom, self->zoom, self->rotation);


    RCam_matrices_update(&self->cam);

    // update cam
    self->proj.cam = self->cam.vp;
}


vec2 RCam_pos_set(oobj obj, vec2 pos, bool update)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    if (self->pixelperfect_view) {
        pos = r_proj_round_real_pixel(&self->proj, m_2(pos));
    }
    self->pos = pos;
    if (update) {
        RCam_update(self);
    }
    return self->pos;
}

vec2 RCam_center(oobj obj)
{
    return mat4_mul_vec(*RCam_cam_inv(obj), vec4_(0.0, 0.0, 0.0, 1.0)).xy;
}

vec2 RCam_center_set(oobj obj, vec2 center_pos, bool cam_update)
{
    vec2 center = RCam_center(obj);
    vec2 center_to_pos = vec2_sub_v(RCam_pos(obj), center);
    vec2 pos = vec2_add_v(center_pos, center_to_pos);
    return RCam_pos_set(obj, pos, cam_update);
}

float RCam_zoom_set(oobj obj, float zoom, bool update)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    assert(zoom>0);
    if (self->pixelperfect_view) {
        if (zoom > 1.0) {
            zoom = m_round(zoom);
        }
        else {
            zoom = 1.0f / m_round(1.0f / zoom);
        }
    }
    self->zoom = zoom;
    if (update) {
        RCam_update(self);
    }
    return self->zoom;
}


float RCam_rotation_set(oobj obj, float rotation_rad, bool update)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    if (self->pixelperfect_view) {
        rotation_rad = m_round(rotation_rad * m_2_PI) * m_PI_2;
    }
    self->rotation = rotation_rad;
    if (update) {
        RCam_update(self);
    }
    return self->rotation;
}

vec4 RCam_rect(oobj obj, float x, float y)
{
    vec4 lt = {{-1, 1, 0, 1}};
    vec4 rt = {{1, 1, 0, 1}};
    vec4 lb= {{-1, -1, 0, 1}};
    vec4 rb = {{1, -1, 0, 1}};
    lt = mat4_mul_vec(*RCam_cam_inv(obj), lt);
    rt = mat4_mul_vec(*RCam_cam_inv(obj), rt);
    lb = mat4_mul_vec(*RCam_cam_inv(obj), lb);
    rb = mat4_mul_vec(*RCam_cam_inv(obj), rb);
    vec2 min = vec2_min_v(lt.xy, vec2_min_v(rt.xy, vec2_min_v(lb.xy, rb.xy)));
    vec2 max = vec2_max_v(lt.xy, vec2_max_v(rt.xy, vec2_max_v(lb.xy, rb.xy)));
    return vec4_(min.x+x, min.y+y, max.x-min.x, max.y-min.y);
}

mat4 RCam_pose(oobj obj, float x, float y)
{
    vec4 lt = {{-1, 1, 0, 1}};
    vec4 rt = {{1, 1, 0, 1}};
    vec4 lb= {{-1, -1, 0, 1}};
    lt = mat4_mul_vec(*RCam_cam_inv(obj), lt);
    rt = mat4_mul_vec(*RCam_cam_inv(obj), rt);
    lb = mat4_mul_vec(*RCam_cam_inv(obj), lb);
    mat4 pose = pose_new_3(lt.xy, rt.xy, lb.xy);
    pose.m30 += x;
    pose.m31 += y;
    return pose;
}

void RCam_apply_proj(oobj obj, oobj tex)
{
    *RTex_proj(tex) = RCam_proj(obj);
}

void RCam_apply_proj_p(oobj obj, oobj tex)
{
    *RTex_proj(tex) = RCam_proj_p(obj);
}

void RCam_fit(oobj obj, vec4 rect, bool crop, float opt_zoom_scale)
{
    vec2 zoom_wh = vec2_div_v(rect.zw, RCam_proj_size(obj));
    float zoom;
    if (crop) {
        zoom = o_min(zoom_wh.x, zoom_wh.y);
    } else {
        zoom = o_max(zoom_wh.x, zoom_wh.y);
    }
    
    if(opt_zoom_scale>0) {
        zoom *= opt_zoom_scale;
    }
    
    // needs an update, so RCam_center_set works fine
    RCam_zoom_set(obj, zoom, true);
    
    // center of canvas_size, using zoom
    vec2 cpos = vec2_add_scaled(rect.xy, rect.zw, 0.5);
    RCam_center_set(obj, cpos, true);
}

void RCam_zoom_at(oobj obj, vec2 zoom_pos, float zoom, const vec2 *opt_pos_set)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    
    vec4 display = mat4_mul_vec(self->cam.vp, vec4_(m_2(zoom_pos), 0, 1));
    
    // call update so cam vp is updated
    RCam_zoom_set(self, zoom, true);
    
    vec4 zp2 = mat4_mul_vec(self->cam.v_p_inv, display);
    
    vec2 diff = vec2_sub_v(zp2.xy, zoom_pos);
    
    vec2 pos = opt_pos_set? *opt_pos_set : self->pos;
    
    pos = vec2_sub_v(pos, diff);
    RCam_pos_set(self, pos, true);
}

