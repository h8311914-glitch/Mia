#ifdef MIA_SHADER_VERTEX

layout (location = 0) in vec4 in_rect;
layout (location = 1) in vec4 in_uv_rect;
layout (location = 2) in vec4 in_sprite;
layout (location = 3) in vec4 in_fx;

out vec2 v_tex_coord;
flat out vec4 v_fx;

uniform mat4 u_vp;

uniform vec2 u_tex_scale;

#include "$r/Box__common.glsl"


void main() {
    gl_Position = c_box_vertex(u_vp, in_rect);
    v_tex_coord = c_box_tex_coord(in_uv_rect, in_sprite, u_tex_scale);

    v_fx = in_fx;
}
#endif


#ifdef MIA_SHADER_FRAGMENT

in vec2 v_tex_coord;
flat in vec4 v_fx;

layout(location = 0) out vec4 f_rgba;

uniform sampler2D u_tex;

uniform vec4 u_fx;

void main() {
    vec4 rgba = texture(u_tex, v_tex_coord);

    rgba = rgba * v_fx * u_fx;

    f_rgba = rgba;
}

#endif
