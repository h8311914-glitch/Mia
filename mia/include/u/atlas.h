#ifndef U_ATLAS_H
#define U_ATLAS_H

/**
 * @file atlas.h
 *
 * an atlas helps in animating sprites (RObjQuad -> r_quad.uv) in setting their uv positions
 */

#include "m/flt.h"

/**
 * Wrap modes for a sprite if its out of bounds.
 * - ASSERT calls assert on the bounds and falls through REPEAT.
 * - REPEAT calls o_mod on the bounds (positive modulo)
 * - CLAMP calls o_clamp on the bounds
 */
enum u_atlas_wrap_mode {
    u_atlas_wrap_ASSERT,
    u_atlas_wrap_REPEAT,
    u_atlas_wrap_CLAMP,
    u_atlas_wrap_ENUM_MAX
};

/**
 * start modes for u_atlas_new_fill (Left, Right, Bottom, Top)
 */
enum u_atlas_start {
    u_atlas_start_LT,
    u_atlas_start_LB,
    u_atlas_start_RT,
    u_atlas_start_RB,
    u_atlas_start_ENUM_MAX
};

struct u_atlas {
    // array of ltwh rects for each sprite in the spritesheet
    vec4 *rects;
    int num;

    // defaults to ASSERT
    enum u_atlas_wrap_mode wrap_mode;
    
    // may be deleted
    oobj parent;
};

/**
 * Creates a new atlas with an pos array allocated on obj (see u_atlas_move, u_atlas_free).
 * pos array is cleared to 0
 * @param obj object to allocate on
 * @param num_sprites number of sprites
 * @return u_atlas object with pos allocated on obj
 * @note wrap_mode defaults to u_atlas_WRAP_ASSERT
 */
struct u_atlas u_atlas_new(oobj obj, int num_sprites);

/**
 * Creates a new atlas with an pos array allocated on obj (see u_atlas_move, u_atlas_free)
 * @param obj object to allocate on
 * @param size of each sprite
 * @param num_sprites number of sprites
 * @param pos_0 left top position of the first sprite
 * @param offset offset to the next sprite (values may be negative...)
 * @param row_major if true, use offset.x until block size is reached, then offset.y, else other way around
 * @param block if row_major: columns, else rows
 * @return u_atlas object with pos allocated on obj
 * @note wrap_mode defaults to u_atlas_WRAP_ASSERT.
 *       calls u_atlas_set_grid with the allocated pos array
 */
struct u_atlas u_atlas_new_grid(oobj obj, vec2 size, int num_sprites,
                                vec2 pos_0, vec2 offset, bool row_major, int block);

/**
 * Creates a new atlas with an pos array allocated on obj (see u_atlas_move, u_atlas_free).
 * "fill" is a special filled out grid for easier setup.
 * @param obj object to allocate on
 * @param size of the sprites
 * @param cols number of closed up together sprite columns
 * @param rows number of closed up together sprite rows
 * @param start for example: u_atlas_START_LT for left top
 * @param row_major if true, first traverses through columns (the full row)
 * @return u_atlas object with pos allocated on obj
 * @note wrap_mode defaults to u_atlas_WRAP_ASSERT.
 *       calls u_atlas_new_grid
 */
struct u_atlas u_atlas_new_fill(oobj obj, vec2 size, int cols, int rows, enum u_atlas_start start, bool row_major);

/**
 * Creates a new atlas with an pos array allocated on obj (see u_atlas_move, u_atlas_free).
 * "fill" is a special filled out grid for easier setup.
 * @param obj object to allocate on
 * @param tex RTex object to get the filled sprite size from (vec2_div_v(RTex_size(tex), vec2_(cols, rows))
 * @param cols number of closed up together sprite columns
 * @param rows number of closed up together sprite rows
 * @param start for example: u_atlas_START_LT for left top
 * @param row_major if true, first traverses through columns (the full row)
 * @return u_atlas object with pos allocated on obj
 * @note wrap_mode defaults to u_atlas_WRAP_ASSERT.
 *       calls u_atlas_new_grid
 */
