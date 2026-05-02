#ifndef U_ZOOM_H
#define U_ZOOM_H

/**
 * @file zoom.h
 *
 * RCam zoom using with a_pointer and multitouch or mouse wheel.
 * The resulting cam is not rounded to units.
 */

#include "m/types/flt.h"


struct u_zoom {

    // true if camera is move'd
    // set false until both pointer(0) and pointer(1) are up
    bool move;

    // resulting cam position and zoom
    vec2 pos;
    float zoom;

    // RCam to work on
    oobj cam;

    // true to handle events (default)
    bool handle_events;

    // time until timeout
    float move_timeout;
    
    // zoom scale each step
    float wheel_zoom;
    
    bool move_active;
    float move_time;
    vec2 move_start_pos;
    float move_start_dist;
    float wheel_integral;
};

/**
 * Creates a new u_zoom member
 * @param cam RCam object reference
 *            calls RCam_pixelperfect_view_set(cam, false);  to allow smooth zooming
 * @return u_zoom member
 */
struct u_zoom u_zoom_new(oobj cam);


//
// struct functions
//


/**
 * Updates the zooming, uses a_pointer_p to get a pointer
 * @param self u_zoom member
 * @param dt delta time to apply the zoom speed
 * @return self->zoom; true if camera is move'd
 */
bool u_zoom_update(struct u_zoom *self, float dt);


#endif //U_ZOOM_H
