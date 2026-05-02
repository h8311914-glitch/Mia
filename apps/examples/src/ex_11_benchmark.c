/**
 * Performance tests:
 * - DYN:   dynamical set render boxes on the (visible) screen
 * - GPU:   like dyn, but only a single time set, so performance of raw gpu without cpu
 * - GPU_V: vertex shader test, renders boxes hidden from the screen. so they get discarded after the vertex shader
 * - GPU_F: each box is rendered full screen, so a lot of overdrawing done by the fragment shader
 * - CPU:   cpu calculations without the need to load or write to the ram
 * - RAM:   mostly copies on ram
 * - ARRAY: tricks how to access array elements. Most visible in debug build.
 *          Release may or may not optimize some of the access overhead.
 */


#include "mia.h"

// A WSlider selects the amount of work to do.
// Calculated by 1 >> slinder_n
#define SLIDER_N_MAX 20

// if fps drops below that for MIN_FPS_RETRY times, slider will get reset
#define MIN_FPS_RESET 10.0
#define MIN_FPS_RETRY 3

// RAM mode copied one mega byte.
#define RAM_1MB (1024*1024)


enum testmode {
    TOGGLE_DYN,
    TOGGLE_GPU,
    TOGGLE_GPU_V,
    TOGGLE_GPU_F,
    TOGGLE_CPU,
    TOGGLE_RAM,
    TOGGLE_ARRAY,
    NUM_TOGGLES
};

static const char *testmode_names[] = {
    "DYN",
    "GPU",
    "GPU_V",
    "GPU_F",
    "CPU",
    "RAM",
    "ARRAY",
    NULL
};

static const vec4 testmode_colors[] = {
    {{0.8, 0, 0.8, 1}},
    {{0, 0, 0.8, 1}},
    {{0, 0, 0.7, 1}},
    {{0.1, 0.1, 0.7, 1}},
    {{0.8, 0, 0, 1}},
    {{0.8, 0.8, 0, 1}},
    {{0.0, 0.8, 0.8, 1}},
};

struct context {
    oobj ro;

    oobj theme;
    oobj gui;

    oobj testmodes[NUM_TOGGLES];
    bool ro_update;
    int current_mode;

    oobj tex_mode;
    oobj tex;

    oobj slider;
    oobj slider_lbl;

    oobj text;
    oobj text_result;

    int fps_retries;
    float fps_reset_info_time;
    
    double smooth[3];

    obyte *ram_data_a;
    obyte *ram_data_b;
};


// running avg smoothing for info results
static double smooth_result(double *in_out_smooth, double new_value) 
{
    if(md_isnan(new_value) || md_isinf(new_value)) {
        return *in_out_smooth;
    }
    double dt = a_dt();
    if(dt > 0.1) {
        // only apply smooth if >= 10 fps
        *in_out_smooth = new_value;
        return *in_out_smooth;
    }
    *in_out_smooth = md_mix(*in_out_smooth, new_value, 10.0*dt);
    return *in_out_smooth;
}


