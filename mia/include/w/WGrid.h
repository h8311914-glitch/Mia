#ifndef W_WGRID_H
#define W_WGRID_H

/**
 * @file WGrid.h
 *
 * Object (derives WObj)
 *
 * Widget that renders children in a grid.
 * Each col and row may have a fixed size.
 * Optionally as minimal fixed size with maximal previous child size.
 * And optionally with a weighted min_size, so cells may grow using the min_size room left.
 * The children have alignments for there cells (start, center, end; for H and V).
 * A child can render outside the cell and its grid, if to large.
 *
 * To set the cell for a child, set the child WObj_option to a dumped json ivec4
 * with the key of WGrid_cell_KEY: "[0, 0, 1, 1]" for first col, first row, expands to a 1x1 cell
 * 
 * Each child of WGrid may have its own WGrid_align_mode.
 * Default, see WGrid_align_h|v.
 * For a specific child, set the child WObj_option to one of the WGrid_align_VALUES
 * with the key of WGrid_align_KEY_H|V
 * for example:
 * ```c
 *     WObj_option_set(child, WGrid_align_KEY_H, WGrid_align_VALUE_CENTER); 
       WObj_option_set(child, WGrid_align_KEY_V, WGrid_align_VALUE_CENTER);   
 * ```
 *
 * @note the row and col weights uses a min_size cache to estimate the available room.
 *       It auto resets if (and layout is *_WEIGHTS):
 *           - min_size <= 0 is passed on update
 *           - children num changed
 *           - WGrid_weights_cached_min_size_clear is called
 */

#include "WObj.h"

/** object id */
#define WGrid_ID WObj_ID "Grid."


#define WGrid_cell_KEY "WGrid_cell"


// START -> left and top
// END -> right and bottom
enum WGrid_align_mode {
    WGrid_align_START,
    WGrid_align_CENTER,
    WGrid_align_END,
    WGrid_align_FIT,
    WGrid_align_NUM_MODES
};

#define WGrid_align_KEY_H "WGrid_align_h"
#define WGrid_align_KEY_V "WGrid_align_v"

static const char *WGrid_align_VALUES[] = {
        "start",
        "center",
        "end",
        "fit"
};

#define WGrid_align_VALUE_START   (WGrid_align_VALUES[0])
#define WGrid_align_VALUE_CENTER  (WGrid_align_VALUES[1])
#define WGrid_align_VALUE_END     (WGrid_align_VALUES[2])
#define WGrid_align_VALUE_FIT    (WGrid_align_VALUES[3])

/**
 * Helper function to get the cell key option.
 * Calls for example WObj_option_set(obj_child, WGrid_cell_KEY, "[0, 0, 1, 1]");
 * @param obj_child WObj child of WGrid
 * @return cell as [col, row, extend_w, extend_h], or the default [0, 0, 1, 1] if not set or invalid
 */
ivec4 WGrid_child_cell(oobj obj_child);

/**
 * Helper function to set the cell key option.
 * Calls for example WObj_option_set(obj_child, WGrid_cell_KEY, "[0, 0, 1, 1]");
 * @param obj_child WObj child of WGrid
 * @return cell as [col, row, extend_w, extend_h]
 */
ivec4 WGrid_child_cell_set(oobj obj_child, ivec4 cell);



typedef struct {
    WObj super;

    float *col_sizes;
    float *row_sizes;

    // of last update
    float *col_used_sizes;
    float *row_used_sizes;

    // weights for min size, <=0 to ignore (default)
    float *col_weights;
    float *row_weights;

    int cols, rows;
    vec2 spacing;

    // defaults
    enum WGrid_align_mode align_h, align_v;
} WGrid;


/**
 * Initializes the object.
 * @param obj WGrid object
 * @param parent to inherit from
 * @param cols, rows size of the grid
 * @param cell_size size for each cell (may be changed for each row and col)
 *                  pass <0 (-1) to auto size the cols, rows (uses col|row_child_sizes)
 * @return obj casted as WGrid
 */
WGrid *WGrid_init(oobj obj, oobj parent, int cols, int rows, vec2 cell_size);


