#ifndef MP_GRID_H
#define MP_GRID_H

/**
 * @file grid.h
 *
 * grid positions for equally sized items
 */

#include "m/types/int.h"
#include "m/types/flt.h"
#include "m/vec/vec2.h"

struct u_grid {
    // position of item 0 as lt
    vec2 pos;
    // may be negative, stride to next item
    vec2 stride;
    // maximal items in the grid
    int cols, rows;
    // order of items
    bool row_major;
};

/**
 * @param pos_lt outer grid left top position
 * @param size outer size
 * @param item_size size of a single item
 * @param spacing spacing between items
 * @param row_major order
 * @return cached grid data
 */
static struct u_grid u_grid_new(vec2 pos_lt, vec2 size, vec2 item_size, vec2 spacing, bool row_major)
{
    struct u_grid self = {0};
    self.stride = vec2_add_v(item_size, spacing);
    self.cols = (int) m_floor(size.x / self.stride.x);
    self.rows = (int) m_floor(size.y / self.stride.y);
    self.row_major = row_major;
    self.pos = pos_lt;
    return self;
}


/**
 * @param pos_lt outer grid left top position
 * @param size outer size
 * @param item_size size of a single item
 * @param spacing spacing between items
 * @param row_major order
 * @return cached grid data
 * @note just calls u_grid_new...
 */
o_inline struct u_grid u_grid_new_lt(vec2 pos_lt, vec2 size, vec2 item_size, vec2 spacing, bool row_major)
{
    return u_grid_new(pos_lt, size, item_size, spacing, row_major);
}

/**
 * @param pos_lb outer grid left bottom position
 * @param size outer size
 * @param item_size size of a single item
 * @param spacing spacing between items
 * @param row_major order
 * @return cached grid data
 */
static struct u_grid u_grid_new_lb(vec2 pos_lb, vec2 size, vec2 item_size, vec2 spacing, bool row_major)
{
    struct u_grid self = {0};
    self.stride = vec2_add_v(item_size, spacing);
    self.cols = (int) m_floor(size.x / self.stride.x);
    self.rows = (int) m_floor(size.y / self.stride.y);
    self.row_major = row_major;
    self.pos = vec2_(pos_lb.x, pos_lb.y-item_size.y);
    return self;
}

/**
 * @param pos_rt outer grid right top position
 * @param size outer size
 * @param item_size size of a single item
 * @param spacing spacing between items
 * @param row_major order
 * @return cached grid data
 */
static struct u_grid u_grid_new_rt(vec2 pos_rt, vec2 size, vec2 item_size, vec2 spacing, bool row_major)
{
    struct u_grid self = {0};
    self.stride = vec2_add_v(item_size, spacing);
    self.cols = (int) m_floor(size.x / self.stride.x);
    self.rows = (int) m_floor(size.y / self.stride.y);
    self.row_major = row_major;
    self.pos = vec2_(pos_rt.x-item_size.x, pos_rt.y);
    self.stride.x *= -1;
    self.stride.y *= -1;
    return self;
}




/**
 * @param pos_rb outer grid right bottom position (NOT center position of first item)
 * @param size outer size
 * @param item_size size of a single item
 * @param spacing spacing between items
 * @param row_major order
 * @return cached grid data
 */
static struct u_grid u_grid_new_rb(vec2 pos_rb, vec2 size, vec2 item_size, vec2 spacing, bool row_major)
{
    struct u_grid self = {0};
    self.stride = vec2_add_v(item_size, spacing);
    self.cols = (int) m_floor(size.x / self.stride.x);
    self.rows = (int) m_floor(size.y / self.stride.y);
    self.row_major = row_major;
    self.pos = vec2_(pos_rb.x-item_size.x, pos_rb.y-item_size.y);
    self.stride.x *= -1;
    return self;
}


//
// struct functions
//


/**
 * @return the maximal number of items in the grid
 */
o_inline int u_grid_num(struct u_grid self) {
    return self.cols * self.rows;
}

/**
 * @return the col and row of an item in the grid
 * @note ignores out of bounds
 */
o_inline ivec2 u_grid_position(struct u_grid self, int idx)
{
    ivec2 position;
    if(self.row_major) {
        position.x = idx%self.cols;
        position.y = idx/self.cols;
    } else {
        position.y = idx%self.rows;
        position.x = idx/self.rows;
    }
    return position;
}


/**
 * @return the center position of an item at a specific col and row
 * @note ignores out of bounds
 */
o_inline vec2 u_grid_at_position(struct u_grid self, int col, int row)
{
    vec2 pos = vec2_scale_v(self.stride, vec2_(col, row));
    pos = vec2_add_v(self.pos, pos);
    return pos;
}

/**
 * @return the center position of an item at a specific item index
 * @note ignores out of bounds
 */
o_inline vec2 u_grid_at(struct u_grid self, int idx)
{
    ivec2 position = u_grid_position(self, idx);
    return u_grid_at_position(self, m_2(position));
}

/**
 * @return the center position of an item at a specific item index
 * @note if out of grid bounds, "out_of_bounds" is returned instead (could be set to vec2_(m_MAX) or smth)
 */
o_inline vec2 u_grid_bounds(struct u_grid self, int idx, vec2 out_of_bounds)
{
    if(idx >= u_grid_num(self)) {
        return out_of_bounds;
    }
    return u_grid_at(self, idx);
}

#endif //MP_GRID_H
