#ifndef U_POSE_H
#define U_POSE_H

/**
 * @file pose.h
 *
 * Functions dealing with a mat4 (3D) pose in 2D space.
 * 
 *
 * Pose matrix (printed in column major order...):
 * Xx Xy Xz 0
 * Yx Yy Yz 0
 * Zx Zy Zz 0
 * Tx Ty Tz 1
 *
 * Pose in form of eye will be at left top position 0, 0, with a size of 1, 1
 *
 * u_pose_aa_* are axis aligned (angle=0)
 * w  0  0  0
 * 0  h  0  0
 * 0  0  1  0
 * x  y  0  1
 *
 * a rect is defined as vec4, see "u/rect.h"
 * left
 * top
 * width
 * height
 *
 * some functions have *_aa_* in their names
 * aa stand for axis aligned / not rotated
 * these functions are usually faster
 */


#include "o/common.h"
#include "m/types/flt.h"
#include "m/sca/flt.h"
#include "m/mat/mat4.h"

/**
 * @param x left
 * @param y top
 * @param w width
 * @param h height
 * @return axis aligned pose
 */
o_inline mat4 u_pose_new(float x, float y, float w, float h)
{
    // mat4 has column major order
    return mat4_new(
        w, 0, 0, 0,
        0, h, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    );
}

/**
 * @param rect as (left, top, width, height)
 * @return centered axis aligned pose
 */
o_inline mat4 u_pose_new_rect(vec4 rect)
{
    return u_pose_new(m_4(rect));
}

/**
 * @param x left
 * @param y top
 * @param w width
 * @param h height
 * @param angle_rad rotation in radians
 * @return rotated pose
 * @note rotates clock wise (cause +y is down...)
 *       and around the left top corner
 */
o_inline mat4 u_pose_new_angle(float x, float y, float w, float h, float angle_rad)
{
    // mat4 has column major order
    return mat4_new(
        +m_cos(angle_rad) * w, +m_sin(angle_rad) * w, 0, 0,
        -m_sin(angle_rad) * h, +m_cos(angle_rad) * h, 0, 0,
        0, 0, 1, 0,
        x, y, 0, 1
    );
}

// forward, see below
o_inline void u_pose_rotate_around(mat4 *p, float rel_x, float rel_y, float angle_rad);

/**
 * @param x left
 * @param y top
 * @param w width
 * @param h height
 * @param rel_x, rel_y relative (to w and h) center of rotation (so 0.5 for pose center)
 * @param angle_rad rotation in radians
 * @return rotated pose
 * @note rotates clock wise (cause +y is down...)
 *       and around the left top corner
 */
o_inline mat4 u_pose_new_angle_around(float x, float y, float w, float h, float rel_x, float rel_y, float angle_rad)
{
    mat4 pose = u_pose_new(x, y, w, h);
    u_pose_rotate_around(&pose, rel_x, rel_y, angle_rad);
    return pose;
}


/**
 * @param l left
 * @param t top
 * @param w width
 * @param h height
 * @return left top relative axis aligned pose
 * @note same as u_pose_new...
 */
o_inline mat4 u_pose_new_lt(float l, float t, float w, float h)
{
    return u_pose_new(l, t, w, h);
}

/**
 * @param l left
 * @param b bottom
 * @param w width
 * @param h height
 * @return left bottom relative axis aligned pose
 */
o_inline mat4 u_pose_new_lb(float l, float b, float w, float h)
{
    return u_pose_new(l, b - h, w, h);
}

/**
 * @param r left
 * @param t top
 * @param w width
 * @param h height
 * @return right top relative axis aligned pose
 */
o_inline mat4 u_pose_new_rt(float r, float t, float w, float h)
{
    return u_pose_new(r - w, t, w, h);
}


/**
 * @param r right
 * @param b bottom
 * @param w width
 * @param h height
 * @return right bottom relative axis aligned pose
 */
