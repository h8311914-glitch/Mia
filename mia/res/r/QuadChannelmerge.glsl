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
flat out vec4 v_fx;
flat out vec4 v_fy;
flat out vec4 v_fz;

uniform mat4 u_vp;

uniform vec2 u_tex_scale;

#include "$r/Quad__common.glsl"

void main() {
    gl_Position = c_quad_vertex(u_vp, in_pose);
    v_tex_coord = c_quad_tex_coord(in_uv_pose, in_sprite, u_tex_scale);

    v_fx = in_fx;
    v_fy = in_fy;
    v_fz = in_fz;
}
#endif


#ifdef MIA_SHADER_FRAGMENT

in vec2 v_tex_coord;
flat in vec4 v_fx;
flat in vec4 v_fy;
flat in vec4 v_fz;

layout(location = 0) out vec4 f_rgba;

uniform sampler2D u_tex;
uniform sampler2D u_tex_g;
uniform sampler2D u_tex_b;
uniform sampler2D u_tex_a;

uniform vec4 u_fx;
uniform vec4 u_fy;
uniform vec4 u_fz;

void main() {
    vec4 r = texture(u_tex, v_tex_coord);
    vec4 g = texture(u_tex_g, v_tex_coord);
    vec4 b = texture(u_tex_b, v_tex_coord);
    vec4 a = texture(u_tex_a, v_tex_coord);
    vec4 rgba = vec4(r.x, g.x, b.x, a.x);
    f_rgba = rgba;
}

#endif
