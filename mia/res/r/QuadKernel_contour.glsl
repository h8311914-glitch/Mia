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
uniform sampler2D u_kernel;

uniform vec4 u_fx;
uniform vec4 u_fy;
uniform vec4 u_fz;

uniform ivec2 u_kernel_offset;// offset from the kernel center
uniform vec4 u_kernel_color;
uniform vec4 u_kernel_mask;// should be normalized


void main() {
    ivec2 tex_size = textureSize(u_tex, 0);
    vec2 tex_texel = 1.0 / vec2(tex_size);
    
    ivec2 kernel_size = textureSize(u_kernel, 0);
    vec2 kernel_texel = 1.0 / vec2(kernel_size);

    ivec2 pos = ivec2(v_tex_coord * vec2(tex_size));

    ivec2 offset = pos - u_kernel_offset - kernel_size/2;

    float alpha = 0.0;

    // traverse through the kernel
    for (int y=0; y<kernel_size.y; y++) {
        for (int x=0; x<kernel_size.x; x++) {
            
            // kernel and texture positions
            ivec2 kernel_pos = ivec2(x, y);
            ivec2 tex_pos = offset + kernel_pos;
            tex_pos = clamp(tex_pos, ivec2(0), tex_size - ivec2(1));
            
            // get kernel
            float kernel = texture(u_kernel, (vec2(kernel_pos)+vec2(0.1)) * kernel_texel).r;
            
            // get color
            vec4 tex_color = texture(u_tex, (vec2(tex_pos)+vec2(0.1)) * tex_texel);

            // apply mask; for example a mask of
            //     vec4(0, 0, 0, 1) will only "select" non transparent pixels
            float masked = dot(tex_color, u_kernel_mask);


            // result color is max color in the kernel, where mask applies
            alpha = max(alpha, kernel * masked);
        }
    }

    vec4 rgba = u_kernel_color;
    rgba.a *= alpha;

    f_rgba = rgba;
}

#endif
