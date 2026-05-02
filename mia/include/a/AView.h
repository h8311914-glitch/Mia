#ifndef A_AVIEW_H
#define A_AVIEW_H

/**
 * @file AView.h
 *
 * Object
 *
 * Holds a view tex with setup, update and render functions, bundled in an AView_layer.
 * Views are used in AScene or for ASplit's, for example.
 *
 * You need to hold some data?
 * Have a look at o_user* to set and get for example an allocated (on OTask) struct of data.
 *
 * The overall call chain for updates and rendering of AScene's and AView_layer's'm is:
 * (first <-> last)
 *       update
 *     update
 *   update
 * update
 * render
 *   render
 *     render
 *       render
 * 
 * So for your main AScene (on the left) or a main AView_layer (first) it does not matter to distinguish between update and render.
 * But for each following it DOES matter (handle pointers).
 */


#include "o/OObj.h"
#include "m/types/int.h"
#include "r/quad.h"
#include "a/pointer.h"

/** object id */
#define AView_ID OObj_ID "AView."

/**
 * Virtual update|render function to handle events, updates and render stuff.
 * @param obj AView object
 * @param tex RTex object to render on ( AView_tex() )
 * @param dt time between last frame
 */
typedef void (*AView__fn)(oobj view, oobj tex, float dt);

struct AView_layer {
    // will be set to NULL after first call (always directly before update)
    OObj__event_fn opt_setup;
    AView__fn update;
    AView__fn render;
};

/**
 * Returns a pointer in gl / display coordinations [-1 : 1] (x, y, 0, 1) (+y is up).
 * If hovering is not available, pos will be set to NAN in xy.
 * @param view AView object
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen
 *                      1+ -> touch as above
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @return the pointer for the given index (0=default)
 */
typedef struct a_pointer (*AView__pointer_display_fn)(oobj view, int idx, int history);


typedef struct {
    OObj super;

    // in coordinates of the tex to render on
    ivec4 viewport;

    // AView's position and size on the display (for pointers) (display coords [-1:+1] centered +y up)
    mat4 display_pose;
    mat4 display_pose_inv;

    // RCam, which is updated to the viewport size
    oobj cam;
    bool cam_units_auto;

    float scale;

    // if true (defaults to false), scale is automatically passed from the projection.
    // but maximal "scale_auto_max" (defaults to m_MAX)
    bool scale_auto;
    float scale_auto_max;

    // automatically calls AView_render_tex which renders the own_tex on the backbuffer
    bool render_auto;


    // color shader mode for AView_render_tex (default is false)
    bool render_colored;
    vec4 render_colored_rgba;
    vec4 render_colored_hsva;

    // added up on each render call
    double time;

    // RTex reference to render on (either from the calling update function, or own_tex)
    oobj tex;

    // own texture to render for this AView, optional (if use_own_tex is true)
    oobj own_tex;
    bool use_own_tex;
    
    // OArray of struct AView_layer
    oobj layers;
    
    // idx of currently updated or rendered layer
    // -1 if not active
    int current_layer;
    
    // true if currently in update, render state
    bool in_update;
    bool in_render;

    AView__pointer_display_fn v_pointer_display;
} AView;


/**
 * Initializes the object.
 * @param obj AView object
 * @param parent to inherit from
 * @param opt_setup Virtual setup function. Called once on scene startup, or NULL
 * @param update Virtual update function to handle events and updates
 * @param render Virtual render function 
 * @return obj casted as AView
 */
AView *AView_init(oobj obj, oobj parent,
                  OObj__event_fn opt_setup,
                  AView__fn update, AView__fn render);

/**
 * Creates a new AView object.
 * @param parent to inherit from
 * @param opt_setup Virtual setup function. Called once on scene startup, or NULL
 * @param update Virtual update function to handle events and updates
 * @param render Virtual render function 
 * @return The new object
 */
o_inline AView *AView_new(oobj parent,
                 OObj__event_fn opt_setup,
                 AView__fn update, AView__fn render) {
    OObj_DECL_IMPL_NEW(AView, parent, opt_setup, update, render);
}