o_inline mat4 u_pose_new_rb(float r, float b, float w, float h)
{
    return u_pose_new(r - w, b - h, w, h);
}

/**
 * @param cx center x
 * @param cy center y
 * @param w width
 * @param h height
 * @return center relative pose
 */
o_inline mat4 u_pose_new_center(float cx, float cy, float w, float h)
{
    return u_pose_new(cx - w / 2.0f, cy - h / 2.0f, w, h);
}

/**
 * @param cx center x
 * @param cy center y
 * @param w width
 * @param h height
 * @param angle_rad rotation in radians
 * @return center relative pose
 */
o_inline mat4 u_pose_new_center_angle(float cx, float cy, float w, float h, float angle_rad)
{
    mat4 pose = u_pose_new_center(cx, cy, w, h);
    u_pose_rotate_around(&pose, 0.5f, 0.5f, angle_rad);
    return pose;
}


/**
 * @param lt left top corner
 * @param rt right top corner
 * @param lb left bottom corner
 * @return a pose from three corner points, which may be rotated (or sheered)
 */
o_inline mat4 u_pose_new_3(vec2 lt, vec2 rt, vec2 lb)
{
    return mat4_new(
        rt.x - lt.x, rt.y - lt.y, 0, 0,
        lb.x - lt.x, lb.y - lt.y, 0, 0,
        0, 0, 1, 0,
        lt.x, lt.y, 0, 1);
}


/**
 * Tries to fit a pose into 4 given edge points.
 * Creates not only a rotated or sheered pose, 
 *     but also a perspective distorted pose.
 * Trivially implemented iterative fitting function.
 * Using 4 points is nevertheless a bit over determinded,
 * so may not fit perfectly.
 * Can be called multiple times to reach the desired error.
 * @param p pose to refine, use (mat4) {0} to init (checks for m.m33!=1)
 * @param lt left top corner
 * @param rt right top corner
 * @param lb left bottom corner
 * @param rb right bottom corner
 * @param steps number of iterations, <=0 for default
 * @reruen remaining error
 * @sa u_pose_new_4_fit_task u_pose_new_4_fit
 */
double u_pose_new_4_fit_step(mat4 *p, vec2 lt, vec2 rt, vec2 lb, vec2 rb, int steps);

/**
 * Tries to fit a pose into 4 given edge points.
 * Creates not only a rotated or sheered pose, 
 *     but also a perspective distorted pose.
 * Trivially implemented iterative fitting function.
 * Using 4 points is nevertheless a bit over determinded,
 * so may not fit perfectly.
 * @parent to inherit from
 * @param p pose to fit, must not be initialized
 * @param lt left top corner
 * @param rt right top corner
 * @param lb left bottom corner
 * @param rb right bottom corner
 * @param opt_steps number of iterations each run, <=0 for default
 * @param opt_max_runs maximal runs, <=0 for default
 * @param opt_min_error_rate minimal error change to continue, <=0 for default
 * @return OTask object to run
 * @sa u_pose_new_4_fit_step u_pose_new_4_fit
 * @note dont forget to o_del the task
 */
oobj u_pose_new_4_fit_task(oobj parent, mat4 *p, vec2 lt, vec2 rt, vec2 lb, vec2 rb,
                           int opt_steps, int opt_max_runs, double opt_min_error_rate);

/**
 * Tries to fit a pose into 4 given edge points.
 * Creates not only a rotated or sheered pose, 
 *     but also a perspective distorted pose.
 * Trivially implemented iterative fitting function.
 * Using 4 points is nevertheless a bit over determinded,
 * so may not fit perfectly.
 * @param p pose to fit, must not be initialized
 * @param lt left top corner
 * @param rt right top corner
 * @param lb left bottom corner
 * @param rb right bottom corner
 * @param opt_steps number of iterations each run, <=0 for default
 * @param opt_max_runs maximal runs, <=0 for default
 * @param opt_min_error_rate minimal error change to continue, <=0 for default
 * @return a pose fit into 4 corners
 * @sa u_pose_new_4_fit_task u_pose_new_4_fit_step
 */
