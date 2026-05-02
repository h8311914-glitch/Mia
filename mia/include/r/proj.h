#ifndef R_PROJ_H
#define R_PROJ_H

/**
 * @file proj.h
 *
 * Projections used for render objects, created default for RTex or by an RCam for example.
 * Origin is at left top.
 * +x -> to the right
 * +y -> to the bottom
 */

#include "m/types/flt.h"
#include "m/types/int.h"
#include "m/sca/flt.h"

struct r_proj {
    mat4 cam;

    // viewport | texture size
    ivec2 viewport_size;
    // viewport_size_half, to quickly set for render object uniforms
    vec2 vpsh;
    // viewport_even_offset, to quickly set for render object uniforms (each either 0 or 0.5)
    vec2 vpeo;

    // scaled size == viewport_size / scale
    vec2 size;
    float scale;
};

/**
 * @param viewport_size size of the viewport or texture
 * @param fixed_scale: if >0, will be exactly this scale
 *                     else , will use min_units_size
 * @param min_units_size to calc the scale, use <= to ignore, both <=0 to set scale to 1.0f
 * @param pixelperfect true to use integer scaling
 * @return a projection for the viewport
 */
struct r_proj r_proj_new(ivec2 viewport_size, float fixed_scale, vec2 min_units_size, bool pixelperfect);


/**
 * @param proj reference to the projection matrices
 * @return unit size as integer which may be slightly more in the view to be an integer
 */
o_inline ivec2 r_proj_size_int(const struct r_proj *proj)
{
    return ivec2_(
        m_ceil(proj->size.x),
        m_ceil(proj->size.y)
    );
}

/**
 * @param proj reference to the projection matrices
 * @return center position (may be half a pixel, see r_proj_center_int)
 */
o_inline vec2 r_proj_center(const struct r_proj *proj)
{
    return vec2_(
        proj->size.x / 2.0f,
        proj->size.y / 2.0f
    );
}

/**
 * @param proj reference to the projection matrices
 * @return center position rounded as integer
 */
o_inline ivec2 r_proj_center_int(const struct r_proj *proj)
{
    return ivec2_(
        m_floor(proj->size.x / 2.0f),
        m_floor(proj->size.y / 2.0f)
    );
}

/**
 * @param proj reference to the projection matrices
 * @return vec2 version of r_proj_center_int
 */
o_inline vec2 r_proj_center_unit(const struct r_proj *proj)
{
    return vec2_(
        m_floor(proj->size.x / 2.0f),
        m_floor(proj->size.y / 2.0f)
    );
}

/**
 * @param proj reference to the projection matrices
 * @param rect as ltwh in projection camera
 * @return viewport in pixels
 */
o_inline ivec4 r_proj_rect_to_viewport(const struct r_proj *proj, vec4 rect)
{
    return ivec4_(
        m_round(rect.v0 * proj->scale),
        m_round(rect.v1 * proj->scale),
        m_round(rect.v2 * proj->scale),
        m_round(rect.v3 * proj->scale)
    );
}

/**
 * @param proj reference to the projection matrices
 * @param viewport rect in pixels
 * @return rect as ltwh in projection camera
 */
o_inline vec4 r_proj_viewport_to_rect(const struct r_proj *proj, ivec4 viewport)
{
    return vec4_(
        (float) viewport.v0 / proj->scale,
        (float) viewport.v1 / proj->scale,
        (float) viewport.v2 / proj->scale,
        (float) viewport.v3 / proj->scale
    );
}

/**
 * @param proj reference to the projection matrices
 * @param rect as ltwh in projection camera
 * @return display as relative rect for display_pose stuff (xy are centered, y is up, origin centered [-1:+1])
 */
o_inline vec4 r_proj_rect_to_display(const struct r_proj *proj, vec4 rect)
{
    vec2 rect_center = vec2_(
        rect.x + rect.v2 / 2.0f,
        rect.y + rect.v3 / 2.0f
    );
    return vec4_(
        -1.0f + 2.0f * rect_center.x / proj->size.x,
        +1.0f - 2.0f * rect_center.y / proj->size.y,
        rect.v2 / proj->size.x,
        rect.v3 / proj->size.y
    );
}

/**
 * @param proj reference to the projection matrices
 * @param display as rect for diplay_pose stuff (xy are centered, y is up, origin centered [-1:+1])
 * @return rect as ltwh in projection camera
 */
o_inline vec4 r_proj_display_to_rect(const struct r_proj *proj, vec4 display)
{
    vec4 rect = vec4_(
        0.5f * (1.0f + display.x) * proj->size.x,
        0.5f * (1.0f - display.y) * proj->size.y,
        display.v2 * proj->size.x,
        display.v3 * proj->size.y
    );
    // back to left top
    rect.x -= rect.v2 / 2.0f;
    rect.y -= rect.v3 / 2.0f;
    return rect;
}

/**
 * @param proj reference to the projection matrices
 * @param x position (left)
 * @param y position (top)
 * @return unit rect as ltwh containing the full projection on position xy (left top)
 */
o_inline vec4 r_proj_rect(const struct r_proj *proj, float x, float y)
{
    return (vec4){{x, y, proj->size.x, proj->size.y}};
}


/**
 * @param proj reference to the projection matrices
 * @param x position (left)
 * @param y position (top)
 * @return unit pose containing the full projection on position xy (left top)
 */
o_inline mat4 r_proj_pose(const struct r_proj *proj, float x, float y)
{
    // mat4 has column major order
    return (mat4){
        {
            proj->size.x, 0, 0, 0,
            0, proj->size.y, 0, 0,
            0, 0, 1, 0,
            x, y, 0, 1
        }
    };
}


/**
 * @param proj reference to the projection matrices
 * @return true if camera is in portrait mode (if back is the screen -> smartphone portrait mode)
 * @note according to the p matrix, independent to position, rotation and zoom
 */
o_inline bool r_proj_is_portrait(const struct r_proj *proj)
{
    return proj->size.y > proj->size.x;
}

/**
 * @param proj reference to the projection matrices
 * @return x, y rounded on a real pixel, used internally by updating the pos for example.
 */
o_inline vec2 r_proj_round_real_pixel(const struct r_proj *proj, float x, float y)
{
    x = m_round(x * proj->scale) / proj->scale;
    y = m_round(y * proj->scale) / proj->scale;
    return (vec2){{x, y}};
}


#endif //R_PROJ_H
