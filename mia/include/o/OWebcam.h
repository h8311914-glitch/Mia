#ifndef O_OWEBCAM_H
#define O_OWEBCAM_H

/**
 * @file OWebcam.h
 *
 * Object
 *
 * Access to the hardware webcam to take images / photos / video streams.
 *
 * @note The returned frames are camera native.
 *       Smartphones in portrait mode may show the cam 90° offset in its raw landscape format.
 *       An app code developer need to be aware of that and either rotate, if on portrait
 *           or just give the option to rotate to the user.
 *       (Mostly in native mobile apps, mobile browsers seem to handle that already)
 *       In contrast mobile browsers seem to needa re-open
 *           of the camera to align to its new source ratio.
 * 
 * @note TLDR;
 *       - Rotate manually or create user rotate buttons (platform mobile native)
 *       - Reopen the webcam on portrait <> landscape change (platform emscripten)
 *
 */


#include "OObj.h"
#include "img.h"

/** object id */
#define OWebcam_ID OObj_ID "OWebcam."

enum OWebcam_position {
    OWebcam_position_UNKNOWN,
    OWebcam_position_FRONT_FACING,
    OWebcam_position_BACK_FACING,
    OWebcam_position_ENUM_MAX
};

typedef struct {
    OObj super;

    struct o_img frame;
    ou64 time_ns;

    int camera_idx;
    int num_cameras;

    void *impl;
} OWebcam;


/**
 * Initializes the object
 * @param obj OWebcam object
 * @param parent to inherit from
 * @param auto_open if true, OWebcam_open(.., 0, 0) is called immediately
 * @return obj casted as OWebcam
 */
OWebcam *OWebcam_init(oobj obj, oobj parent, bool auto_open);

/**
 * Creates a new OWebcam object
 * @param parent to inherit from
 * @param auto_open if true, OWebcam_open(.., 0, 0) is called immediately
 * @return The new object
 */
o_inline OWebcam *OWebcam_new(oobj parent, bool auto_open)
{
    OObj_DECL_IMPL_NEW(OWebcam, parent, auto_open);
}

//
// virtual implementations:
//

/**
 * Calls OWebcam_release()
 * @param obj OWebcam object
 */
void OWebcam__v_del(oobj obj);

//
// object functions:
//

/**
 * @param obj OWebcam object
 * @return true if the camera is ready and available
 */
bool OWebcam_ready(oobj obj);

/**
 * @param obj OWebcam object
 * @return the next captured frame or NULL if no new frames are available.
 *         will return a reference to OWebcam_frame.
 * @note May free the old frame.
 *       Raw camera rotation, so may be 90° in phones in portrait mode (mostly native mobile apps)
 */
struct o_img *OWebcam_capture(oobj obj);


/**
 * @param obj OWebcam object
 * @return the last captured frame or NULL if no frame is available.
 * @note Raw camera rotation, so may be 90° in phones in portrait mode (mostly native mobile apps)

 */
o_inline struct o_img *OWebcam_frame(oobj obj)
{
    OObj_assert(obj, OWebcam);
    OWebcam *self = obj;
    return self->frame.data? &self->frame : NULL;
}

/**
 * @param obj OWebcam object
 * @return timestamp in [ns] of the last captured frame
 */
OObj_DECL_GET(OWebcam, ou64, time_ns)

/**
 * @param obj OWebcam object
 * @return current used camera idx [0:num_cameras) or -1 if nothing is opened
 */
OObj_DECL_GET(OWebcam, int, camera_idx)

/**
 * @param obj OWebcam object
 * @return available cameras (may be 0)
 */
OObj_DECL_GET(OWebcam, int, num_cameras)

/**
 * @param obj OWebcam object
 * @param camera_idx to query the position from, often UNKNOWN
 * @return the hardware camera position
 */
enum OWebcam_position OWebcam_position(oobj obj, int camera_idx);


/**
 * @param obj OWebcam object
 * @param camera_idx To be opened [0:num_cameras) (or -1 to just release)
 * @param max_resolution if >0 and the camera allows for selecting a custom resolution:
 *                             the nearest maximal resolition is used (max(cols, rows)).
 *                       else (<=0): set to 1280px (which may result in 1280px X 720px = HD)
 *                       As said only if querying camera specs is allowed.
 *                          So may fail in browsers.
 * @return true if a camera is now open, false if the camera is closed (also on error)
 * @note Will free the frame.
 */
bool OWebcam_open(oobj obj, int camera_idx, int max_resolution);


/**
 * Release the current opened camera (or noop if nothing was opened)
 * @param obj OWebcam object
 * @note Will free the frame.
 */
o_inline void OWebcam_release(oobj obj)
{
    OWebcam_open(obj, -1, 0);
}

#endif //O_OWEBCAM_H