mat4 u_pose_new_4_fit(vec2 lt, vec2 rt, vec2 lb, vec2 rb,
                           int opt_steps, int opt_max_runs, double opt_min_error_rate);




/**
 * @return a hidden pose with valid size
 */
o_inline mat4 u_pose_new_hidden(void)
{
    return u_pose_new(m_MAX, m_MAX, 1, 1);
}

//
// pose functions
//

/**
 * @param p pose
 * @return the x left position
 */
o_inline float u_pose_x(mat4 p)
{
    return p.m30;
}

/**
 * @param p pose
 * @return the y top position
 */
o_inline float u_pose_y(mat4 p)
{
    return p.m31;
}

/**
 * @param p pose
 * @return the xy left top position
 */
o_inline vec2 u_pose_xy(mat4 p)
{
    return vec2_(p.m30, p.m31);
}

/**
 * @param p pose
 * @return the angle of the rotation in [rad]
 */
o_inline float u_pose_angle(mat4 p)
{
    return m_atan2(p.m01, p.m00);
}

/**
 * @param p pose
 * @return the width (always positive)
 */
o_inline float u_pose_width(mat4 p)
{
    return m_sqrt(m_pow(p.m00, 2) + m_pow(p.m01, 2));
}

/**
 * @param p pose
 * @return the height (always positive)
 */
o_inline float u_pose_height(mat4 p)
{
    return m_sqrt(m_pow(p.m10, 2) + m_pow(p.m11, 2));
}

/**
 * @param p pose
 * @return the size (always positive)
 */
o_inline vec2 u_pose_size(mat4 p)
{
    return vec2_(
        u_pose_width(p),
        u_pose_height(p)
    );
}

/**
 * @param p pose
 * @return the xy left top position
 * @sa u_pose_xy
 */
o_inline vec2 u_pose_lt(mat4 p)
{
    return vec2_(p.m30, p.m31);
}

/**
 * @param p pose
 * @return the xy right top position
 */
o_inline vec2 u_pose_rt(mat4 p)
{
    return vec2_(p.m30+p.m00, p.m31+p.m01);
}


/**
 * @param p pose
 * @return the xy left bottom position
 */
o_inline vec2 u_pose_lb(mat4 p)
{
    return vec2_(p.m30+p.m10, p.m31+p.m11);
}

/**
 * @param p pose
 * @return the xy right bottom position
 */
o_inline vec2 u_pose_rb(mat4 p)
{
    return vec2_(p.m30+p.m00+p.m10, p.m31+p.m01+p.m11);
}

/**
 * @param p axis aligned pose
 * @return the width (may be negative)
 * @note only valid for axis aligned poses
 */
o_inline float u_pose_aa_width(mat4 p)
{
    return p.m00;
}

/**
 * @param p axis aligned pose
 * @return the height (may be negative)
 * @note only valid for axis aligned poses
 */
o_inline float u_pose_aa_height(mat4 p)
{
    return p.m11;
}

/**
 * @param p axis aligned pose
 * @return the size (may be negative)
 * @note only valid for axis aligned poses
 */
o_inline vec2 u_pose_aa_size(mat4 p)
{
    return vec2_(p.m00, p.m11);
}

/**
 * @param p axis aligned pose
 * @return a rect as (left, top, width, height)
 * @note only valid for axis aligned poses
 */
o_inline vec4 u_pose_aa_rect(mat4 p)
{
    return vec4_(p.m30, p.m31, p.m00, p.m11);
}

/**
 * @param p axis aligned pose
 * @return the left border
 * @note only valid for axis aligned poses
 */
o_inline float u_pose_aa_left(mat4 p)
{
    return p.m30;
}

