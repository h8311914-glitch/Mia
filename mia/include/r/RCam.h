#ifndef R_RCAM_H
#define R_RCAM_H

/**
 * @file RCam.h
 *
 * Object
 *
 * A Camera for the render objects.
 * Origin is at left top.
 * +x -> to the right
 * +y -> to the bottom
 */


#include "o/OObj.h"
#include "r/common.h"
#include "r/proj.h"
#include "m/sca/flt.h"

/** object id */
#define RCam_ID OObj_ID "RCam"

struct RCam_matrices
{
    // projection of the camera (perspective / orthogonal)
    // pass 'p' to a render object,
    //     to draw it at a static position on the screen (HUD)
    // use 'p_inv' to transform a touch position to the screen units
    mat4 p;
    mat4 p_inv; // inv(p)

    // view / pose of the camera in 3d space
    mat4 v;
    mat4 v_inv; // inv(v)

    // combination of view and projection
    // pass 'vp' to a render object,
    //     to draw it in the world,
    //     so that the camera movement of 'v' is used
    // use 'v_p_inv' to transform a touch position to the world
    mat4 vp; // p @ v_inv
    mat4 v_p_inv; // v @ p_inv
};

/**
 * Sets matrices to eye
 * @param self The camera matrices to init
 */
void RCam_matrices_init(struct RCam_matrices* self);

/**
 * Updates all matrices (but .v and .p) according to .v and .p
 * @param self The camera matrices to update
 */
void RCam_matrices_update(struct RCam_matrices* self);

/**
 * Virtual update function.
 * Updates the internal .cam matrices and other fields.
 * @param camera_obj RCam object
 */
typedef void (*RCam__update_fn)(oobj camera_obj, int back_cols, int back_rows);


typedef struct
{
    OObj super;

    // given by update
    ivec2 back_size;

    struct r_proj proj;
    struct RCam_matrices cam;

    // options
    bool pixelperfect_scale;
    bool pixelperfect_view;
    float fixed_scale;
    vec2 min_units_size;

    vec2 pos;
    float rotation;
    float zoom;

    // vfuncs
    RCam__update_fn v_update;
} RCam;


/**
 * Initializes the object
 * @param obj RCam object
 * @param parent to inherit from
 * @param pixelperfect if true, each unit as an integer amount of real pixelc
 *                      sets both, _scale and _view
 * @param min_units_size to calc the scale, use <= to ignore, both <=0 to set scale to 1.0f
 * @return obj casted as RCam
 */
RCam* RCam_init(oobj obj, oobj parent, bool pixelperfect, vec2 min_units_size);

/**
 * Creates a new RCam object
 * @param parent to inherit from
 * @param pixelperfect if true, each unit as an integer amount of real pixelc
 *                      sets both, _scale and _view
 * @param min_units_size to calc the scale, use <= to ignore, both <=0 to set scale to 1.0f
 * @return The new object
 */
o_inline RCam* RCam_new(oobj parent, bool pixelperfect, vec2 min_units_size)
{
    OObj_DECL_IMPL_NEW(RCam, parent, pixelperfect, min_units_size);
}

//
// virtual implementations:
//

/**
 * virtual function
 * Updates the internal .cam matrices and other fields.
 * @param obj RCam object
 */
void RCam__v_update(oobj obj, int back_cols, int back_rows);


//
// object functions
//

/**
 * Calls the virtual resize function.
 * Extended version.
 * Updates the internal .cam matrices and other fields.
 * @param obj RCam object
 */
o_inline void RCam_update_ex(oobj obj, int back_cols, int back_rows)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    self->v_update(self, back_cols, back_rows);
}

/**
 * Calls the virtual resize function.
 * Simplified version, using the old back size
 * Updates the internal .cam matrices and other fields.
 * @param obj RCam object
 */
o_inline void RCam_update(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    self->v_update(self, m_2(self->back_size));
}


