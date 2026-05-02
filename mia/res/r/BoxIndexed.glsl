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

layout (location = 0) out vec4 f_rgba;

uniform sampler2D u_tex;
uniform sampler2D u_palette;

uniform vec4 u_fx;

void main() {
    vec4 index_norm = texture(u_tex, v_tex_coord);

    // normalized to index
    // +0.1 to compensate rounding errors
    float index = floor(index_norm.r * 255.0 + 0.1);
    
    // +0.5 to get into the texel center
    vec2 pal_coord = vec2(index + v_fx.x + u_fx.x + 0.5,
                          v_fx.y + u_fx.y + 0.5);

    // to norm
    vec2 pal_size = vec2(textureSize(u_palette, 0));
    pal_coord /= pal_size;
   
    // get palette color
    vec4 rgba = texture(u_palette, pal_coord);

    // apply alpha
    rgba.a *= v_fx.a * u_fx.a;

    f_rgba = rgba;
}

#endif