/**
 * @param p axis aligned pose
 * @return the right border
 * @note only valid for axis aligned poses
 */
o_inline float u_pose_aa_right(mat4 p)
{
    return p.m30 + p.m00;
}

/**
 * @param p axis aligned pose
 * @return the top border
 * @note only valid for axis aligned poses
 */
o_inline float u_pose_aa_top(mat4 p)
{
    return p.m31;
}

/**
 * @param p axis aligned pose
 * @return the bottom border
 * @note only valid for axis aligned poses
 */
o_inline float u_pose_aa_bottom(mat4 p)
{
    return p.m31 + p.m11;
}

/**
 * @param p axis aligned pose
 * @return the center in x
 * @note only valid for axis aligned poses
 */
o_inline float u_pose_aa_center_x(mat4 p)
{
    return p.m30 + p.m00 / 2.0f;
}

/**
 * @param p axis aligned pose
 * @return the center in y
 * @note only valid for axis aligned poses
 */
o_inline float u_pose_aa_center_y(mat4 p)
{
    return p.m31 + p.m11 / 2.0f;
}

/**
 * @param p axis aligned pose
 * @return the center in xy
 * @note only valid for axis aligned poses
 */
o_inline vec2 u_pose_aa_center(mat4 p)
{
    return vec2_(
        p.m30 + p.m00 / 2.0f,
        p.m31 + p.m11 / 2.0f
    );
}


/**
 * @param p pose reference to set
 * @param x left
 */
o_inline void u_pose_x_set(mat4 *p, float x)
{
    p->m30 = x;
}

/**
 * @param p pose reference to set
 * @param y top
 */
o_inline void u_pose_y_set(mat4 *p, float y)
{
    p->m31 = y;
}

/**
 * @param p pose reference to set
 * @param x left
 * @param y top
 */
o_inline void u_pose_xy_set(mat4 *p, float x, float y)
{
    u_pose_x_set(p, x);
    u_pose_y_set(p, y);
}

/**
 * Sets the position to MAX to hide the rect (only if wh are not MAX...)
 * @param p pose reference to set
 */
o_inline void u_pose_hidden_set(mat4 *p)
{
    u_pose_xy_set(p, m_MAX, m_MAX);
}

/**
 * @param p pose reference to set
 * @param w width
 * @param h height
 * @param angle_rad rotation in [rad]
 * @note rotates clock wise (cause +y is down...)
 *       and around the left top corner
 */
o_inline void u_pose_size_angle_set(mat4 *p, float w, float h, float angle_rad)
{
    p->m00 = +m_cos(angle_rad) * w;
    p->m01 = +m_sin(angle_rad) * w;
    p->m10 = -m_sin(angle_rad) * h;
    p->m11 = +m_cos(angle_rad) * h;
}

/**
 * @param p pose reference to set
 * @param w width
 * @note width must have been valid before (!=0)
 */
o_inline void u_pose_width_set(mat4 *p, float w)
{
    float f = w / u_pose_width(*p);
    p->m00 *= f;
    p->m01 *= f;
}

/**
 * @param p pose reference to set
 * @param h height
 * @note height must have been valid before (!=0)
 */
o_inline void u_pose_height_set(mat4 *p, float h)
{
    float f = h / u_pose_height(*p);
    p->m10 *= f;
    p->m11 *= f;
}


/**
 * @param p pose reference to set
 * @param w width
 * @param h height
 * @note size must have been valid before (!=0)
 */
o_inline void u_pose_size_set(mat4 *p, float w, float h)
{
    u_pose_width_set(p, w);
    u_pose_height_set(p, h);
}

/**
 * @param p pose reference to set
 * @param angle_rad rotation in [rad]
 * @note rotates clock wise (cause +y is down...)
 *       and around the left top corner
 */
o_inline void u_pose_angle_set(mat4 *p, float angle_rad)
{
    u_pose_size_angle_set(p, u_pose_width(*p), u_pose_height(*p), angle_rad);
}

