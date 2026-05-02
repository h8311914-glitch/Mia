#ifndef X_XVIEWTEX_H
#define X_XVIEWTEX_H

/**
 * @file XViewTex.h
 *
 * object.
 *
 * AView, which shows a zoom and drag able RTex.
 * Both touch gestures and mouse wheel work.
 * The RTex is located with lb at 0,0 and rt at size, so the pointer callback positions are in pixel coordinations.
 *
 * Subclass of the AView object
 */

#include "m/types/flt.h"
#include "a/AView.h"
#include "u/zoom.h"


/** object id */
#define XViewTex_ID AView_ID "XViewTex."

typedef void (*XViewTex_pointer_fn)(oobj view, struct a_pointer pointer);


typedef struct {
    AView super;

    // RTex to render
    oobj tex;
    
    // AView to display the tex
    oobj tex_view;

    // WTheme and WBox for gui
    oobj theme;
    oobj gui;
    oobj exit_btn;
    oobj cam_btn;
    oobj save_btn;
    oobj title_text;

    char *save_file;
    
    struct u_zoom zoom;

    // vfuncs
    OObj__event_fn v_done;
    XViewTex_pointer_fn v_opt_pointer;
} XViewTex;


/**
 * Initializes the object.
 * Creates an AView that renders a RTex with lb at 0, 0 and rt at size.
 * So the pointer position is on the pixel coordinate.
 * @param obj XViewTex object
 * @param parent to inherit from
 * @param tex to show
 * @param move_tex if true, tex is o_move'd into the internal RObjQuad object ro
 * @param done called if exit button is clicked
 * @param opt_title if given, rendered below the RTex
 * @param opt_pointer if given, called for pointer events
 * @return obj casted as XViewTex
 */
XViewTex *XViewTex_init(oobj obj, oobj parent, oobj tex, bool move_tex, OObj__event_fn done,
                        const char *opt_title, XViewTex_pointer_fn opt_pointer);

/**
 * Creates a new XViewTex object
 * Creates an XViewTex that renders a RTex with lb at 0, 0 and rt at size.
 * So the pointer position is on the pixel coordinate.
 * @param parent to inherit from
 * @param tex to show
 * @param move_tex if true, tex is o_move'd into the internal RObjQuad object ro
 * @param done called if the exit button is clicked
 * @param opt_title if given, rendered below the RTex
 * @param opt_pointer if given, called for pointer events
 * @return The new object
 */
o_inline XViewTex *XViewTex_new(oobj parent, oobj tex, bool move_tex, OObj__event_fn done,
                       const char *opt_title, XViewTex_pointer_fn opt_pointer)
{
    OObj_DECL_IMPL_NEW(XViewTex, parent, tex, move_tex, done, opt_title, opt_pointer);
}


//
// virtual implementations
//

void XViewTex__v_setup(oobj view);

void XViewTex__v_update(oobj view, oobj tex, float dt);

void XViewTex__v_render(oobj view, oobj tex, float dt);


//
// object functions:
//

/**
 * @param obj XViewTex object
 * @return rendered RTex object
 */
OObj_DECL_GET(XViewTex, oobj, tex)

/**
 * @param obj XViewTex object
 * @return rendered RTex object
 */
oobj XViewTex_tex_set(oobj obj, oobj tex, bool del_old);

/**
 * @param obj XViewTex object
 * @return WTheme for gui
 */
OObj_DECL_GET(XViewTex, oobj, theme)


/**
 * @param obj XViewTex object
 * @return WBox gui, add w tools like WNum here
 */
OObj_DECL_GET(XViewTex, oobj, gui)


#endif //X_XVIEWTEX_H