/**
 * Creates a new AView object with cleared background color.
 * For debugging of AView layouts
 * @param parent to inherit from
 * @param opt_setup Virtual setup function. Called once on scene startup, or NULL
 * @param update Virtual update function to handle events and updates
 * @param render Virtual render function 
 * @return The new object
 */
AView *AView_new_color(oobj parent, vec4 color);

//
// virtual implementations
//

/**
 * Just a delegator to a_pointer_back
 */
struct a_pointer AView__v_pointer_display(oobj obj, int idx, int history);

//
// object functions:
//


/**
 * May update the internal tex and calls the virtual layers update functions
 * @param obj AView object
 * @param viewport for this view in tex viewport coordinates, may be unsafe
 * @param display_pose relative pose of the AView on the screen in display coords ([-1:+1] centered +y up)
 *                     For rendering offscreen, pass smth like u_pose_new_hidden where trans x (,y) >> +1
 * @note if this is a sub AView, combine the relative display_pose with the top like parent_pose @ child_pose
 */
void AView_update_ex(oobj obj, oobj tex, ivec4 viewport, mat4 display_pose);

/**
 * May update the internal tex and calls the virtual layers update functions.
 * Automatically sets the display_pose according to the current a_app_view() display_pose and the given viewport
 * @param obj AView object
 * @param viewport for this view in tex viewport coordinates, may be unsafe
 */
void AView_update(oobj obj, oobj tex, ivec4 viewport);


/**
 * May update the internal tex and calls the virtual layers render functions
 * @param obj AView object
 */
void AView_render(oobj obj, oobj tex);

/**
 * Renders this view onto the current bound framebuffer
 * @param obj AView object
 * @param tex RTex object to render on
 */
void AView_render_tex(oobj obj, oobj tex);


/**
 * @param obj AView object
 * @return Current viewport in upper RTex viewport coordinates (lbwh)
 */
OObj_DECL_GET(AView, ivec4, viewport)

/**
 * @param obj AView object
 * @return Current pose of the AView on the display in display coordinates [-1:+1]
 *         (So mat4_eye() would be the full display...)
 */
OObj_DECL_GET(AView, mat4, display_pose)

/**
 * @param obj AView object
 * @return inv(display_pose) for the pointer events
 */
OObj_DECL_GET(AView, mat4, display_pose_inv)

/**
 * @param obj AView object
 * @return RCam which is updated to the tex size
 */
OObj_DECL_GETSET(AView, oobj, cam)

/**
 * @param obj AView object
 * @return true if min_units_size should be set to match the given size in update
 */
OObj_DECL_GETSET(AView, bool, cam_units_auto)


/**
 * Will set AView_cam_units_auto to false.
 * Set the min_units_size to AView_cam and call RCam_update afterwards
 * @param obj AView object
 * @param min_units_size min_units_size for the RCam
 */
void AView_cam_min_units_set(oobj obj, vec2 min_units_size);

/**
 * @param obj AView object
 * @return unit scale, may be updated according to current render cam scale
 */
OObj_DECL_GETSET(AView, float, scale)

/**
 * @param obj AView object
 * @return if true (defaults to true), scale is updated to the current render cam scale
 * @note defaults to true
 */
OObj_DECL_GETSET(AView, bool, scale_auto)

/**
 * @param obj AView object
 * @return if scale_auto true, maximal scale (defaults to m_MAX)
 * @note defaults to true
 */
OObj_DECL_GETSET(AView, float, scale_auto_max)


/**
 * @param obj AView object
 * @return if true, calls AView_render on update to render this view to the "parent" (in case RTex_use_own_tex is true)
 */
OObj_DECL_GETSET(AView, bool, render_auto)

/**
 * @param obj AView object
 * @return if true, AView_render renders using a color shader and render_color_rgba + *hsva color shifts
 */
OObj_DECL_GETSET(AView, bool, render_colored)

/**
 * @param obj AView object
 * @return AView_render color scale if AView_render_colored is true and it renders using own_tex
 */
OObj_DECL_GETSETREF(AView, vec4, render_colored_rgba)

/**
 * @param obj AView object
 * @return AView_render color shift if AView_render_colored is true and it renders using own_tex
 */
