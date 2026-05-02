#ifndef M_DISCR_H
#define M_DISCR_H

/**
 * @file discr.h
 *
 * discretize primitives like 2d lines, curves, circles, rect borders into points
 *
 */

#include "o/common.h"
#include "m/types/flt.h"
#include "m/sca/flt.h"

//
// dash/dist floating point descretize functions
//

/**
 * Appends a single point to the points array ; just calls OArray_push(points_array, &pt)
 * @param points_array OArray of vec2
 * @note useful as starting point before calling m_discr_line for example
 */
void m_discr_point(oobj points_array, vec2 pt);

/**
 * Appends a line segment to the points array
 * @param points_array OArray of vec2
 * @param a start point (not included)
 * @param b end point (included)
 * @param dist distance between each sampled point, may be reduced to create equal density
 * @return number of sampled points
 * @note does >NOT< include the starting point
 */
int m_discr_line(oobj points_array, vec2 a, vec2 b, float dist);

/**
 * Appends an axis aligned rectangle to the points array
 * @param points_array OArray of vec2
 * @param a edge
 * @param b edge on the opposite site
 * @param dist distance between each sampled point, may be reduced to create equal density
 * @return number of sampled points
 * @note all edge points are included
 */
int m_discr_box(oobj points_array, vec2 a, vec2 b, float dist);

/**
 * Appends a circle bow to the points array
 * @param points_array OArray of vec2
 * @param center of the circle
 * @param radius of the circle
 * @param a_rad start angle in radiant (not included)
 * @param b_rad end angle in radiant (included)
 * @param dist distance between each sampled point, may be reduced to create equal density
 * @return number of sampled points
 * @note does >NOT< include the starting point
 */
int m_discr_circle_bow(oobj points_array, vec2 center, float radius, float a_rad, float b_rad, float dist);


/**
 * Appends a full circle to the points array
 * @param points_array OArray of vec2
 * @param center of the circle
 * @param radius of the circle
 * @param dist distance between each sampled point, may be reduced to create equal density
 * @return number of sampled points
 */
o_inline int m_discr_circle(oobj points_array, vec2 center, float radius, float dist)
{
    return m_discr_circle_bow(points_array, center, radius, 0, 2*m_PI, dist);
}

/**
 * Appends a quadratic Bezier curve to the points array.
 * @param points_array OArray of vec2
 * @param p0 First control point
 * @param p1 Second control point
 * @param p2 Third control point
 * @param dist Desired distance between sampled points
 * @return Number of sampled points
 * @note Does NOT include the starting point p0 by default.
 */
int m_discr_bezier(oobj points_array, vec2 p0, vec2 p1, vec2 p2, float dist);


//
// integer grid based descretize functions
// returned as vec2 array
// always rounds to floor
//


/**
 * Appends a single point to the points array.
 * just calls m_discr_grid_point(points_array, vec2_floor(pt));
 * @param points_array OArray of vec2
 * @note useful as starting point before calling m_discr_line for example
 */
void m_discr_grid_point(oobj points_array, vec2 pt);

/**
 * Appends a line segment to the points array.
 * Uses the integer grid and the 8 neighboors for the next point.
 * @param points_array OArray of vec2
 * @param a start point (not included)
 * @param b end point (included)
 * @return number of sampled points
 * @note does >NOT< include the starting point
 */
int m_discr_grid_line(oobj points_array, vec2 a, vec2 b);

/**
 * Appends an axis aligned rectangle to the points array.
 * Uses the integer grid and the 4 neighboors for the next point.
 * @param points_array OArray of vec2
 * @param a edge
 * @param b edge on the opposite site
 * @return number of sampled points
 * @note all edge points are included
 */
int m_discr_grid_box(oobj points_array, vec2 a, vec2 b);

/**
 * Appends a circle bow to the points array.
 * Uses the integer grid and the 8 neighboors for the next point.
 * @param points_array OArray of vec2
 * @param center of the circle
 * @param radius of the circle
 * @param a_rad start angle in radiant (not included)
 * @param b_rad end angle in radiant (included)
 * @return number of sampled points
 * @note does >NOT< include the starting point
 */
int m_discr_grid_circle_bow(oobj points_array, vec2 center, float radius, float a_rad, float b_rad);

/**
 * Appends a full circle to the points array.
 * Uses the integer grid and the 8 neighboors for the next point.
 * @param points_array OArray of vec2
 * @param center of the circle
 * @param radius of the circle
 * @return number of sampled points
 */
o_inline int m_discr_grid_circle(oobj points_array, vec2 center, float radius)
{
    return m_discr_grid_circle_bow(points_array, center, radius, 0, 2*m_PI);
}


#endif //M_DISCR_H
