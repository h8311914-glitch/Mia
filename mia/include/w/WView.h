#ifndef W_WVIEW_H
#define W_WVIEW_H

/**
 * @file WView.h
 *
 * Object (derives WObj)
 *
 * Widget that bundles an AView.
 *
 * >DOES NOT< call update or render on the AView automatically!
 * Either use WView_render(..) or call AView_update and AView_render yourself (WView_viewport...)
 */

#include "WObj.h"
#include "a/AView.h"

// forward struct
struct u_scroll;

/** o_user context for WView_new_theme */
struct WView__theme_context {
    oobj theme;
    oobj wobj;

    // NULL if disabled
    struct u_scroll *opt_scroll;
    vec4 scroll_extra_bounds;
};

/** object id */
#define WView_ID WObj_ID "View."

typedef struct {
    WObj super;

    bool hidden;
    AView *view;
    
    bool view_update;
    
    vec4 rect;
} WView;


/**
 * Initializes the object
 * @param obj WView object
 * @param parent to inherit from
 * @param view AView object to bundle in
 * @param move_view if true, view is o_move'd into this object
 * @return obj casted as WView
 */
WView *WView_init(oobj obj, oobj parent, oobj view, bool move_view);


/**
 * Creates a new WView object
 * @param parent to inherit from
 * @param view AView object to bundle in
 * @param move_view if true, view is o_move'd into this object
 * @return The new object
 */
o_inline WView *WView_new(oobj parent, oobj view, bool move_view)
{
    OObj_DECL_IMPL_NEW(WView, parent, view, move_view);
}

/**
 * Creates a new WView object.
 * Uses a new AView to render a WTheme with WTheme_update_full_tex.
 * @param parent to inherit from
 * @param theme WTheme to be used
 * @param move_theme if true, view is o_move'd into the internal AView
 * @param wobj WObj object to render with the theme
 * @param move_wobj if true, wobj is o_move'd into the internal AView
 * @param opt_scrollable_extra_bounds if not NULL, the AView cam gets scrollable with u_scroll and those extra bounds
 *                                    (the bounds may also be 0...)
 *                                    if .x < 0 -> only Y scrolling
 *                                    if .y < 0 -> only X scrolling
 * @return The new object
 * @note sets and uses o_user with struct WView__theme_context on the internal AView
 */
WView *WView_new_theme(oobj parent, oobj theme, bool move_theme, oobj wobj, bool move_wobj,
    const vec4 *opt_scrollable_extra_bounds);

/**
 * Creates a new WView object.
 * Uses a new AView to render a WTheme with WTheme_update_full_tex.
 * Calls WView_new_theme with WTheme_new_tiny(self)
 * @param parent to inherit from
 * @param wobj WObj object to render with the theme
 * @param move_wobj if true, wobj is o_move'd into the interal AView
 * @param opt_scrollable_extra_bounds if not NULL, the AView cam gets scrollable with u_scroll and those extra bounds
 *                                    (the bounds may also be 0...)
 *                                    if .x is NAN-> only Y scrolling
 *                                    if .y is NAN -> only X scrolling
 * @return The new object
 * @note sets and uses o_user with struct WView__theme_context on the internal AView
 */
WView *WView_new_theme_tiny(oobj parent, oobj wobj, bool move_wobj,
    const vec4 *opt_scrollable_extra_bounds);

//
// virtual implementations:
//


/**
 * Virtual implementation.
 * Calls the default WObj__v_update.
 */
vec2 WView__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn);


//
// object functions:
//


/**
 * @param obj WView object
 * @return hidden flag, set by wobj's update, reset by WView_render (which only renders if set)
 */
OObj_DECL_GETSET(WView, bool, hidden)

/**
 * @param obj WView object
 * @return in projection space ltwh (!=viewport...). Auto set in update from either child_size, min_size or fixed_size
 */
OObj_DECL_GETSET(WView, vec4, rect)

/**
 * @param obj WView object
 * @return AView object to bundle
 */
OObj_DECL_GET(WView, AView *, view)

/**
 * @param obj WView object
 * @param view AView object to bundle
 * @param del_old if true, the olf view is o_del'ed
 */
AView *WView_view_set(oobj obj, oobj view, bool del_old);

/**
 * Calls update and render on the bundled AView
 * @param obj WView object
 * @param tex RTex used to determine the viewport (needs the projection and RTex_viewport ...)
 * @return the RTex style viewport lbwh
 */
ivec4 WView_viewport(oobj obj, oobj tex);

/**
 * Calls update on the bundled AView.
 * Only if view_update is false
 * @param obj WView object
 * @param tex RTex to render onto
 */
void WView_update(oobj obj, oobj tex);

/**
 * Calls update and render on the bundled AView.
 * Only updates if view_update is false.
 * Sets view_update = false (for the next frame)
 * @param obj WView object
 * @param tex RTex to render onto
 */
void WView_render(oobj obj, oobj tex);

#endif //W_WVIEW_H
