#ifndef M_MMAT_IMGPROC_H
#define M_MMAT_IMGPROC_H

/**
 * @file MMat_imgproc.h
 *
 * Object extension
 *
 * MMat_ namespace
 *
 * image processing tools
 *
 * @sa MMat_ex to import all MMat object extensions
 */

#include "MMat.h"

//
// Imgproc
//


void MMat_distance_transform_into(oobj obj, oobj into, bool full);
struct oobj_opt MMat_distance_transform_try(oobj obj, bool full);
/**
 * Creates a distance transform of the given MMat.
 * Uses the first channel ".r>0" for the distance
 * @param obj MMat object
 * @param full if true, also diagonal checks are done
 * @return MMat allocated on obj, as M_FORMAT_1_UR16
 */
o_inline MMat *MMat_distance_transform(oobj obj, bool full)
{
    MMat *self = MMat_distance_transform_try(obj, full).o;
    o_assume(self, "MMat allocation failed");
    return self;
}



#endif //M_MMAT_IMGPROC_H
