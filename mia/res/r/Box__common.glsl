//
//// common
//////
uniform int u_c_pixel_snap;
uniform float u_c_viewport_scale;
uniform float u_c_viewport_mirror_y;
uniform vec2 u_c_viewport_size_half;
uniform vec2 u_c_viewport_even_offset;
const vec2 c_box_vertices[4] = vec2[](
vec2(0.0, 0.0),
vec2(0.0, 1.0),
vec2(1.0, 0.0),
vec2(1.0, 1.0)
);
// rounds rect pose position to a pixel, adds a bias of 0.01 so that the real vertex round is explicit
vec4 c_box_rect_round(vec4 rect)
{
    if(u_c_pixel_snap==0) {
        return rect;
    }
    rect[0] = (0.01 + round(rect[0] * u_c_viewport_scale)) / u_c_viewport_scale;
    rect[1] = (0.01 + round(rect[1] * u_c_viewport_scale)) / u_c_viewport_scale;
    return rect;
}
// basic rect to vertex transformation
vec4 c_box_vertex_transform(mat4 vp, vec4 rect)
{
    vec4 vertex = vec4(rect.x + rect.z * c_box_vertices[gl_VertexID].x,
    rect.y + rect.w * c_box_vertices[gl_VertexID].y,
    0.0, 1.0);
    vertex = vp * vertex;
    return vertex;
}
// a vertex to be exactly on a pixel (if viewport is odd, we need to address that with the "even_offset")
vec4 c_box_pixel_snap(vec4 vertex)
{
    if(u_c_pixel_snap==0) {
        return vertex;
    }
    vertex.xy = (round(vertex.xy * u_c_viewport_size_half) - u_c_viewport_even_offset) / u_c_viewport_size_half;
    return vertex;
}
vec4 c_box_vertex(mat4 vp, vec4 rect)
{
    rect = c_box_rect_round(rect);
    vec4 vertex = c_box_vertex_transform(vp, rect);
    vertex = c_box_pixel_snap(vertex);
    vertex.y *= u_c_viewport_mirror_y;
    return vertex;
}

// basic rect to tex_coord transformation
vec2 c_box_tex_coord(vec4 uv_rect, vec4 sprite, vec2 tex_scale)
{
    uv_rect.xy = uv_rect.xy + floor(sprite.xy) * sprite.zw;
    vec2 tex_coord = vec2(uv_rect.x + uv_rect.z * c_box_vertices[gl_VertexID].x,
    uv_rect.y + uv_rect.w * c_box_vertices[gl_VertexID].y);
    tex_coord = tex_coord * tex_scale;
    return tex_coord;
}
//////
//// end common
//
