#ifndef X_VIEWVIRTUAL_H
#define X_VIEWVIRTUAL_H

/**
 * @file viewvirtual.h
 *
 * Create a scene with a XViewVirtual as AView.
 * Or install it with an inbuilt installed AViewStage to apps stage
 */

#include "XViewTex.h"



/**
 * Creates a full opaque scene, which shows XViewVirtual
 * @param parent to inherit from
 * @param view AView to show virtually
 * @param move_view if true, view is o_move'd into this object
 * @param resolution for the virtual view
 * @return AScene object
 */
oobj x_viewvirtual_scene(oobj parent, AView *view, bool move_view, ivec2 resolution);

/**
 * Creates a full opaque scene, which shows XViewVirtual.
 * Installs a new AViewStage as view and sets it as current app stage
 * @param parent to inherit from
 * @param resolution for the virtual view
 * @return AScene object
 */
oobj x_viewvirtual_scene_install_stage(oobj parent, ivec2 resolution);



#endif //X_VIEWVIRTUAL_H
