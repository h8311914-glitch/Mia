#ifndef U_RECT_H
#define U_RECT_H

/**
 * @file rect.h
 *
 * Functions dealing with a vec4 rect (ltwh) in 2D space.
 * 
 * a rect is defined as vec4:
 * left
 * top
 * width
 * height
 */


#include "o/common.h"
#include "m/types/flt.h"
#include "m/types/int.h"
#include "m/sca/flt.h"
#include "m/vec/vec2.h"

/**
 * @param x left
 * @param y top
 * @param w width
 * @param h height
 * @return rect
 */
o_inline vec4 u_rect_new(float l, float t, float w, float h)
{
    return vec4_(l, t, w, h);
}

/**
 * @param l left
 * @param t top
 * @param w width
 * @param h height
 * @return left top relative rect
 * @note just like u_rect_new ...
 */
o_inline vec4 u_rect_new_lt(float l, float t, float w, float h)
{
    return u_rect_new(l, t, w, h);
}

/**
 * @param l left
 * @param b bottom
 * @param w width
 * @param h height
 * @return left bottom relative rect
 */
o_inline vec4 u_rect_new_lb(float l, float b, float w, float h)
{
    return u_rect_new(l, b - h, w, h);
}

/**
 * @param r left
 * @param t top
 * @param w width
 * @param h height
 * @return right top relative rect
 */
o_inline vec4 u_rect_new_rt(float r, float t, float w, float h)
{
    return u_rect_new(r - w, t, w, h);
}


/**
 * @param r right
 * @param b bottom
 * @param w width
 * @param h height
 * @return right bottom relative rect
 */
o_inline vec4 u_rect_new_rb(float r, float b, float w, float h)
{
    return u_rect_new(r - w, b - h, w, h);
}


/**
 * @param l left
 * @param t top
 * @param r right
 * @param b bottom
 * @return w h calced from r b
 */
o_inline vec4 u_rect_new_ltrb(float l, float t, float r, float b)
{
    return u_rect_new(l, t, r-l, b-t);
}



/**
 * @param cx center x
 * @param cy center y
 * @param w width
 * @param h height
 * @return center relative rect
 */
o_inline vec4 u_rect_new_center(float cx, float cy, float w, float h)
{
    return u_rect_new(cx - w/2.0f, cy-h/2.0f, w, h);
}


/**
 * @return a hidden rect with valid size
 */
o_inline vec4 u_rect_new_hidden(void)
{
    return u_rect_new(m_MAX, m_MAX, 1, 1);
}

/**
 * @param r rect
 * @return the left border
 */
o_inline float u_rect_left(vec4 r)
{
    return r.left;
}

/**
 * @param r rect
 * @return the right border
 */
o_inline float u_rect_right(vec4 r)
{
    return r.left + r.width;
}

/**
 * @param r rect
 * @return the top border
 */
o_inline float u_rect_top(vec4 r)
{
    return r.top;
}

/**
 * @param r rect
 * @return the bottom border
 */
o_inline float u_rect_bottom(vec4 r)
{
    return r.top + r.height;
}

/**
 * @param r rect
 * @return the width
 */
o_inline float u_rect_width(vec4 r)
{
    return r.width;
}

/**
 * @param r rect
 * @return the height
 */
o_inline float u_rect_height(vec4 r)
{
    return r.height;
}

/**
 * @param r rect
 * @return wh
 */
o_inline vec2 u_rect_size(vec4 r)
{
    return r.zw;
}

/**
 * @param r rect
 * @return center x
 */
o_inline float u_rect_center_x(vec4 r)
{
    return r.left + r.width/2.0f;
}

/**
 * @param r rect
 * @return center y
 */
o_inline float u_rect_center_y(vec4 r)
{
    return r.top + r.height/2.0f;
}

/**
 * @param r rect
 * @return center y
 */
o_inline vec2 u_rect_center(vec4 r)
{
    return vec2_(u_rect_center_x(r), u_rect_center_y(r));
}


/**
 * Sets the position to MAX to hide the rect (only if wh are not MAX...)
 * @param r rect reference to set
 */
o_inline void u_rect_hidden_set(vec4 *r)
{
    r->xy = vec2_(m_MAX, m_MAX);
}


/**
 * @param r rect reference to set
 * @param l left border
 */
o_inline void u_rect_left_set(vec4 *r, float l)
{
    r->left = l;
}

/**
 * @param r rect reference to set
 * @param right right border
 */
o_inline void u_rect_right_set(vec4 *r, float right)
{
    r->left = right - r->width;
}

/**
 * @param r rect reference to set
 * @param t top border
 */