struct u_atlas u_atlas_new_fill_tex(oobj obj, oobj tex, int cols, int rows, enum u_atlas_start start, bool row_major);


/**
 * moves the atlas pos array into another object
 * @param self to move
 * @param into the new object to move the list into
 * @note just calls o_move_res(obj, into, self.pos)
 */
void u_atlas_move(struct u_atlas *self, oobj into);

/**
 * frees the atlas pos array
 * @param self to free
 * @note just calls o_free(obj, self.pos)
 */
void u_atlas_free(struct u_atlas *self);


//
// struct functions
//

/**
 * @param self the sprite atlas
 * @param size for each sprite
 */
void u_atlas_set_size(struct u_atlas self, vec2 size);

/**
 * @param self the sprite atlas
 * @param pos_0 left top position of the first sprite
 * @param offset offset to the next sprite (values may be negative...)
 * @param row_major if true, use offset.x until block size is reached, then offset.y, else other way around
 * @param block if row_major: columns, else rows
 */
void u_atlas_set_grid(struct u_atlas self, vec2 pos_0, vec2 offset, bool row_major, int block);

/**
 * @param self the sprite atlas
 * @param sprite the sprite index
 * @return sprite wrapped with the wrap mode if out of bounds
 */
o_inline int u_atlas_wrap_sprite(struct u_atlas self, int sprite)
{
    switch (self.wrap_mode) {
        default:
        case u_atlas_wrap_ASSERT:
            assert(sprite>=0 && sprite<self.num && "invalid sprite");
            /* fall-through */
        case u_atlas_wrap_REPEAT:
            sprite = o_mod(sprite, self.num);
            break;
        case u_atlas_wrap_CLAMP:
            sprite = o_clamp(sprite, 0, self.num-1);
            break;
    }
    return sprite;
}

/**
 * @param self the sprite atlas
 * @param sprite sprite which may be wrapped if out of bounds
 * @return (uv) rect as ltwh
 */
o_inline vec4 u_atlas_rect(struct u_atlas self, int sprite)
{
    return self.rects[u_atlas_wrap_sprite(self, sprite)];
}

/**
 * @param self the sprite atlas
 * @param sprite sprite which may be wrapped if out of bounds
 * @return left top position
 */
o_inline vec2 u_atlas_pos(struct u_atlas self, int sprite)
{
    return u_atlas_rect(self, sprite).xy;
}

/**
 * @param self the sprite atlas
 * @param sprite sprite which may be wrapped if out of bounds
 * @return size of the sprite
 */
o_inline vec2 u_atlas_size(struct u_atlas self, int sprite)
{
    return u_atlas_rect(self, sprite).zw;
}

/**
 * @param self the sprite atlas
 * @param sprite sprite which may be wrapped if out of bounds
 * @return a full (uv) pose matrix for an axis aligned sprite
 */
o_inline mat4 u_atlas_pose(struct u_atlas self, int sprite)
{
    vec4 r = u_atlas_rect(self, sprite);
    // as u_pose matrices...
    return mat4_new(
            r.v2, 0, 0, 0,
            0, r.v3, 0, 0,
            0, 0, 1, 0,
            r.x, r.y, 0, 1
    );
}


/**
 * @param self the sprite atlas
 * @param in_out_uv reference to an existing uv matrix, to just update the new position
 * @param sprite sprite which may be wrapped if out of bounds
 */
o_inline void u_atlas_uv_update_pos(struct u_atlas self, mat4 *in_out_uv, int sprite)
{
    vec4 r = u_atlas_rect(self, sprite);
    // as u_pose matrices...
    in_out_uv->m30 = r.x;
    in_out_uv->m31 = r.y;
}

/**
 * Shifts all center positions with offset
 * @param self the sprite atlas to shift
 * @param offset to be added on each center pos
 */
o_inline void u_atlas_offset(struct u_atlas self, vec2 offset)
{
    for(int i=0; i<self.num; i++) {
        self.rects[i].xy = vec2_add_v(self.rects[i].xy, offset);
    }
}


#endif //U_ATLAS_H
