/**
 * iui ^= immediate user interface.
 * Main purpose are debugging windows to edit values or colors in runtime.
 * In this example, a white box is jumping on pointer down (cursor click or touch begin).
 * The jump speed can be set with an iui window.
 * In addition to that, the background color can be set by another iui window.
 */


#include "mia.h"

/**
 * box physics:
 */
#define GRAVITY 500
#define MIN_POS 140

struct context {
    /**
     * These variables can be controlled by an iui window, see the update and render function
     */
    vec4 bg_color;
    float jump_speed;

    /**
     * box physics state
     */
    float pos;
    float speed;

    /**
     * RObjText object
     */
    oobj text;

    /**
     * RTex for the ground
     */
    oobj ground;
};


static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    /**
     * Adds an offset to the start position "left top".
     * Otherwise it would be displayed behind the overlying examples close button.
     * (Optional call, default is vec2_(8, 16))
     */
    x_iui_init_lt_set(vec2_(8, 48));

    C->bg_color = vec4_(0.5, 0.1, 0.1, 1.0);
    C->jump_speed = 200;


    /**
     * Info text.
     * Like new_font35 but adds a shadow border.
     * The shadow color can be changed if passed a vec4 color (pointer) instead of NULL.
     * In contrast to RTex_text_font35_shadow* we use an RObjText render object which caches the character boxes.
     * (In fact each character is rendered with a single r_box with in internal RObjBox render object).
     */
    C->text = RObjText_new_font35_shadow(view,
                                         "TAP TO JUMP\n\n\n"
                                         "OPEN \"JMP\" WINDOW TO\n"
                                         "ADJUST JUMP SPEED\n\n"
                                         "SLIDE TO CHANGE\n\n"
                                         "PRESS ON THE NUM TO\n"
                                         "OPEN A VIRTUAL KEYBOARD",
                                         NULL);

    /**
     * Top left pose for the whole text. Size of 1, 1 is default
     */
    RObjText_pose_set(C->text, u_pose_new(8, 96, 1, 1));


    /**
     * We create a new RTex for the ground box.
     * It should fade from top with 50% alpha (=128) to 6.25% (=16)
     * The default filter mode is NEAREST, which is great for pixel art related stuff.
     * We change that to LINEAR, so each drawn pixel on the screen fades linear between the texture pixels.
     */
    bvec4 shading[2];
    shading[0] = bvec4_(0, 0, 0, 128);
    shading[1] = bvec4_(0, 0, 0, 16);
    C->ground = RTex_new(view, shading, 1, 2);
    RTex_filter_set(C->ground, RTex_filter_LINEAR);
}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /**
     * Check for pointer down to jump
     */
    struct a_pointer p = a_pointer(0, 0);
    if (a_pointer_pressed(p)) {
        C->speed = -C->jump_speed;
    }


    /**
     * his one-liner creates an iui window to adjust the jump speed at runtime within the range [0, 500].
     */
    x_iui_float("JMP", &C->jump_speed, 0, 500);

    /**
     * Apply speed
     */
    C->speed += GRAVITY * dt;
    C->pos += C->speed * dt;
    C->pos = o_min(C->pos, MIN_POS);
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /**
     * Another iui window to set the rgb color component of the background color
     * .xyz is the rgb sub vec3 of the rgba/xyzw vec4
     * 
     * So call the x_iui_* stuff either in the update or render call.
     * To dismiss the window, stop calling it.
     *
     * The x_iui_* windows are registered as a new layer in the current AView
     */

    x_iui_rgb("BG", &C->bg_color.xyz);

    /**
     * clear will override the full texture with the given color 
     *     ... full viewport, for the full texture use RTex_clear_full
     */
    RTex_clear(tex, C->bg_color);


    /**
     * Render the info text
     */
    RTex_ro(tex, C->text);



    const float box_size = 20;
    const float box_left = 120;

    /**
     * Renders the white jumping box
     */
    oobj white = r_tex_white();
    vec4 box_rect = u_rect_new(box_left, C->pos, box_size, box_size);
    RTex_blit_rect(tex, white, box_rect, RTex_rect(white, 0, 0));


    /**
     * Renders the shading ground box.
     * This time we blend so we see the fading alpha values of the ground tex.
     * Cause ground tex was setup with LINEAR filtering, its a smooth fading from top to bottom
     * We'll create our own uv rect for texture lookup.
     *   If it would be the full uv (like RTex_rect(C->ground, 0, 0) == vec4_(0, 0, 1, 2))
     *     -> it would render 25% thick constant colors at top and bottom.
     *        (texels are centered, constant color comes from clamping, thats why.)
     *   So we resize the height to 1 centered (== vec4_(0, 0.25, 1, 1) )
     *     -> now it will only show the fading in between.
     */
    vec4 ground_rect = u_rect_new(box_left-box_size, MIN_POS+box_size, box_size*3, box_size);
    vec4 ground_uv = u_rect_new_center(0.5, 1, 1, 1);
    RTex_blend_rect(tex, C->ground, ground_rect, ground_uv);
}


oobj ex_03_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_UNSAFE);
    return scene;
}

/**
 * Summary:
 *
 * This example shows how easy it is to add an iui window.
 * With these you can tune colors or fixed value on runtime.
 * We also created a custom RTex with a linear filter mode for a fading ground
 */
