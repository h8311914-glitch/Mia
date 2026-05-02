#ifndef R_QUAD_H
#define R_QUAD_H

/**
 * @file quad.h
 *
 * Defines a render quadrilaterals
 */

#include "o/common.h"
#include "m/types/flt.h"


struct r_quad {
    mat4 pose;      // 3d pose for the rect position (see u/pose.h)
    mat4 uv_pose;   // 3d pose for the uv tex position (see u/pose.h)

    // uv.col[3].xy += floor(sprite.xy) * sprite.zw
    vec4 sprite;
    
    // general purpose like rgba shading
    vec4 fx;
    vec4 fy;
    vec4 fz;  
};

/**
 * Initializes the quad
 * @param w, h as the size for pose and uv (will get a min of 1 to be a valid size)
 * @return a new default initialized quad, positioned at top left
 * @note fx = vec4_(1); fy = fz = vec4_(0);
 *       sprite .zw = wh
 */
struct r_quad r_quad_new(float w, float h);


/**
 * Initializes the quad for a full sprite grid
 * @param full_size may br texture size.
 *        wh = full_size / sprites
 * @return a new sprite enabled initialized box, positioned at top left, set to sprite 0, 0
 * @note fx = vec4_(1); fy = fz = vec4_(0);
 */
o_inline struct r_quad r_quad_new_sprite(vec2 full_size, int sprite_cols, int sprite_rows)
{
    assert(sprite_cols>0 && sprite_rows>0);
    return r_quad_new(full_size.x/sprite_cols,
                     full_size.y/sprite_rows);
}


#endif //R_QUAD_H
