//
//// common
//////
uniform int u_c_pixel_snap;
uniform float u_c_viewport_scale;
uniform float u_c_viewport_mirror_y;
uniform vec2 u_c_viewport_size_half;
uniform vec2 u_c_viewport_even_offset;
const vec4 c_quad_vertices[4] = vec4[](
vec4(0.0, 0.0, 0, 1),
vec4(0.0, 1.0, 0, 1),
vec4(1.0, 0.0, 0, 1),
vec4(1.0, 1.0, 0, 1)
);
// rounds the pose position to a pixel, adds a bias of 0.01 so that the real vertex round is explicit
mat4 c_quad_pose_round(mat4 pose)
{
    if(u_c_pixel_snap==0) {
        return pose;
    }
    pose[3][0] = (0.01 + round(pose[3][0] * u_c_viewport_scale)) / u_c_viewport_scale;
    pose[3][1] = (0.01 + round(pose[3][1] * u_c_viewport_scale)) / u_c_viewport_scale;
    return pose;
}
// basic pose to vertex transformation
vec4 c_quad_pose_vertex_transform(mat4 vp, mat4 pose)
{
    vec4 vertex = vec4(vp * pose * c_quad_vertices[gl_VertexID]);
    return vertex;
}
// a vertex to be exactly on a pixel (if viewport is odd, we need to address that with the "even_offset")
vec4 c_quad_pixel_snap(vec4 vertex)
{
    if(u_c_pixel_snap==0) {
        return vertex;
    }
    vertex.xy = (round(vertex.xy * u_c_viewport_size_half) - u_c_viewport_even_offset) / u_c_viewport_size_half;
    return vertex;
}
// combines basic transformation to generate a vertex from a pose on an exact pixel
vec4 c_quad_vertex(mat4 vp, mat4 pose)
{
    pose = c_quad_pose_round(pose);
    vec4 vertex = c_quad_pose_vertex_transform(vp, pose);
    vertex = c_quad_pixel_snap(vertex);
    vertex.y *= u_c_viewport_mirror_y;
    return vertex;
}
// basic pose to tex_coord transformation
vec2 c_quad_tex_coord(mat4 uv, vec4 sprite, vec2 tex_scale)
{
    uv[3].xy = uv[3].xy + floor(sprite.xy) * sprite.zw;
    vec2 tex_coord = vec4((uv * c_quad_vertices[gl_VertexID])).xy;
    tex_coord = tex_coord * tex_scale;
    return tex_coord;
}
//////
//// end common
//