/**
 * Creates a new WGrid object
 * @param parent to inherit from
 * @param cols, rows size of the grid
 * @param cell_size size for each cell (may be changed for each row and col)
 *                  pass <0 (-1) to auto size the cols, rows (uses col|row_child_sizes)
 * @return The new object
 */
o_inline WGrid *WGrid_new(oobj parent, int cols, int rows, vec2 cell_size)
{
    OObj_DECL_IMPL_NEW(WGrid, parent, cols, rows, cell_size);
}

//
// virtual implementations:
//

/**
 * Virtual implementation that updates the positions of the children.
 * Calls WObj_update on all children and places them into the grid.
 */
vec2 WGrid__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);

//
// object functions:
//

/**
 * @param obj WGrid object
 * @return grid columns
 */
OObj_DECL_GET(WGrid, int, cols)

/**
 * @param obj WGrid object
 * @return grid rows
 */
OObj_DECL_GET(WGrid, int, rows)

/**
 * @param obj WGrid object
 * @return current spacing between each col and row
 */
OObj_DECL_GETSET(WGrid, vec2, spacing)

/**
 * @param obj WGrid object
 * @return mutable size array with a width for each column
 *         pass <0 (-1) to auto size a column (uses col_child_sizes)
 */
OObj_DECL_GET(WGrid, float *, col_sizes)

/**
 * @param obj WGrid object
 * @return mutable size array with a height for each row
 *         pass <0 (-1) to auto size a row (uses row_child_sizes)
 */
OObj_DECL_GET(WGrid, float *, row_sizes)

/**
 * @param obj WGrid object
 * @return actual used column widths after an update
 */
OObj_DECL_GET(WGrid, float *, col_used_sizes)

/**
 * @param obj WGrid object
 * @return actual used row heights after an update
 */
OObj_DECL_GET(WGrid, float *, row_used_sizes)

/**
 * @param obj WGrid object
 * @return weights for min size, <=0 to ignore (default)
 */
OObj_DECL_GET(WGrid, float *, col_weights)

/**
 * @param obj WGrid object
 * @return weights for min size, <=0 to ignore (default)
 */
OObj_DECL_GET(WGrid, float *, row_weights)

/**
 * @param obj WGrid object
 * @return horizontal default align mode (left : right) (defaults to start)
 */
OObj_DECL_GETSET(WGrid, enum WGrid_align_mode, align_h)

/**
 * @param obj WGrid object
 * @return vertical default align mode (top : bottom) (defaults to start)
 */
OObj_DECL_GETSET(WGrid, enum WGrid_align_mode, align_v)

/**
 * @param obj WGrid object
 * @param align_h horizontal default align mode (left : right) (defaults to start)
 * @param align_v vertical default align mode (top : bottom) (defaults to start)
 */
o_inline void WGrid_align_set(oobj obj, enum WGrid_align_mode align_h, enum WGrid_align_mode align_v)
{
    WGrid_align_h_set(obj, align_h);
    WGrid_align_v_set(obj, align_v);
}

/**
 * @param obj WGrid object
 * @param cols, rows new grid size
 */
void WGrid_resize(oobj obj, int cols, int rows);


/**
 * Returns all WObj children at that col, row.
 * @param obj WGrid object
 * @param col, row to search at
 * @param only_lt if true, child needs to start at that pos (else including span)
 * @param opt_out_num set to the returned list size
 * @return NULL terminated list of children, needs to be o_free'd (allocated on obj)
 * @note quite heavy call, needs to look at all children
 */
WObj **WGrid_at_all(oobj obj, int col, int row, bool only_lt, osize *opt_out_num);

/**
 * Returns first WObj children at that col, row.
 * @param obj WGrid object
 * @param col, row to search at
 * @param only_lt if true, child needs to start at that pos (else including span)
 * @return first child at that pos, or NULL if none
 * @note calls WGrid_at_all and returns [0]
 */
struct oobj_opt WGrid_at(oobj obj, int col, int row, bool only_lt);

#endif //W_WGRID_H