o_inline void u_rect_top_set(vec4 *r, float t)
{
    r->top = t;
}


/**
 * @param r rect reference to set
 * @param b bottom border
 */
o_inline void u_rect_bottom_set(vec4 *r, float b)
{
    r->top = b - r->height;
}

/**
 * @param r rect reference to set
 * @param w width
 */
o_inline void u_rect_width_set(vec4 *r, float w)
{
    r->width = w;
}

/**
 * @param r rect reference to set
 * @param h width
 */
o_inline void u_rect_height_set(vec4 *r, float h)
{
    r->height = h;
}


/**
 * @param r rect reference to set
 * @param cx center_x
 */
o_inline void u_rect_center_x_set(vec4 *r, float cx)
{
    r->left = cx - r->width/2.0f;
}

/**
 * @param r rect reference to set
 * @param cy center_y
 */
o_inline void u_rect_center_y_set(vec4 *r, float cy)
{
    r->top = cy - r->height/2.0f;
}

/**
 * @param r rect reference to set
 * @param l left
 * @param t top
 */
o_inline void u_rect_lt_set(vec4 *r, float l, float t)
{
    u_rect_left_set(r, l);
    u_rect_top_set(r, t);
}

/**
 * @param r rect reference to set
 * @param l left
 * @param b bottom
 */
o_inline void u_rect_lb_set(vec4 *r, float l, float b)
{
    u_rect_left_set(r, l);
    u_rect_bottom_set(r, b);
}



/**
 * @param r rect reference to set
 * @param right right
 * @param t top
 */
o_inline void u_rect_rt_set(vec4 *r, float right, float t)
{
    u_rect_right_set(r, right);
    u_rect_top_set(r, t);
}

/**
 * @param r rect reference to set
 * @param right right
 * @param b bottom
 */
o_inline void u_rect_rb_set(vec4 *r, float right, float b)
{
    u_rect_right_set(r, right);
    u_rect_bottom_set(r, b);
}

/**
 * @param r rect reference to set
 * @param w width
 * @param h height
 */
o_inline void u_rect_size_set(vec4 *r, float w, float h)
{
    u_rect_width_set(r, w);
    u_rect_height_set(r, h);
}


/**
 * @param r rect reference to set
 * @param cx center_x
 * @param cy center_y
 */
o_inline void u_rect_center_set(vec4 *r, float cx, float cy)
{
    u_rect_center_x_set(r, cx);
    u_rect_center_y_set(r, cy);
}



/**
 * @param r rect
 * @return 4x4 pose matrix as with u/pose.h
 */
o_inline mat4 u_rect_as_pose(vec4 r)
{
    return (mat4) {
            {
                    r.width, 0, 0, 0,
                    0, r.height, 0, 0,
                    0, 0, 1, 0,
                    r.left, r.top, 0, 1
            }
    };
}

/**
 * @param r rect
 * @param pos xy position to check if its in the rect rect
 * @return true if pos is in p
 */
o_inline bool u_rect_contains(vec4 r, vec2 pos)
{
    float l = u_rect_left(r);
    float right = u_rect_right(r);
    float t = u_rect_top(r);
    float b = u_rect_bottom(r);

    return pos.left >= l && pos.left <= right && pos.top >= t && pos.top <= b;
}

/**
 * @param a, b: rects
 * @return "boolean and" of the aabb rects a and b (so rect that both contain)
 */
o_inline vec4 u_rect_and(vec4 a, vec4 b)
{
    vec2 rb_a = vec2_(a.left + a.width, a.top + a.height);
    vec2 rb_b = vec2_(b.left + b.width, b.top + b.height);
    vec2 lt = vec2_max_v(a.xy, b.xy);
    vec2 rb = vec2_min_v(rb_a, rb_b);
    vec4 res = u_rect_new_ltrb(lt.x, lt.y, rb.x, rb.y);
    res.zw = vec2_min(res.zw, 0);
    return res;
}

/**
 * @param a, b: rects
 * @return "boolean or" of the aabb rects a and b (so rect that contains both)
 */
o_inline vec4 u_rect_or(vec4 a, vec4 b)
{
    vec2 rb_a = vec2_(a.left + a.width, a.top + a.height);
    vec2 rb_b = vec2_(b.left + b.width, b.top + b.height);
    vec2 lt = vec2_min_v(a.xy, b.xy);
    vec2 rb = vec2_max_v(rb_a, rb_b);
    vec4 res = u_rect_new_ltrb(lt.x, lt.y, rb.x, rb.y);
    res.zw = vec2_min(res.zw, 0);
    return res;
}

#endif //U_RECT_H