static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    C->ro = RObjBox_new(view, 1, "$ex/icon16.png", 1, 1);

    // blitting, so without blending overhead.
    RShader_blend_set(RObjBox_shader(C->ro, 0), false);


    C->theme = WTheme_new_tiny(view);
    C->gui = WWindow_new_title(view, vec2_(1), "OPTIONS", NULL);
    WWindow_lt_set(C->gui, vec2_(32));
    oobj box = WBox_new_v(WWindow_body(C->gui));
    WBox_spacing_set(box, vec2_(2));

    oobj togglebox = WBox_new(WAlign_new(box), WBox_H_V);
    WObj_min_size_ref(togglebox)->x = 96;
    WBox_spacing_set(togglebox, vec2_(2));

    for (int i = 0; i < NUM_TOGGLES; i++) {
        oobj btn = WBtn_new(togglebox);
        WBtn_color_set(btn, testmode_colors[i]);
        oobj txt = WText_new(btn, testmode_names[i]);
        if (i == TOGGLE_RAM) {
            WText_color_set(txt, R_BLACK);
        }
        C->testmodes[i] = btn;
    }
    C->current_mode = TOGGLE_DYN;
    WBtn_down_set(C->testmodes[C->current_mode], true);
    C->ro_update = true;

    C->tex_mode = WBtn_new(WAlign_new(box));
    WObj_padding_ref(C->tex_mode)->v1 = 8;
    WText_new(C->tex_mode, "RENDER TO TEX");
    C->tex = RTex_new(view, NULL, 512, 512);

    oobj slider_frame = WFrame_new_title_shadow(box, "NUM", NULL);
    oobj slider_box = WBox_new_h(slider_frame);
    WObj_padding_set(slider_box, vec4_(1));
    C->slider = WSlider_new(slider_box);
    WObj_min_size_ref(C->slider)->x = 96;
    C->slider_lbl = WTextShadow_new(slider_box, NULL);

    C->text = WTextShadow_new(box, NULL);
    C->text_result = WTextShadow_new(box, NULL);
    WObj_hide_set(C->text_result, true);
    WObj_padding_ref(C->text_result)->v1 = 8;

    C->ram_data_a = o_new(view, obyte, RAM_1MB);
    C->ram_data_b = o_new(view, obyte, RAM_1MB);
}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    ivec2 tex_size = RTex_size_int(WBtn_down(C->tex_mode) ? C->tex : tex);
    const char *reset_info = C->fps_reset_info_time > 0
                                 ? "\n\n!!! FPS < " O_TO_STRING(MIN_FPS_RESET) " -> RESET!!!"
                                 : "";

    char *txt = o_strf_a(
        "SIZE:%5i x%5i\n"
        "LOAD:%3.0f%%  +%3.0f%%  ->%3.0f%%\n"
        "L_MS:%3.0fms +%3.0fms ->%3.0fms\n"
        "FPS: %3.0f"
        "%s",
        m_2(tex_size),
        a_load_update() * 100,
        a_load_render() * 100,
        a_load() * 100,
        a_load_update_ms(),
        a_load_render_ms(),
        a_load_ms(),
        a_fps(),
        reset_info);

    WText_text_set(C->text, txt);

    WTheme_update_full_tex(C->theme, C->gui, tex);

    for (int i = 0; i < NUM_TOGGLES; i++) {
        if (WBtn_pressed(C->testmodes[i])) {
            C->current_mode = i;
            WBtn_set_group(C->testmodes[i], C->testmodes, NUM_TOGGLES, false);
            WSlider_progress_set(C->slider, 0);
            C->ro_update = true;
            WObj_hide_set(C->text_result, true);
            dvecn_set(C->smooth, 0, 3);
            break;
        }
    }

    if (WBtn_toggled(C->tex_mode)) {
        C->ro_update = true;
    }

    WObj_hide_set(C->tex_mode, C->current_mode>=TOGGLE_CPU);


    float progress = WSlider_progress(C->slider);
    int slider_n = m_round(progress * SLIDER_N_MAX);
    progress = (float) slider_n / SLIDER_N_MAX;
    WSlider_progress_set(C->slider, progress);

    osize slider_num = (osize) (1 << slider_n);
    char label[32];
    o_strf_buf(label, "%7i", (int) slider_num);
    WText_text_set(C->slider_lbl, label);


    if (o_num(C->ro) != slider_num
        || WBtn_down(C->testmodes[TOGGLE_DYN])) {
        C->ro_update = true;
    }

    if (C->ro_update) {
        oobj boxes = RObjBox_boxes(C->ro);

        OArray_resize(boxes, slider_num);

        vec2 size;
        if (WBtn_down(C->tex_mode)) {
            size = RTex_size(C->tex);
        } else {
            size = RCam_size(AView_cam(view));
        }
        vec2 field = vec2_sub(size, 16);

        struct r_box def = r_box_new(16, 16);
        struct r_box *b_arr = o_at(boxes, 0);

        if (C->current_mode == TOGGLE_DYN || C->current_mode == TOGGLE_GPU) {
            for (osize i = 0; i < slider_num; i++) {
                vec2 pos = vec2_random_range_v(vec2_(0), field);
                b_arr[i] = def;
                u_rect_lt_set(&b_arr[i].rect, m_2(pos));
            }
        }

        if (C->current_mode == TOGGLE_GPU_V) {
            for (osize i = 0; i < slider_num; i++) {
                vec2 pos = vec2_(9e9);
                b_arr[i] = def;
                u_rect_lt_set(&b_arr[i].rect, m_2(pos));
            }
        }

        if (C->current_mode == TOGGLE_GPU_F) {
            for (osize i = 0; i < slider_num; i++) {
                b_arr[i] = def;
                b_arr[i].rect = u_rect_new(0, 0, m_2(size));
            }
        }
    }


    if (C->current_mode == TOGGLE_CPU) {
        // Monte Carlo estimation of PI
        int inside_circle = 0;
        for (osize i = 0; i < slider_num; i++) {
            // Generate random x and y coordinates between -1.0 and 1.0
            vec2 pos = vec2_random_range(-1.0f, 1.0f);
            // Check if the point is within the unit circle
            if (vec2_dot(pos, pos) <= 1.0f) {
                inside_circle++;
            }
        }
        // Estimate PI using the ratio of points inside to total points
        double estimated_pi = (double) (4.0 * (double) inside_circle / slider_num);
        
        // smooth result with running avg
        estimated_pi = smooth_result(C->smooth, estimated_pi);

        // Display the results on the UI
        char *text = o_strf_a("PI: %.6f", estimated_pi);
        WText_text_set(C->text_result, text);
        WObj_hide_set(C->text_result, false);
    }

    if (C->current_mode == TOGGLE_RAM) {
        ou64 start = o_timer();
        for (osize i = 0; i < slider_num; i++) {
            o_memcpy(C->ram_data_a, C->ram_data_b, 1, RAM_1MB);

            // a compiler in release mode may optimize this "unnecessary" memory copy.
            // so we prevent him from doing this by passing the array pointers to a noop (no operation) function.
            // As the name suggest, the o_noop does nothing with the pointers, but the compiled is not aware of that.
            o_noop(o_list_compound(void*, C->ram_data_a, C->ram_data_b));
        }
        double time = o_timer_elapsed_s(start);

        // Display the results on the UI
        double gb_s = (double) slider_num / (1024.0 * time);
        
        // smooth result with running avg
        gb_s = smooth_result(C->smooth, gb_s);
        
        char *text = o_strf_a("MOVING: %"osize_PRI " MB\n"
                              " AT ~ : %.2f GB/S",
                              slider_num,
                              gb_s);
        WText_text_set(C->text_result, text);
        WObj_hide_set(C->text_result, false);
    }

    if (C->current_mode == TOGGLE_ARRAY) {
        oobj boxes = RObjBox_boxes(C->ro);
        OArray_resize(boxes, slider_num);

        float seed = m_random();

        const int repeat = 10;
        void *boxes_0 = o_at(boxes, 0);


        for (int r = 0; r < repeat; r++) {

            // easy but "slow"
            for (osize i = 0; i < o_num(boxes); i++) {
                struct r_box *b = o_at(boxes, i);
                b->rect.x = seed + i * i;
            }

            // prevent optimization
            o_noop(boxes_0);
        }

        ou64 timer = o_timer();
        for (int r = 0; r < repeat; r++) {

            // redoing easy but "slow" for accurate timings
            for (osize i = 0; i < o_num(boxes); i++) {
                struct r_box *b = o_at(boxes, i);
                b->rect.x = seed + i * i;
            }

            o_noop(boxes_0);
        }

        double time_easy = o_timer_elapsed_millis(timer);
        seed = m_random();
        timer = o_timer();

        for (int r = 0; r < repeat; r++) {

            // less abstraction overhead; inlined OArray_at instead of virtual o_at
            for (osize i = 0; i < OArray_num(boxes); i++) {
                struct r_box *b = OArray_at(boxes, i, struct r_box);
                b->rect.x = seed + i * i;
            }

            o_noop(boxes_0);
        }

        double time_less = o_timer_elapsed_millis(timer);
        seed = m_random();
        timer = o_timer();

        for (int r = 0; r < repeat; r++) {

            // fastest option
            // fetches num and the array once
            // does not work for all containers, but OArray is packed and works fine for that
            // o_at(,0) may be replaced with OArray_data (would be a negligible nano optimization)
            osize num = o_num(boxes);
            struct r_box *boxes_v = o_at(boxes, 0);
            for (osize i = 0; i < num; i++) {
                struct r_box *b = &boxes_v[i];
                b->rect.x = seed + i * i;
            }

            o_noop(boxes_0);
        }

        double time_fast = o_timer_elapsed_millis(timer);

        // smooth results with running avg
        
        time_easy = smooth_result(C->smooth, time_easy);
        time_less = smooth_result(C->smooth+1, time_less);
        time_fast = smooth_result(C->smooth+2, time_fast);
        

        // Display the results on the UI
        char *text = o_strf(view, "ARRAY ACCESS RESULTS:\n"
                                  "  EASY:%6.2f ms\n"
                                  "  LESS:%6.2f ms\n"
                                  "  FAST:%6.2f ms",
                                  time_easy,
                                  time_less,
                                  time_fast);
        WText_text_set(C->text_result, text);
        WObj_hide_set(C->text_result, false);
    }

    // in case the system slows down to much, reset...
    C->fps_reset_info_time = m_max(0, C->fps_reset_info_time-dt);
    if (a_fps() < MIN_FPS_RESET) {
        C->fps_retries++;
        o_log_warn("fps too low: %f", a_fps());
        if (C->fps_retries >= MIN_FPS_RETRY) {
            C->fps_retries = 0;
            C->fps_reset_info_time = 3.0f;
            o_log_warn("fps too low, resetting slider");
            WSlider_progress_set(C->slider, 0);
            C->ro_update = true;
        }
    } else {
        C->fps_retries = 0;
    }
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    bool gpu_stuff = C->current_mode<TOGGLE_CPU;
    if (gpu_stuff) {
        oobj ro_tex = WBtn_down(C->tex_mode) ? C->tex : tex;
        RTex_ro_ex(ro_tex, C->ro, NULL, C->ro_update);
        C->ro_update = false;
    }

    WTheme_render(C->theme, tex);
}


oobj ex_11_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_UNSAFE);
    return scene;
}


/**
 * Summary:
 * With this example we created a little benchmark tool for the Mia engine.
 * The various modes give you a hint on what is possible for a game.
 * For large datasets, we showed how to access an OArray using a for loop for maximal performance.
 */
