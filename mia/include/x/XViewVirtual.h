#ifndef X_XVIEWVIRTUAL_H
#define X_XVIEWVIRTUAL_H

/**
 * @file XViewVirtual.h
 *
 * object.
 *
 * AView which shows a given AView virtually in that on the bottom.
 * The resolution is independent and can be controlled with WNum's at the top.
 * Also, a screenshot may be taken, cause the AView is (of course...) rendered to a new RTex.
 *
 * Subclass of the AView object
 */


#include "a/AView.h"


/** object id */
#define XViewVirtual_ID AView_ID "XViewVirtual."

typedef struct {
    AView super;

    // virtual view
    AView *view;

    // RTex to render the virtual view in
    // AView's option to use its own tex is for a different purpose...
    oobj tex;

    ivec2 resolution;
    bool resolution_set;

    float zoom;

    // WObj based gui, which has is a WBox vertical
    oobj theme;
    oobj gui_root;
    oobj container_space;
    oobj container_view;
    oobj gui;
    oobj num_cols;
    oobj num_rows;
    oobj num_zoom;
    oobj btn_screenshot;

} XViewVirtual;


/**
 * Initializes the object.
 * @param obj XViewVirtual object
 * @param parent to inherit from
 * @param view AView to show virtually
 * @param move_view if true, view is o_move'd into this object
 * @param resolution for the virtual view
 * @return obj casted as XViewVirtual
 */
XViewVirtual *XViewVirtual_init(oobj obj, oobj parent, AView *view, bool move_view, ivec2 resolution);

/**
 * Creates a new XViewVirtual object
 * @param parent to inherit from
 * @param view AView to show virtually
 * @param move_view if true, view is o_move'd into this object
 * @param resolution for the virtual view
 * @return The new object
 */
o_inline XViewVirtual *XViewVirtual_new(oobj parent, AView *view, bool move_view, ivec2 resolution)
{
    OObj_DECL_IMPL_NEW(XViewVirtual, parent, view, move_view, resolution);
}


//
// virtual implementations
//

void XViewVirtual__v_setup(oobj view);

void XViewVirtual__v_update(oobj view, oobj tex, float dt);

void XViewVirtual__v_render(oobj view, oobj tex, float dt);


//
// object functions:
//


/**
 * @param obj XViewVirtual object
 * @return AView which is shown virtually
 */
OObj_DECL_GET(XViewVirtual, AView *, view)

/**
 * @param obj XViewVirtual object
 * @return resolution for the virtual view
 */
OObj_DECL_GET(XViewVirtual, ivec2, resolution)

/**
 * @param obj XViewVirtual object
 * @return resolution for the virtual view
 */
o_inline ivec2 XViewVirtual_resolution_set(oobj obj, ivec2 set)
{
    OObj_assert(obj, XViewVirtual);
    XViewVirtual *self = obj;
    self->resolution = set;
    self->resolution_set = true;
    return self->resolution;
}

/**
 * @param obj XViewVirtual object
 * @return WObj (WBox vertical) for the gui at the top
 */
OObj_DECL_GET(XViewVirtual, oobj, gui)

#endif //X_XVIEWVIRTUAL_H