/**
 * @param p pose reference to set
 * @param x left
 * @param y top
 * @param w width
 * @param h height
 * @param angle_rad rotation in [rad]
 * @note rotates clock wise (cause +y is down...)
 *       and around the left top corner
 */
o_inline void u_pose_set(mat4 *p, float x, float y, float w, float h, float angle_rad)
{
    u_pose_xy_set(p, x, y);
    u_pose_size_angle_set(p, w, h, angle_rad);
}

/**
 * @param p pose reference to set
 * @param rel_x, rel_y relative (to the pose size) rotation center from left top
 *                     For example 0.5 for a center rotation
 * @param angle_rad rotation in [rad]
 */
o_inline void u_pose_rotate_around(mat4 *p, float rel_x, float rel_y, float angle_rad)
{
    vec2 size = u_pose_size(*p);
    mat4 shift_a = u_pose_new(rel_x, rel_y, 1.0 / size.x, 1.0 / size.y);
    mat4 rotate = u_pose_new_angle(0, 0, 1, 1, angle_rad);
    mat4 shift_b = u_pose_new(-rel_x * size.x, -rel_y * size.y, size.x, size.y);
    *p = mat4_mul_mat(*p, shift_a);
    *p = mat4_mul_mat(*p, rotate);
    *p = mat4_mul_mat(*p, shift_b);
}

/**
 * @param p pose reference to set
 * @param x offset
 */
o_inline void u_pose_shift_x(mat4 *p, float x)
{
    p->m30 += x;
}

/**
 * @param p pose reference to set
 * @param y offset
 */
o_inline void u_pose_shift_y(mat4 *p, float y)
{
    p->m31 += y;
}

/**
 * @param p pose reference to set
 * @param x offset
 * @param y offset
 */
o_inline void u_pose_shift_xy(mat4 *p, float x, float y)
{
    u_pose_shift_x(p, x);
    u_pose_shift_y(p, y);
}

/**
 * @param p pose reference to set
 * @param angle_rad rotation offset in [rad]
 * @note rotates clock wise (cause +y is down...)
 *       and around the left top corner
 */
o_inline void u_pose_shift_angle(mat4 *p, float angle_rad)
{
    u_pose_angle_set(p, u_pose_angle(*p) + angle_rad);
}


/**
 * @param p pose reference to set
 * @param x offset
 * @param y offset
 * @param angle_rad rotation offset in [rad]
 */
o_inline void u_pose_shift(mat4 *p, float x, float y, float angle_rad)
{
    u_pose_shift_xy(p, x, y);
    u_pose_shift_angle(p, angle_rad);
}

/**
 * @param p axis aligned pose reference to set
 * @param l left border
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_left_set(mat4 *p, float l)
{
    p->m30 = l;
}

/**
 * @param p axis aligned pose reference to set
 * @param r right border
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_right_set(mat4 *p, float r)
{
    p->m30 = r - p->m00;
}

/**
 * @param p axis aligned pose reference to set
 * @param t top border
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_top_set(mat4 *p, float t)
{
    p->m31 = t;
}

/**
 * @param p axis aligned pose reference to set
 * @param b bottom border
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_bottom_set(mat4 *p, float b)
{
    p->m31 = b - p->m11;
}

/**
 * @param p axis aligned pose reference to set
 * @param w width
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_width_set(mat4 *p, float w)
{
    p->m00 = w;
}

/**
 * @param p axis aligned pose reference to set
 * @param h height
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_height_set(mat4 *p, float h)
{
    p->m11 = h;
}


/**
 * @param p axis aligned pose reference to set
 * @param cx center_x
 */
o_inline void u_pose_aa_center_x_set(mat4 *p, float cx)
{
    u_pose_aa_left_set(p, cx - u_pose_aa_width(*p) / 2.0f);
}

