/**
 * Renders a batch of colored quads, which can be positioned by a pointer.
 * A pointer is either a mouse cursor or touch input
 */


/**
 * Instead of including all of mia (with mia.h) we can include the needed modules.
 *
 * "o" is the base library, containing the object oriented base class, logging, arrays, ...
 * "r" is for rendering
 * "a" is for app stuff like creating the AScene
 * "u" is for utilities like u_pose stuff
 */
#include "o/o.h"
#include "r/r.h"
#include "a/a.h"
#include "u/u.h"


/**
 * Number of colored boxes to render
 */
#define NUM_BOXES 14


/**
 * As with example 00, all stuff used here.
 * But this time not as a static struct, but as an object context struct.
 * This has the advantage that the context is >NOT< shared by multiple instances of the same view.
 * Each view gets its own context in contrast to a static struct, which would be shared by all.
 */
struct context {
    /**
     * RObjBox to render a batch of colored boxes in a single draw call
     */
     oobj batch;

     /**
      * stores which box is currently dragged/picked
      * osize is one of the number primitives, in this case int64_t or oi64.
      * Mia uses signed sizes which are less error-prone and can pass -1 for failures, invalid item, etc.
      */
     osize dragged_idx;
};


static void setup(oobj view)
{

    /**
     * Each OObj may have user data.
     * We create the cleared context struct with o_new0 with view as parent.
     * So its automatically free'd when view is deleted.
     * o_user can save any void*, feel free to use it.
     */
    struct context *C = o_user_new0(view, *C, 1);

    /**
     * returns a shared RTex with a single white pixel, should not be deleted (asserts it)
     */
    oobj white = r_tex_white();

    /**
     * A RObjBox can render multiple (NUM_BOXES) boxes batched in a single draw call
     * "*_new" are constructors,
     * "*_new_tex_rgba" is a specialized constructor.
     *     in this case a texture is passed
     *     and the shader makes use of passed rgba colors
     *
     * We pass false for the "move_tex" parameter, because this object should >not< delete the shared RTex
     * And pass `1, 1`, because its a single/full sprite (1 col * 1 row).
     *      The next example demonstrates how to use sprites.
     */
    C->batch = RObjBox_new_tex_rgba(view, NUM_BOXES, white, false, 1, 1);

    /**
     * o_num is one of mia's object operators, its overloaded in RObjBox to return the number of boxes
     */
    for(osize i=0; i<o_num(C->batch); i++) {
        /**
         * o_at is another mia object operator to access an array item.
         * The RObjBox has an internal array of r_box'es
         */
        struct r_box *b = o_at(C->batch, i);

        /**
         * We set the rect for each r_box.
         * The batch's array box order is kept while rendering, so first will be drawn first
         * (x, y, w, h)
         */
        b->rect = u_rect_new(80, 20 + i*10, 20, 20);

        /**
         * converting a hsv(a) color to rgb(a)
         * vec4 -> 4x float
         * vec4_(...) creates an vec4 inplace
         *
         * rgba -> red, green, blue, alpha (transparency) [0:1]
         * hsva -> hue (color angle), saturation, value, alpha [0:1]
         */
        float t = (float) i / (float) o_num(C->batch);
        float hue = t * 0.66;
        vec4 hsva = vec4_(hue, 1, 1, 1);
        vec4 rgba = vec4_hsv2rgb(hsva);

        /**
         * An r_box has besides the rect additional fields for shaders, called 'fx'
         * In this case the rgba shader uses vec4 'fx' for the rgba color (albedo color)
         */
        b->fx = rgba;
    }

    /**
     * idx=0 will be a valid index, so we use -1 as "nothing dragged"
     */
    C->dragged_idx = -1;
}

