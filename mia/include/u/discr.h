#ifndef U_DISCR_H
#define U_DISCR_H

/**
 * @file discr.h
 *
 * discretize primitives like 2d lines, curves, circles, rect borders into points
 *
 * based on m/discr.h and expends it for r_box and r_quad
 */

#include "o/common.h"
#include "m/discr.h"
#include "r/box.h"
#include "r/quad.h"


/**
 * Creates an OArray of r_box's from an OArray of vec2's
 * @param points_array OArray of vec2
 * @param init besides xy, copies this into all created boxes
 * @return OArray of struct r_box
 */
oobj u_discr_as_boxes(oobj points_array, struct r_box init);

/**
 * Creates an OArray of r_quad's from an OArray of vec2's
 * @param points_array OArray of vec2
 * @param init besides xy, copies this into all created quad
 * @return OArray of struct r_quad
 */
oobj u_discr_as_quads(oobj points_array, struct r_quad init);

#endif //U_DISCR_H
