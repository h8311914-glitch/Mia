#ifndef X_XVIEWLOG_H
#define X_XVIEWLOG_H

/**
 * @file XViewLog.h
 *
 * object.
 *
 * AView which shows the last few logs live.
 *
 * Subclass of the AView object
 */

#include "m/vec/vec2.h"
#include "a/AView.h"
#include "o/log.h"

/** object id */
#define XViewLog_ID AView_ID "XViewLog."

#define XViewLog_LOG_BUF_SIZE 256


typedef struct {
    AView super;

    vec4 bg_color;
    float text_alpha;
    
    // log line needs to have this string somewhere
    // NULLor "" to ignore
    char *filter;

    char text_buf[O_LOG_RING_SIZE * XViewLog_LOG_BUF_SIZE];
    
    int zoom;

    oobj text;
    oobj theme;
    oobj gui;
    oobj btn_zoom_plus;
    oobj btn_zoom_minus;
    oobj btn_level_prev;
    oobj btn_level_next;
    oobj btn_metrics;
    oobj level_label;
} XViewLog;


/**
 * Initializes the object.
 * Creates an AView that renders the last few logs live.
 * @param obj XViewLog object
 * @param parent to inherit from
 * @return obj casted as XViewLog
 */
XViewLog *XViewLog_init(oobj obj, oobj parent);

/**
 * Creates an AView that renders the last few logs live.
 * @param parent to inherit from
 * @return The new object
 */
o_inline XViewLog *XViewLog_new(oobj parent)
{
    OObj_DECL_IMPL_NEW(XViewLog, parent);
}


//
// virtual implementations
//

void XViewLog__v_setup(oobj view);

void XViewLog__v_update(oobj view, oobj tex, float dt);

void XViewLog__v_render(oobj view, oobj tex, float dt);


//
// object functions:
//


/**
 * @param obj XViewLog object
 * @return The used background color (done with blending a colored white box), defaults to R_TRANSPARENT
 */
OObj_DECL_GETSET(XViewLog, vec4, bg_color)

/**
 * @param obj XViewLog object
 * @return The used text alpha (defaults to 0.8)
 */
OObj_DECL_GETSET(XViewLog, float, text_alpha)

/**
 * @param obj XViewLog object
 * @return log line needs to have this string somewhere.
 *         NULL or "" to ignore.
 */
OObj_DECL_GET(XViewLog, const char *, filter)

/**
 * @param obj XViewLog object
 * @return log line needs to have this string somewhere.
 *         NULL or "" to ignore.
 */
const char *XViewLog_filter_set(oobj obj, const char *filter);

#endif //X_XVIEWLOG_H
