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
uniform sampler2D u_tex_old;
uniform sampler2D u_mask_a;
uniform sampler2D u_mask_b;

uniform vec4 u_fx;
uniform vec4 u_fy;
uniform vec4 u_fz;

void main() {
    vec4 rgba = texture(u_tex, v_tex_coord);
    vec4 rgba_old = texture(u_tex_old, v_tex_coord);
    vec4 mask_a = texture(u_mask_a, v_tex_coord);
    vec4 mask_b = texture(u_mask_b, v_tex_coord);

    float mask = mask_a.r * mask_b.r;

    // apply general coloring
    rgba *= v_fx * u_fx;

    // mix with mask
    rgba = mix(rgba_old, rgba, clamp(mask, 0.0, 1.0));

    // subtractive coloring
    rgba.rgb = min(rgba.rgb, rgba_old.rgb);

    f_rgba = rgba;
}

#endif
