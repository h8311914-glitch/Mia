//
//// common
//////
// All components are in the range [0:1], including hue.
vec3 c_rgb2hsv_unsafe(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
// All components are in the range [0:1], including hue.
vec3 c_hsv2rgb_unsafe(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
// All components are in the range [0:1], including hue.
vec3 c_rgb2hsv(vec3 rgb)
{
    rgb = clamp(rgb, vec3(0.0), vec3(1.0));
    return c_rgb2hsv_unsafe(rgb);
}
// All components are in the range [0..1], including hue.
vec3 c_hsv2rgb(vec3 hsv)
{
    // hue already mod / fract
    hsv.yz = clamp(hsv.yz, vec2(0.0), vec2(1.0));
    return c_hsv2rgb_unsafe(hsv);
}
//
// rgba wrapper
//
// All components are in the range [0:1], including hue.
vec4 c_rgba2hsva_unsafe(vec4 rgba)
{
    return vec4(c_rgb2hsv_unsafe(rgba.rgb), rgba.a);
}
// All components are in the range [0:1], including hue.
vec4 c_hsva2rgba_unsafe(vec4 hsva)
{
    return vec4(c_hsv2rgb_unsafe(hsva.rgb), hsva.a);
}
// All components are in the range [0:1], including hue.
vec4 c_rgba2hsva(vec4 rgba)
{
    return vec4(c_rgb2hsv(rgba.rgb), rgba.a);
}
// All components are in the range [0:1], including hue.
vec4 c_hsva2rgba(vec4 hsva)
{
    return vec4(c_hsv2rgb(hsva.rgb), hsva.a);
}
//////
//// end common
//
