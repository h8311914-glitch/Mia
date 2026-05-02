#ifdef MIA_SHADER_VERTEX

layout(location = 0) in mat4 in_pose;
// uses location [0:3] (for each col)

layout(location = 4) in mat4 in_uv_pose;
// uses location [4:7] (for each col)

layout(location = 8) in vec4 in_sprite;

layout(location = 9) in vec4 in_fx;
layout(location = 10) in vec4 in_fy;
layout(location = 11) in vec4 in_fz;

out vec2 v_tex_coord;
flat out vec4 v_fs;
flat out vec4 v_fx;
flat out vec4 v_fy;
flat out vec4 v_fz;

uniform mat4 u_vp;

uniform vec2 u_tex_scale;

#include "$r/Quad__common.glsl"

void main() {
    gl_Position = c_quad_vertex(u_vp, in_pose);
    v_tex_coord = c_quad_tex_coord(in_uv_pose, vec4(0), u_tex_scale);

    v_fs = in_sprite;
    v_fx = in_fx;
    v_fy = in_fy;
    v_fz = in_fz;
}
#endif


#ifdef MIA_SHADER_FRAGMENT

in vec2 v_tex_coord;
flat in vec4 v_fs;
flat in vec4 v_fx;
flat in vec4 v_fy;
flat in vec4 v_fz;

layout(location = 0) out vec4 f_rgba;

uniform sampler2D u_tex;

uniform vec4 u_fx;
uniform vec4 u_fy;
uniform vec4 u_fz;

// x>0? val*x
// x<0? (1-val)*x // inv...
float invmix_float(float val, float x) {
    return x * (val - step(0.0, -x));
}

// sum of rgba channels, invmixed with xyzw
float invmix_vec4(vec4 rgba, vec4 xyzw) {
    return invmix_float(rgba.r, xyzw.x)
            + invmix_float(rgba.g, xyzw.y)
            + invmix_float(rgba.b, xyzw.z)
            + invmix_float(rgba.a, xyzw.w);
}

void main() {
    vec4 rgba_src = texture(u_tex, v_tex_coord);
    vec4 rgba_res;

    rgba_res.r = invmix_vec4(rgba_src, v_fs);
    rgba_res.g = invmix_vec4(rgba_src, v_fx);
    rgba_res.b = invmix_vec4(rgba_src, v_fy);
    rgba_res.a = invmix_vec4(rgba_src, v_fz);

    f_rgba = rgba_res;
}

#endif
