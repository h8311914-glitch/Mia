/**
 * This example is a simple drawing app.
 * Below the canvas are pressure sensitive pen axles as progress bars.
 * This works also in the web build (not all browsers, not all pens).
 * As an example the S-Pen is detected in chromium browsers and is able to retrieve:
 *     pressure, tilt.x, tilt.y
 */

#include "mia.h"

// Size of the canvas on the screen
static ivec2 DISPLAY_SIZE = {{128, 96}};

// Actual image / texture size
static ivec2 IMAGE_SIZE = {{128, 96}};

struct context {
    
    // RTex to draw on
    oobj canvas;

    oobj theme;
    oobj gui;

    // WObj which reserves space to render the canvas
    oobj canvas_area;

    // these are all WProgress bars (framed)
    oobj info;
    oobj pressure;
    oobj tilt_x;
    oobj tilt_y;
    oobj distance;
    oobj rotation;
    oobj slider;
    oobj tangential_pressure;

    // options set by iui
    vec4 brush_color;
    float stroke_size;

    // render object for the canvas
    oobj ro;

    struct a_pointer prev;
};

static oobj framed_progress(oobj parent, const char *title)
{
    oobj frame = WFrame_new_title(parent, title, NULL);
    oobj bg = WColor_new(frame, R_GRAY_X(0.33));
    oobj progress = WProgress_new(bg);
    WProgress_color_set(progress, vec4_(0.1, 0.8, 0.1, 1.0));
    WObj_min_size_ref(progress)->y = 4;
    return progress;
}

/**
 * Draws a line (batched) from a to b
 * @param a, b line end points, a==b draws a single point
 */
static void draw_line(oobj view, vec2 a, vec2 b)
{
    struct context *C = o_user(view);

    float dist = vec2_distance(a, b);
    int steps = 1 + dist * 2;

    oobj boxes = RObjBox_boxes(C->ro);
    OArray_resize(boxes, steps);

    for (int i = 0; i < steps; i++) {
        struct r_box *box = OArray_at(boxes, i, struct r_box);
        *box = r_box_new(1, 1);

        float t = (float) (i+1) / (float) steps;
        vec2 p = vec2_mix(a, b, t);

        box->rect = u_rect_new_center(p.x, p.y, C->stroke_size, C->stroke_size);
        box->fx = C->brush_color;
    }

    RTex_ro(C->canvas, C->ro);
}