static void update(oobj view, oobj tex, float dt)
{
    /**
     * Get user context from the view, created by setup
     */
    struct context *C = o_user(view);

    /**
     * Get current pointer
     * @param idx=0: is the multitouch index, so just ignoring and using the first pointer
     * @param history=0: can be used to travel back in history.
     *                   touch events may have a higher frequency then the screen refresh (often 60 fps)
     *                   so we can access back in history to get the in between pointers.
     *                   a_pointer_history_num() to get the number of history pointer items
     *
     */
    struct a_pointer p = a_pointer(0, 0);

    /**
     * Compares the current pointer with the one from the previous frame.
     * Check if its pressed (first frame to touch or click)
     */
    if(a_pointer_pressed(p)) {
        /**
         * Search which box contains the pointer position
         */
        C->dragged_idx = -1;
        for(osize i=0; i<o_num(C->batch); i++) {
            struct r_box *b = o_at(C->batch, i);

            /**
             * Returns true if the position is within the borders of the rect.
             * b: current checked box from the batch
             * p: current pointer
             * p.pos: vec4 of the pointer position (x, y, 0,  1)
             * p.pos.xy: vec2 ad xy of the position
             */
            if(u_rect_contains(b->rect, p.pos.xy)) {
                C->dragged_idx = i;
            }
        }
        if(C->dragged_idx>=0) {
            o_log("picked: %i", C->dragged_idx);
        }
    }

    /**
     * As with pressed, when the mouse unclicks or the touch ends
     */
    if(a_pointer_released(p)) {
        o_log("lost: %i", C->dragged_idx);
        C->dragged_idx = -1;
    }

    /**
     * While dragging, update the box's position
     */
    if(C->dragged_idx>=0) {
        struct r_box *b = o_at(C->batch, C->dragged_idx);

        /**
         * A short excursion on the subject of the math types from the m module ( "m/m_types/flt.h" )
         * The vector and matrix types are not just simple structs ala "struct {float x, y, z, w;};"
         * But make use of the union type.
         * A union >shares< memory for different internal names / types
         * Like:
         *
         *     union example {
         *         obyte bytes[4];
         *         int value;
         *     };
         *
         *     union example a;
         *     a.value = 1234;
         *
         * You can now access the bytes of the value with the field a.bytes and vice versa.
         * The vec2 type is formed as:
         *
         *     typedef union {
         *        float v[2];
         *        struct {
         *            float v0, v1;
         *        };
         *        struct {
         *            float r, g;
         *        };
         *        struct {
         *            float s, t;
         *        };
         *        struct {
         *            float left, top;
         *        };
         *        struct {
         *            float x, y;
         *         };
         *     } m_vec2;
         *
         * > typedef'fed to vec2 in ( "m/types/flt.h" )
         * So you can access the fields of the vec2 with the classic
         *      x and y
         *      OR r(ed) and (g)reen
         *      OR s, t, (p, q) like glsl texture uv coords
         *      OR left, top, (width, height) for vec4 -> rect's
         *      OR v0 and v1
         *      OR v[0] and v[1] (or pass the array pointer with .v)
         *
         * The vec3 gets a little bit more complicated:
         *
         *     typedef union {
         *         float v[3];
         *         struct {
         *             float v0, v1, v2;
         *         };
         *         struct {
         *             float r, g, b;
         *         };
         *         struct {
         *             float s, t, p;
         *         };
         *         struct {
         *             float left, top, width;
         *         };
         *         m_vec2 xy;
         *         struct {
         *             float x;
         *             union {
         *                 struct {
         *                     float y, z;
         *                 };
         *                 m_vec2 yz;
         *             };
         *         };
         *     } m_vec3;
         *
         * Apart from accessing a single element like x, y, z,
         * its also possible to access internal sub vec2 parts with .xy and .yz
         * vec4 also make use of this an addition to internal sub vec3 parts ala .xyz
         *
         * Back to this example we copy the pointers position into the box rect using the u_rect_set_center helper function.
         */
        u_rect_center_set(&b->rect, p.pos.x, p.pos.y);
    }

    /**
     * Pointer stuff should be handled in the update function.
     * When multiple transparent scenes work the same time, pointers are "handled" in the right order.
     */
}

static void render(oobj view, oobj tex, float dt)
{
    /**
     * Get user context from the view, created by setup
     */
    struct context *C = o_user(view);

    /**
     * Renders the batch onto the texture.
     * RTex_ro(tex, C->batch) could also be used.
     */
    RObj_render(C->batch, tex);
}


oobj ex_01_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}


/**
 * Summary:
 *
 * In this example, we demonstrated interactive pointer handling and rendering with Mia:
 *
 * - **Pointer Handling:** The update function tracks touch or mouse input to determine which box is being dragged.
 *   It manages pointer down events to pick a box, updates its position while dragging, and handles pointer up events.
 *
 * - **Rendering:** The RObjBox is used to batch and render multiple colored boxes in a single draw call.
 *   The box's positions and colors are updated based on user input, with rendering handled in the render function
 */