/**
 * @param obj RCam object
 * @return The back_size given by RCam_update_ex
 */
OObj_DECL_GET(RCam, ivec2, back_size)

/**
 * @param obj RCam object
 * @return A reference to the p matrix (perspective | orthogonal matrix).
 *         To be used for hud, etc. when the v is moved, ...
 */
o_inline const mat4* RCam_cam_p(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return &self->cam.p;
}

/**
 * @param obj RCam object
 * @return A reference to the inverted p matrix (perspective | orthogonal matrix)
 *         To be used for pointer events on huds, etc. when the v is moved, ...
 */
o_inline const mat4* RCam_cam_p_inv(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return &self->cam.p_inv;
}


/**
 * @param obj RCam object
 * @return A reference to the vp matrix.
 *         To be used to render.
 */
o_inline const mat4* RCam_cam(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return &self->cam.vp;
}

/**
 * @param obj RCam object
 * @return A reference to the inverted vp matrix.
 *         To be used for pointer events.
 */
o_inline const mat4* RCam_cam_inv(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return &self->cam.v_p_inv;
}

/**
 * @param obj RCam object
 * @return The pixelperfect scale option.
 */
OObj_DECL_GETSET(RCam, bool, pixelperfect_scale)

/**
 * @param obj RCam object
 * @return The pixelperfect view option.
 */
OObj_DECL_GETSET(RCam, bool, pixelperfect_view)

/**
 * @param obj RCam object
 * @return if > 0, used as fixed scale, else (default = -1) uses min_units_size
 */
OObj_DECL_GETSET(RCam, float, fixed_scale)

/**
 * @param obj RCam object
 * @return to calc the scale, use <= to ignore, both <=0 to set scale to 1.0f
 *         If changed, call update...
 */
OObj_DECL_GETSET(RCam, vec2, min_units_size)

/**
 * @param obj RCam object
 * @return The used pos for the v matrix
 * @note if pixelperfect: RCam_round_real_pixel is used on pos
 *
 */
OObj_DECL_GET(RCam, vec2, pos)

/**
 * @param obj RCam object
 * @param pos new position for the v matrix, call RCam_update to use it!
 * @param cam_update if true, RCam_update is called
 * @return The used pos for the v matrix
 * @note if pixelperfect: RCam_round_real_pixel is used on pos
 *
 */
vec2 RCam_pos_set(oobj obj, vec2 pos, bool cam_update);

/**
 * @param obj RCam object
 * @return center position, according to both v and p matrices
 *         `return mat4_mul_vec(*RCam_cam_inv(obj), vec4_(0.0, 0.0, 0.0, 1.0)).xy;`
 */
vec2 RCam_center(oobj obj);

/**
 * @param obj RCam object
 * @return center position rounded as integer, according to both v and p matrices
 *         `return mat4_mul_vec(*RCam_cam_inv(obj), vec4_(0.0, 0.0, 0.0, 1.0)).xy;`
 */
o_inline ivec2 RCam_center_int(oobj obj)
{
    vec2 center = RCam_center(obj);
    return ivec2_(m_floor(center.x), m_floor(center.y));
}

/**
 * @param obj RCam object
 * @return center position rounded as integer (vec2 version "unit"), according to both v and p matrices
 *         `return mat4_mul_vec(*RCam_cam_inv(obj), vec4_(0.0, 0.0, 0.0, 1.0)).xy;`
 */
o_inline vec2 RCam_center_unit(oobj obj)
{
    vec2 center = RCam_center(obj);
    return vec2_(m_floor(center.x), m_floor(center.y));
}

/**
 * @param obj RCam object
 * @param center_pos new position as screen center for the v matrix, call RCam_update before to use it!
 * @param cam_update if true, RCam_update is called
 * @return The used pos for the v matrix
 * @note if pixelperfect: RCam_round_real_pixel is used on pos
 *       Uses RCam_center() internally, that's why update should be called when smth changed before
 */