/**
 * @param p axis aligned pose reference to set
 * @param cy center_y
 */
o_inline void u_pose_aa_center_y_set(mat4 *p, float cy)
{
    u_pose_aa_top_set(p, cy - u_pose_aa_height(*p) / 2.0f);
}

/**
 * @param p axis aligned pose reference to set
 * @param w width
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_size_set(mat4 *p, float w, float h)
{
    u_pose_aa_width_set(p, w);
    u_pose_aa_height_set(p, h);
}


/**
 * @param p axis aligned pose reference to set
 * @param x left
 * @param y top
 * @param w width
 * @param h height
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_set(mat4 *p, float x, float y, float w, float h)
{
    u_pose_xy_set(p, x, y);
    u_pose_aa_size_set(p, w, h);
}

/**
 * @param p axis aligned pose reference to set
 * @param l left
 * @param t top
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_lt_set(mat4 *p, float l, float t)
{
    u_pose_aa_left_set(p, l);
    u_pose_aa_top_set(p, t);
}


/**
 * @param p axis aligned pose reference to set
 * @param l left
 * @param b bottom
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_lb_set(mat4 *p, float l, float b)
{
    u_pose_aa_left_set(p, l);
    u_pose_aa_bottom_set(p, b);
}

/**
 * @param p axis aligned pose reference to set
 * @param r right
 * @param t top
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_rt_set(mat4 *p, float r, float t)
{
    u_pose_aa_right_set(p, r);
    u_pose_aa_top_set(p, t);
}

/**
 * @param p axis aligned pose reference to set
 * @param r right
 * @param b bottom
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_rb_set(mat4 *p, float r, float b)
{
    u_pose_aa_right_set(p, r);
    u_pose_aa_bottom_set(p, b);
}


/**
 * @param p axis aligned pose reference to set
 * @param cx center_x
 * @param cy center_y
 */
o_inline void u_pose_aa_center_set(mat4 *p, float cx, float cy)
{
    u_pose_aa_center_x_set(p, cx);
    u_pose_aa_center_y_set(p, cy);
}

/**
 * @param p axis aligned pose reference to set
 * @param l left
 * @param b bottom
 * @param w width
 * @param h height
 * @note only valid for axis aligned poses
 */
o_inline void u_pose_aa_rect_set(mat4 *p, float l, float t, float w, float h)
{
    u_pose_aa_set(p, l, t, w, h);
}


/**
 * @param pos already transformed with pos = mat4_mul_vec(mat4_inv(pose), pos);
 * @return true if transformed pos is within the pose boundaries
 * @sa u_pose_contains
 */
o_inline bool u_pose_transformed_contains(vec4 pos)
{
    return pos.x >= 0.0f && pos.x <= 1.0f && pos.y >= 0.0f && pos.y <= 1.0f;
}


/**
 * @param p pose
 * @param pos position to check if its in the pose rect
 * @return true if pos is in p
 */
o_inline bool u_pose_contains(mat4 p, vec4 pos)
{
    pos = mat4_mul_vec(mat4_inv(p), pos);
    return u_pose_transformed_contains(pos);
}

/**
 * @param p axis aligned pose
 * @param pos position to check if its in the pose rect
 * @return true if pos is in p
 * @note only valid for axis aligned poses
 */
o_inline bool u_pose_aa_contains(mat4 p, vec2 pos)
{
    float l = u_pose_aa_left(p);
    float r = u_pose_aa_right(p);
    float t = u_pose_aa_top(p);
    float b = u_pose_aa_bottom(p);

    return pos.x >= l && pos.x <= r && pos.y >= t && pos.y <= b;
}

/**
 * @param p axis aligned pose
 * @param a start position of the line
 * @param b end position of the line
 * @return true if line ab intersects with p
 * @note only valid for axis aligned poses
 */
bool u_pose_aa_intersects_line(mat4 p, vec2 a, vec2 b);


#endif //U_POSE_H
