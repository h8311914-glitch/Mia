#ifndef X_VIEWTEX_H
#define X_VIEWTEX_H

/**
 * @file viewtex.h
 *
 * Create a scene with a XViewTex as AView
 */

#include "XViewTex.h"



/**
 * Creates a full opaque scene, which shows XViewTex
 * @param parent to inherit from
 * @param tex to show
 * @param move_tex if true, tex is o_move'd into the views ro
 * @param opt_title if given, rendered below the RTex
 * @param opt_pointer if given, called for pointer events
 * @return AScene object
 */
oobj x_viewtex_scene(oobj parent, oobj tex, bool move_tex,
                     const char *opt_title, XViewTex_pointer_fn opt_pointer);


/**
 * Creates a full opaque scene, which shows XViewTex
 * @param parent to inherit from
 * @param mat to create an RTex from to show
 * @param opt_title if given, rendered below the RTex
 * @param opt_pointer if given, called for pointer events
 * @return AScene object
 */
oobj x_viewtex_mat_scene(oobj parent, oobj mat,
                         const char *opt_title, XViewTex_pointer_fn opt_pointer);



#endif //X_VIEWTEX_H