static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    x_iui_init_lt_set(vec2_(48));

    C->canvas = RTex_new(view, NULL, IMAGE_SIZE.x, IMAGE_SIZE.y);
    RTex_clear_full(C->canvas, R_WHITE);

    C->ro = RObjBox_new_tex_rgba(view, 0, r_tex_white(), false, 1, 1);
    oobj boxes = RObjBox_boxes(C->ro);
    OArray_dyn_realloc_mode_set(RObjBox_boxes(C->ro), OArray_REALLOC_DOUBLED);
    OArray_realloc(boxes, 64, 0);

    C->brush_color.xyz = vec3_random();
    C->brush_color.a = 1.0f;
    C->stroke_size = 3.0f;
    
    //
    // create gui
    //

    C->theme = WTheme_new_tiny(view);

    C->gui = WAlign_new_center(view);

    oobj box = WBox_new_v(C->gui);
    WBox_spacing_set(box, vec2_(4));
    WObj_min_size_ref(box)->x = DISPLAY_SIZE.x;

    oobj canvas_frame = WFrame_new(box);
    C->canvas_area = WObj_new(canvas_frame);
    WObj_fixed_size_set(C->canvas_area, vec2_(m_2(DISPLAY_SIZE)));

    C->info = WText_new(WAlign_new_center_h(box), NULL);

    oobj progress_box = WBox_new_v(box);
    WBox_spacing_set(progress_box, vec2_(2));


    C->pressure = framed_progress(progress_box, "PRESSURE");

    oobj tilt_box = WBox_new(progress_box, WBox_H_WEIGHTS);
    WBox_weight_default_set(tilt_box, 1.0f);
    WBox_spacing_set(tilt_box, vec2_(2));
    C->tilt_x = framed_progress(tilt_box, "TILT_X");
    C->tilt_y = framed_progress(tilt_box, "TILT_Y");

    oobj dist_rot_box = WBox_new(progress_box, WBox_H_WEIGHTS);
    WBox_weight_default_set(dist_rot_box, 1.0f);
    WBox_spacing_set(dist_rot_box, vec2_(2));
    C->distance = framed_progress(dist_rot_box, "DISTANCE");
    C->rotation = framed_progress(dist_rot_box, "ROTATION");

    oobj slid_tpress_box = WBox_new(progress_box, WBox_H_WEIGHTS);
    WBox_weight_default_set(slid_tpress_box, 1.0f);
    WBox_spacing_set(slid_tpress_box, vec2_(2));
    C->slider = framed_progress(slid_tpress_box, "SLIDER");
    C->tangential_pressure = framed_progress(slid_tpress_box, "T_PRESSURE");
}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /**
     * Color and stroke size settings via x_iui floating windows
     */
    x_iui_rgba("BRUSH_COLOR", &C->brush_color);
    x_iui_float("STROKE_SIZE", &C->stroke_size, 1, 32);

    WTheme_update_full_tex(C->theme, C->gui, tex);

    struct a_pointer p = a_pointer(0, 0);

    /**
     * Additional pressure sensitive pen fields (apart from pressure, which is already part of a_pointer)
     */
    struct a_pointer_pen pen = a_pointer_pen(0, 0);

    char *info_text = o_strf_a("TOUCH: %s  PEN: %s\n"
                               "X:%5.1f Y:%5.1f BTNS: %i",
                               a_pointer_touch_used()? "YES" : "NO ",
                               a_pointer_pen_used()? "YES": "NO ",
                               p.pos.x, p.pos.y,
                               pen.state);
    WText_text_set(C->info, info_text);

    WProgress_progress_set(C->pressure, p.pressure);
    WProgress_progress_set(C->tilt_x, (90.0f + pen.tilt.x) / 180.0f);
    WProgress_progress_set(C->tilt_y, (90.0f + pen.tilt.y) / 180.0f);
    WProgress_progress_set(C->distance, pen.distance);
    WProgress_progress_set(C->rotation, (180.0f + pen.rotation) / 360.0f);
    WProgress_progress_set(C->slider, pen.slider);
    WProgress_progress_set(C->tangential_pressure, pen.tangential_pressure);

    mat4 canvas_pose = WObj_gen_pose(C->canvas_area);


    /**
     * To handle all occured pointer events (in between frames) and not only the current one, use the history parameter
     */
    for (osize i=a_pointer_history_num(0); i>0; i--) {
        p = a_pointer(0, i);

        p.pos = mat4_mul_vec(mat4_inv(canvas_pose), p.pos);
        p.pos.xy = vec2_scale_v(p.pos.xy, vec2_(m_2(IMAGE_SIZE)));

        if (p.down && u_rect_contains(vec4_(0, 0, m_2(IMAGE_SIZE)), p.pos.xy)) {
            if (C->prev.down) {
                draw_line(view, C->prev.pos.xy, p.pos.xy);
            } else {
                draw_line(view, p.pos.xy, p.pos.xy);
            }
        }
        C->prev = p;
    }

   

}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    const vec4 bg_color = vec4_cast_byte_1(bvec4_(200, 150, 100, 255).v);
    RTex_clear_full(tex, bg_color);

    WTheme_render(C->theme, tex);

    vec4 canvas_rect = WObj_gen_rect(C->canvas_area);
    RTex_blit(tex, C->canvas, canvas_rect.left, canvas_rect.top);
}


oobj ex_12_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}


/**
 * Summary:
 * This example demonstrated how easy it is to create a simple stupid drawing app with Mia.
 * It also shows which axles of a pressure pen are working.
 * Tip: use the virtual cursor while drawing on mobile ;)
 *      To do so, swipe from the side border into the app 
 *          and use another finger to press down.
 */