vec2 RCam_center_set(oobj obj, vec2 center_pos, bool cam_update);

/**
 * @param obj RCam object
 * @return The zoom of the v matrix (default = 1.0)
 *         If >1.0, the rendered objects appear smaller; <1.0: bigger
 *         If changed, call update...
 * @note if pixelperfect_view: rounded
 */
OObj_DECL_GET(RCam, float, zoom)

/**
 * @param obj RCam object
 * @param zoom new zoom for the v matrix, call RCam_update to use it!
 *        If >1.0, the rendered objects appear smaller; <1.0: bigger
 * @param cam_update if true, RCam_update is called
 * @return The zoom of the v matrix (default = 1.0)
 * @note if pixelperfect_view: if > 1, rounded
 *                             if < 1, rounded to inv "zoom = 1.0 / round(1.0 / zoom)"
 *       asserts zoom > 0
 */
float RCam_zoom_set(oobj obj, float zoom, bool cam_update);

/**
 * @param obj RCam object
 * @return A reference to the rotation of the v matrix [rad]
 * @note if pixelperfect_view: rounded to m_PI_2
 */
OObj_DECL_GET(RCam, float, rotation)

/**
 * @param obj RCam object
 * @param rotation_rad new rotation for the v matrix, call RCam_update to use it!
 * @param cam_update if true, RCam_update is called
 * @return A reference to the rotation of the v matrix [rad]
 * @note if pixelperfect_view: rounded to m_PI_2 steps (90°)
 */
float RCam_rotation_set(oobj obj, float rotation_rad, bool cam_update);

/**
 * @param obj RCam object
 * @param x position (left)
 * @param y position (top)
 * @return ltwh rect containing the current outer view, offset by xy
 *         If unrotated or rotated by m_PI_2 (pixelperfect_view) its exactly the camera view.
 *         Else if rotated around another value, its the bigger outer view rect
 */
vec4 RCam_rect(oobj obj, float x, float y);

/**
 * @param obj RCam object
 * @param x position (left)
 * @param y position (top)
 * @return pose containing the current view, offset by xy
 *         If rotated, the resulting pose is also rotated
 */
mat4 RCam_pose(oobj obj, float x, float y);


/**
 * @param obj RCam object
 * @return ref to r_proj matrices for rendering, etc.
 */
o_inline const struct r_proj* RCam_proj_ref(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return &self->proj;
}

/**
 * @param obj RCam object
 * @return copy of r_proj matrices for rendering, etc.
 */
o_inline struct r_proj RCam_proj(oobj obj)
{
    return *RCam_proj_ref(obj);
}


/**
 * @param obj RCam object
 * @return copy of r_proj matrices for rendering to the perspective only cam, etc.
 */
o_inline struct r_proj RCam_proj_p(oobj obj)
{
    struct r_proj res = *RCam_proj_ref(obj);
    res.cam = *RCam_cam_p(obj);
    return res;
}

/**
 * Applies RCam_proj onto RTex_proj
 * @param obj RCam object
 * @param tex RTex object to apply the proj on
 */
void RCam_apply_proj(oobj obj, oobj tex);

/**
 * Applies RCam_proj_p onto RTex_proj
 * @param obj RCam object
 * @param tex RTex object to apply the perspective only proj on
 */
void RCam_apply_proj_p(oobj obj, oobj tex);


//
// r_proj delegation
//

/**
 * @param obj RCam object
 * @return The projection unit size given by update
 * @note according to the p matrix, independent to position, rotation and zoom
 */
o_inline vec2 RCam_proj_size(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return self->proj.size;
}

/**
 * @param obj RCam object
 * @return projection size of the integer bounds (may be slightly less than proj->size)
 * @note according to the p matrix, independent to position, rotation and zoom
 */
o_inline ivec2 RCam_proj_size_int(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return r_proj_size_int(&self->proj);
}