OObj_DECL_GETSETREF(AView, vec4, render_colored_hsva)



/**
 * @param obj AView object
 * @return time that this view is running in seconds
 */
OObj_DECL_GET(AView, double, time)


/**
 * @param obj AView object
 * @return RTex to render on
 */
OObj_DECL_GET(AView, oobj, tex)


/**
 * @param obj AView object
 * @return RTex of this view
 */
OObj_DECL_GET(AView, oobj, own_tex)


/**
 * @param obj AView object
 * @return if true, tex = own_tex, which is created for this view (see and set AView_scale and AView_scale_auto)
 */
OObj_DECL_GETSET(AView, bool, use_own_tex)


/**
 * @param obj AView object
 * @return OArray of struct AView_layer
 */
OObj_DECL_GET(AView, oobj, layers)


/**
 * Pushs a layer with an OEvent in the next frame.
 * Adding a layer directly may result in an invalid call order (render before setup:update)
 * @param obj AView object
 * @param layer to add deferred
 * @param back if true, front if false
 */
void AView_push_layer_deferred(oobj obj, struct AView_layer layer, bool back);


/**
 * @param obj AView object
 * @return current active layer idx, or -1
 */
OObj_DECL_GET(AView, int, current_layer)

/**
 * @param obj AView object
 * @return true if currently in update state
 */
OObj_DECL_GET(AView, bool, in_update)

/**
 * @param obj AView object
 * @return true if currently in render state
 */
OObj_DECL_GET(AView, bool, in_render)



/**
 * Virtual pointer display function, as AView_pointer* source
 * @param obj AView object
 * @param pointer_display function
 */
o_inline void AView_pointer_display_fn_set(oobj obj, AView__pointer_display_fn pointer_display) {
    OObj_assert(obj, AView);
    AView *self = obj;
    self->v_pointer_display = pointer_display;
}


/**
 * Returns a pointer in display coordinations [-1 : 1] (x, y, 0, 1).
 * If hovering is not available, pos will be set to NAN in xy.
 * @param obj AView object
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @return the pointer for the given index (0=default)
 */
o_inline struct a_pointer AView_pointer_display(oobj obj, int idx, int history)
{
    OObj_assert(obj, AView);
    AView *self = obj;
    return self->v_pointer_display(self, idx, history);
}

/**
 * Returns a pointer in the views coordinates as [-1.0:+1.0]
 * If hovering is not available, pos will be set to NAN in xy.
 * @param obj AView object
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @return pointer pos (x, y, 0, 1) in views cam, active=false if not within scenes quad
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @note asserts bounds
 *       uses the opt_pointer function with opt_pointer_parent
 */
struct a_pointer AView_pointer_raw(oobj obj, int idx, int history);


/**
 * Returns a pointer in the views cam viewport coordinations
 * with only the perspective cam (p_only) (x, y, 0, 1).
 * If hovering is not available, pos will be set to NAN in xy.
 * @param obj AView object
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @return pointer pos (x, y, 0, 1) in views cam, active=false if not within scenes quad
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @note asserts bounds
 *       uses the opt_pointer function with opt_pointer_parent
 */
struct a_pointer AView_pointer_p(oobj obj, int idx, int history);


/**
 * Returns a pointer in the views cam viewport coordinations (x, y, 0, 1).
 * If hovering is not available, pos will be set to NAN in xy.
 * @param obj AView object
 * @param idx Mouse:    0 -> left button
 *                      1 -> right button
 *                      2 -> middle button
 *            Touch:    0 -> single touch / first finger
 *                      1+ -> multitouch
 *                      [0 : A_POINTER_MAX)
 *            Pen:      0 -> pen (also while hovering)
 * @return pointer pos (x, y, 0, 1) in views cam, active=false if not within scenes quad
 * @param history 0 for newest,
 *                  [1 : a_pointer_history_num] for "old" pointer between this and the last frame (bigger:=older)
 * @note asserts bounds
 *       uses the opt_pointer function with opt_pointer_parent
 */
struct a_pointer AView_pointer(oobj obj, int idx, int history);



#endif //A_AVIEW_H
