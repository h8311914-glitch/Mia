#ifndef R_BOX_H
#define R_BOX_H

/**
 * @file box.h
 *
 * Defines a render box which is axis aligned
 */

#include "o/common.h"
#include "m/types/flt.h"


struct r_box {
    // rect as ltwh
    union {
        vec4 rect;
        struct {
            float x, y, w, h;
        };
        struct {
            vec2 xy, wh;
        };
    };

    // texture lookup rect, also as ltwh
    union {
        vec4 uv_rect;
        struct {
            float u, v, uv_w, uv_h;
        };
    };

    // uv.xy += floor(sprite.xy) * sprite.zw
    vec4 sprite;
    
    // general purpose like rgba shading
    vec4 fx;
    
};

/**
 * Initializes the box
 * @param w, h size for the rect and uv (will get a min of 1 to be a valid size)
 * @return a new default initialized box, positioned at top left
 * @note fx = vec4_(1)
 *       sprite .zw = wh
 */
struct r_box r_box_new(float w, float h);

/**
 * Initializes the box for a full sprite grid
 * @param full_size may br texture size.
 *        wh = full_size / sprites
 * @return a new sprite enabled initialized box, positioned at top left, set to sprite 0, 0
 * @note fx = vec4_(1)
 */
o_inline struct r_box r_box_new_sprite(vec2 full_size, int sprite_cols, int sprite_rows)
{
    assert(sprite_cols>0 && sprite_rows>0);
    return r_box_new(full_size.x/sprite_cols,
                     full_size.y/sprite_rows);
}

#endif //R_BOX_H
