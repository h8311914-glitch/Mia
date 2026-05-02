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
out vec2 v_position;
flat out vec4 v_fx;
flat out vec4 v_fy;
flat out vec4 v_fz;

uniform mat4 u_vp;

uniform vec2 u_tex_scale;

const vec4 center_vertices[4] = vec4[](
vec4(-1.0, -1.0, 0, 1),
vec4(-1.0, +1.0, 0, 1),
vec4(+1.0, -1.0, 0, 1),
vec4(+1.0, +1.0, 0, 1)
);

#include "$r/Quad__common.glsl"

void main() {
    gl_Position = c_quad_vertex(u_vp, in_pose);
    v_tex_coord = c_quad_tex_coord(in_uv_pose, in_sprite, u_tex_scale);
    v_position = center_vertices[gl_VertexID].xy;

    v_fx = in_fx;
    v_fy = in_fy;
    v_fz = in_fz;
}
#endif


#ifdef MIA_SHADER_FRAGMENT

in vec2 v_tex_coord;
in vec2 v_position;
flat in vec4 v_fx;
flat in vec4 v_fy;
flat in vec4 v_fz;

layout(location = 0) out vec4 f_rgba;
layout(location = 1) out vec4 f_mask;

uniform sampler2D u_tex;
uniform sampler2D u_pattern;

uniform vec2 u_pattern_size_inv;
uniform vec2 u_pattern_offset;

uniform vec4 u_fx;
uniform vec4 u_fy;
uniform vec4 u_fz;

#include "$r/Quad__common_frag.glsl"

void main() {
    vec4 rgba = texture(u_tex, v_tex_coord);

    // pattern tex coordination
    vec2 pattern_coord = (gl_FragCoord.xy + u_pattern_offset) * u_pattern_size_inv;
    vec4 pattern = texture(u_pattern, pattern_coord);

    //pattern.r = gl_FragCoord.x / textureSize(u_pattern, 0).x;
    //pattern.g = 0.0;
    //pattern.b = 0.0;
    //pattern.a = 1.0;

    // apply pattern
    rgba *= pattern;

    float dist_sqr = v_position.x * v_position.x + v_position.y * v_position.y;
    // dist_sqr max is 2.0 (edges), so recuding to [0:1]
    //dist_sqr *= 0.5;

    // calc opacity from hardness
    float hardness = v_fz.x * u_fz.x;
    float opacity = 1.0;
    if(dist_sqr < hardness) {
        opacity = mix(1.0, hardness, dist_sqr/hardness);
    } else if(hardness<1.0){
        opacity = mix(hardness, 0.0, min(1.0, (dist_sqr-hardness) / (1.0-hardness)));
    }

    // create mask
    float mask = rgba.a * opacity * v_fz.y * u_fz.y;

    // resulting alpha of the main rgba texture, should be 0 or 1.
    // alpha is copied to mask
    // dabs alpha is set by stuv
    rgba.a = step(0.0001, rgba.a);

    // coloring
    rgba = rgba * v_fx * u_fx;
    vec4 hsva = c_rgba2hsva(rgba);
    hsva = hsva + v_fy + u_fy;
    rgba = c_hsva2rgba(hsva);

    // results
    f_rgba = rgba;
    f_mask = vec4(1.0, 1.0, 1.0, mask);
}

#endif
