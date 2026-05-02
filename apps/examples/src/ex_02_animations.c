/**
 * Renders an animated candle with a rotating camera.
 * Below the candle, a text is rendered
 */


/**
 * You can also include each header file separately (instead of including a whole module or just mia.h).
 * This will speed up the compile time a little (barely measurable in unity builds (default) ...).
 * Its also more clear what components of mia are used.
 */
#include "o/common.h"
#include "m/vec/vec2.h"
#include "r/RTex_ex.h"
#include "r/RCam.h"
#include "r/RObjQuad.h"
#include "a/AView.h"
#include "a/AScene.h"
#include "u/rect.h"
#include "u/pose.h"


/**
 * $ex/candle.png is a spritesheet with 4 cols and 2 rows
 */
#define CANDLE_COLS 4
#define CANDLE_ROWS 2
#define CANDLE_FPS 6.0f


struct context {

    /**
     * Sums up dt for camera rotation [seconds]
     */
    float time;

    /**
     * RTex for rendering the background
     */
    oobj bg;

    /**
     * RObjQuad for the animated candle
     */
    oobj candle;

} ;


static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    /**
     * Manually creating the texture data for a gray scaled chess board background
     * from the math module:
     *      vec4 -> 4x float
     *      bvec4 -> 4x bytes (obyte; ou8; uint8_t)
     * This chess board buffer is passed into the RTex GPU texture (always expects rgba as obyte or bvec4 for a pixel)
     */
    bvec4 buf[4];
    buf[0] = buf[3] = bvec4_(40, 40, 40, 255);
    buf[1] = buf[2] = bvec4_(32, 32, 32, 255);
    C->bg = RTex_new(view, buf, 2, 2);

    /**
     * For the background we make use of uv scaling. See render function.
     * For short: the uv rect works like a lookup on the texture.
     *            width and height are normally set to the texture's size
     *            We set a much bigger size, so that the texture should appear multiple times side by side
     * The default RTex_wrap mode is RTex_wrap_CLAMP.
     * In this mode the last readable texture pixel is used and stretched around the actueal texture.
     * So we use mode REPEAT to actually repeat the chess board
     */
    RTex_wrap_set(C->bg, RTex_wrap_REPEAT);


    /**
     * Loading the candle and setting up the spritesheet atlas.
     * Using the sprite_cols, sprite_rows parameter works for
     *     spritesheets that form a (full)grid with equal-sized elements.
     * Its also possible to setup the sprite mechanism for sprite grids with spacing and sub grids in a bigger atlas.
     *     For that manually set the uv + sprite shift data.
     */
    C->candle = RObjQuad_new(view, 1, "$ex/candle.png", CANDLE_COLS, CANDLE_ROWS);
}

static void update(oobj view, oobj tex, float dt)
{
    /**
     * Nothing to do here
     */
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /**
     * Count the time up to eight seconds
     */
    const float interval = 8.0f;
    C->time += dt;
    C->time = m_mod(C->time, interval);
    float t = C->time / interval;

    const float radius = 60.0f;
    float rad = t * 2.0f * m_PI;
    float x = radius * m_cos(rad);
    float y = radius * m_sin(rad);

    /**
     * Get a reference to the views camera "RCam""
     */
    oobj cam = AView_cam(view);

    /**
     * Instead of setting the candle pose, we do reset the camera position this time.
     */
    RCam_center_set(cam, vec2_(x, y), true);
    
    /**
     * Apply the new camera matrices now onto the texture (proj'ection)
     * Else would be done automatically for the next frame
     */
    RCam_apply_proj(cam, tex);
    

    /**
     * For the background, we set a rect of size 1024, so it will hopefully always be rendered using full screen.
     * (Min default unit size is 180x180)
     * the pattern itself has a size of 2x2 pixels.
     * We create a uv scaling that repeats the chessboard with a doubled size, so using 4x4 units.
     */
    const float bg_size = 1024;
    const float bg_uv_scale = 4.0f;

    vec4 bg_rect = u_rect_new_center(0, 0, bg_size, bg_size);
    vec4 bg_uv = u_rect_new(0, 0, bg_size / bg_uv_scale, bg_size / bg_uv_scale);

    RTex_blit_rect(tex, C->bg, bg_rect, bg_uv);


    /**
     * Gets the first and only quad from the render object to set the pose and sprite.
     */
    struct r_quad *q = o_at(C->candle, 0);

    /**
     * sprite.x will be rounded in the shader and sets the current frame in x direction
     * What actually happens is, that the uv texture lookup is shifted like:
     *     uv.x += round(sprite.x) + sprite.width
     * As alternatives to sprites, see u/atlas which can be used to set uv's (great for variation in sizes)
     */
    q->sprite.x += dt * CANDLE_FPS;
    q->sprite.x = m_mod(q->sprite.x, CANDLE_COLS);

    /**
     * normal frames: first row
     * blue frames: second row
     */
    q->sprite.y = t<0.5? 0 : 1;

    /**
     * When rendering using a pose (Quad) instead of a rect (Box), we can also rotate the candle.
     * Here we create a little wobbling angle in [rad]
     */
    float wobble_t = m_mod(t*8, 1);
    float wobble_rad = 0.05*m_signal_wave(wobble_t);

    /**
     * Creating the candle pose, with the candle size according to the atlas.
     * We also add the wobble rotation angle to the pose.
     *     0.5, 0.5 to rotate around the center (relative to its size)
     */
    vec2 candle_size = u_pose_size(q->uv_pose);
    candle_size = vec2_scale(candle_size, 4);
    q->pose = u_pose_new_center(0, 0, m_2(candle_size));
    u_pose_rotate_around(&q->pose, 0.5, 0.5, wobble_rad);


    /**
     * Blends the render object onto our result tex
     */
    RTex_ro(tex, C->candle);


    /**
     * Renders a text onto the RTex tex.
     * "RTex_font35" is the used (pixel art) font texture, it has a size of 3 cols * 5 rows for a character.
     * "RTex_font35_shadow" also exist with a shadow for the text.
     * As well as font55(_shadow) and font58(_shadow)
     *      -16, 40: let top position of the text
     *        2,  2: width and height scaling, so double the unit size
     * Have a look at the render object RObjText, if you need more control.
     */
    const char *text = t<0.5? "FIRE" : "BLUE\nFIRE";
    RTex_text_rect(tex, text, vec4_(-16, 40, 2, 2), RTex_font35, R_WHITE);
}


oobj ex_02_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);

    /**
     * We use the unsafe mode, because we want to render the moving background on the full screen.
     * Most platforms have the SAFE==UNSAFE zone, but modern smartphones (as an example) have a smaller SAFE zone.
     * The unsafe zone extends to the display region where a camera hole or nodge may block for interactive elements.
     */
    oobj scene = AScene_new(root, view, true, AScene_UNSAFE);
    return scene;
}

/**
 * Summary:
 *
 * In this example we learned how to deal with uv's and sprite animations.
 * We used a pose (Quad) instead of a rect (Box) to render the wobbling candle.
 * And controlling the view's camera instead of the sprite.
 * Last but not least, we printed a text on the screen
 */

