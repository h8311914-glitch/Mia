#ifndef M_MMAT_VIEW_H
#define M_MMAT_VIEW_H

/**
 * @file MMat_view.h
 *
 * Object extension
 * 
 * MMat_ namespace
 *
 * MMat windows and views like rotation
 *
 * @sa MMat_ex to import all MMat object extensions
 */

#include "MMat.h"


//
// Windows, rotate and mirror as views
//

/**
 * @param obj MMat object
 * @param data begin of data segment, somewhere in the data memory of obj, or NULL for MMat_data(obj)
 * @param size window size, or ==0 for MMat_size(obj)
 * @param stride window stride, or ==0 for MMat_stride(obj)
 * @return new MMat view on obj for that window
 */
MMat *MMat_window(oobj obj, void *data, m_ivec2 size, m_ivec2 stride);


/**
 * @param obj MMat object
 * @param rect ltwh rect in obj coords as sub window, using MMat_stride(obj).
 *             if .width or .height <= 0, uses the full rect instead.
 * @param clamp if true, clamped into the MMat oobj, else asserts (and clamps...)
 * @return new MMat view on obj for that window rect
 * @note asserts rect in in MMat obj bounds if not clamped
 */
MMat *MMat_window_rect(oobj obj, m_ivec4 rect, bool clamp);

/**
 * @param obj MMat object
 * @return MMat view on obj, but transposed (cols and rows swapped, so mirrored diagonally (lt <> rb))
 *         Use MMat_owning_make or MMat_clone to get a copy of the data.
 * @note just swapps col and row stride internally
 */
MMat *MMat_transpose(oobj obj);


/**
 * @param obj MMat object
 * @return MMat view on obj, but mirrored along a vertical axis.
 *         So cols left and right are swapped
 *         Use MMat_owning_make or MMat_clone to get a copy of the data.
 * @note just changes col stride and data internally
 */
MMat *MMat_mirror_v(oobj obj);

/**
 * @param obj MMat object
 * @return MMat view on obj, but mirrored along a horizontal axis.
 *         So rows top and bottom are swapped
 *         Use MMat_owning_make or MMat_clone to get a copy of the data.
 * @note just changes row stride and data internally
 */
MMat *MMat_mirror_h(oobj obj);


/**
 * @param obj MMat object
 * @return MMat view on obj, but rotated 90 degrees left (ccw)
 *         Use MMat_owning_make or MMat_clone to get a copy of the data.
 * @note just changes strides and data internally
 */
MMat *MMat_rotate_left(oobj obj);

/**
 * @param obj MMat object
 * @return MMat view on obj, but rotated 90 degrees right (cw)
 *         Use MMat_owning_make or MMat_clone to get a copy of the data.
 * @note just changes strides and data internally
 */
MMat *MMat_rotate_right(oobj obj);

/**
 * @param obj MMat object
 * @return MMat view on obj, but rotated 180 degrees
 *         Use MMat_owning_make or MMat_clone to get a copy of the data.
 * @note just changes strides and data internally
 */
MMat *MMat_rotate_180(oobj obj);


#endif //M_MMAT_VIEW_H