/**
 * @param obj RCam object
 * @return Currently displayed unit size (according to zoom)
 */
o_inline vec2 RCam_size(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam *self = obj;
    return vec2_(self->proj.size.x * self->zoom,
                 self->proj.size.y * self->zoom);
}

/**
 * @param obj RCam object
 * @return Currently displayed unit size (according to zoom)
 *         integer bounds
 */
o_inline ivec2 RCam_size_int(oobj obj)
{
    vec2 size = RCam_size(obj);
    return ivec2_(m_ceil(size.x), m_ceil(size.y));
}


/**
 * @param obj RCam object
 * @return center position (may be half a pixel, see RCam_center_int)
 * @note according to the p matrix, independent to position, rotation and zoom
 */
o_inline vec2 RCam_proj_center(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return r_proj_center(&self->proj);
}

/**
 * @param obj RCam object
 * @return center position rounded as integer
 * @note according to the p matrix, independent to position, rotation and zoom
 */
o_inline ivec2 RCam_proj_center_int(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return r_proj_center_int(&self->proj);
}

/**
 * @param obj RCam object
 * @return vec2 version of RCam_center_int
 * @note according to the p matrix, independent to position, rotation and zoom
 */
o_inline vec2 RCam_proj_center_unit(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return r_proj_center_unit(&self->proj);
}

/**
 * @param obj RCam object
 * @return The scale (real pixels used for a unit)
 */
o_inline float RCam_proj_scale(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return self->proj.scale;
}

/**
 * @param obj RCam object
 * @param x position (left)
 * @param y position (top)
 * @return unit rect as ltwh according to the p matrix, containing the full projection on position xy
 */
o_inline vec4 RCam_proj_rect(oobj obj, float x, float y)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return r_proj_rect(&self->proj, x, y);
}

/**
* @param obj RCam object
 * @param x position (left)
 * @param y position (top)
 * @return unit pose according to the p matrix, containing the full projection on position xy (left top)
 */
o_inline mat4 RCam_proj_pose(oobj obj, float x, float y)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return r_proj_pose(&self->proj, x, y);
}


/**
 * @param obj RCam object
 * @return true if camera is in portrait mode (if cam is for the full screen -> smartphone portrait mode)
 * @note according to the p matrix, independent to position, rotation and zoom
 * @warning returns true if smartphone is in landscape but (sub) view is in portrait!
 */
o_inline bool RCam_is_portrait(oobj obj)
{
    OObj_assert(obj, RCam);
    RCam* self = obj;
    return r_proj_is_portrait(&self->proj);
}

/**
 * Sets pos and zoom to fit the rect (aabb) (centered).
 * rotation should have been 0 (is not set...)
 * @param obj RCam object
 * @param rect to fit
 * @param crop true: rect may be cropped (if ratio isnt equal)
 *             false: cam may show beyond the rect
 * @param opt_zoom_scale: if >0 its additinally applied to zoom (else like 1.0)
 *        as an example 1.1 to display 10% beyond the rect (as it would be bigger)
 * @note if ratio don't match, the rect is centered
 */
void RCam_fit(oobj obj, vec4 rect, bool crop, float opt_zoom_scale);

/**
 * Zooms at a specific position
 * @param obj RCam object
 * @param zoom_pos unit pos that should remain 
 *            at the same display position after zooming
 * @param zoom The zoom of the v matrix (default = 1.0)
 *            If >1.0, the rendered objects appear smaller; <1.0: bigger
 *            If changed, call update...
 * @param opt_pos_set if not NULL:
 *            Same as calling 
 *            RCam_pos_set(*opt_pos_set, true);
 *            before calling this (but optimised a lil more)
 * @note the cam should have been updated before.
 */
void RCam_zoom_at(oobj obj, vec2 zoom_pos, float zoom, const vec2 *opt_pos_set);


#endif //R_RCAM_H
