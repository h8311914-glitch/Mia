#ifndef EX_EXVIEWOVERLAY_H
#define EX_EXVIEWOVERLAY_H

/**
 * @file EXViewOverlay.h
 *
 * object.
 *
 * AView which shows a cancel button, which will o_del a given object.
 *                   and a source code button, to open an url (optionally).
 */

#include "a/AView.h"


/** object id */
#define EXViewOverlay_ID AView_ID "EXViewOverlay."

typedef struct {
    AView super;

    OObj__event_fn event;

    oobj theme;
    oobj gui;
    oobj cancel_btn;
    oobj source_btn;
    
    char *opt_url;

} EXViewOverlay;


/**
 * Initializes the object.
 * Creates an AView that renders a button. If its pressed the del_target get's o_del'ed
 * @param obj EXViewOverlay object
 * @param parent to inherit from
 * @param event to be called on cancel button clicked (may be NULL)
 * @param opt_url if not NULL shows a source button to open that url
 * @return obj casted as EXViewOverlay
 */
EXViewOverlay *EXViewOverlay_init(oobj obj, oobj parent, OObj__event_fn event, const char *opt_url);

/**
 * Creates a new EXViewOverlay object
 * Creates an AView that renders a button. If its pressed the del_target get's o_del'ed
 * @param parent to inherit from
 * @param event to be called on cancel button clicked (may be NULL)
 * @param opt_url if not NULL shows a source button to open that url
 * @return The new object
 */
o_inline EXViewOverlay *EXViewOverlay_new(oobj parent, OObj__event_fn event, const char *opt_url)
{
    OObj_DECL_IMPL_NEW(EXViewOverlay, parent, event, opt_url);
}


//
// virtual implementations
//

void EXViewOverlay__v_setup(oobj view);

void EXViewOverlay__v_update(oobj view, oobj tex, float dt);

void EXViewOverlay__v_render(oobj view, oobj tex, float dt);


//
// object functions:
//

/**
 * @param obj EXViewOverlay object
 * @return event to be called on button clicked (may be NULL)
 */
OObj_DECL_GETSET(EXViewOverlay, OObj__event_fn , event)


#endif //EX_EXVIEWOVERLAY_H
