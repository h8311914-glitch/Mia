#include "mp/selection.h"
#ifdef MIA_OPTION_APP_MP_MAIN
#define MIA_MAIN
#include "o/common.h"
#endif

#include "mp/main.h"
#include "mia.h"

#include "mp/history.h"
#include "mp/canvas.h"
#include "mp/palette.h"
#include "mp/selection.h"
#include "mp/toolbar.h"


// should be 88 (16*5.5) for portrait and 56 (16*3.5) for landscape
//     .5 used to show the last row in half
#define PALETTE_SIZE_MAX_V (MP_PALETTE_DROP_SIZE*5.5f)
#define PALETTE_SIZE_MAX_H (MP_PALETTE_DROP_SIZE*3.5f)



static const vec4 DEFAULT_PALETTE[] = {
    {{0, 0, 0, 0}},
    {{0, 0, 0, 1.0}},
    {{0.5, 0.5, 0.5, 1.0}},
    {{1, 1, 1, 1}},
    {{1, 0, 0, 1}},
    {{1, 1, 0, 1}},
    {{0, 1, 0, 1}},
    {{0, 1, 1, 1}},
    {{0, 0, 1, 1}},
    {{1, 0, 1, 1}},
};

struct main_context {
    struct mp_options options;

    struct mp_history *history;
    struct mp_surface *surface;
    struct mp_selection *selection;
    oobj canvas;
    oobj palette;
    oobj split;

    struct mp_manager *manager;
    struct mp_toolbar *toolbar;
    
    oobj selected;

    bool cam_was_fit_to_selection;

};


static vec4 *load_palette_img(oobj parent, const char *file, int *out_num)
{
    struct o_img img = o_img_new_file(parent, file, o_img_RGBA);
    if (!img.data) {
        *out_num = 0;
        return NULL;
    }
    *out_num = o_img_num(img);
    vec4 *palette = o_new(parent, vec4, *out_num);
    for (int i=0; i<*out_num; i++) {
        palette[i] = vec4_cast_byte_1(o_img_at_idx(img, i));
    }
    o_img_free(&img);
    return palette;
}
static void setup(oobj view)
{
    struct main_context *C = o_user(view);


#if 0
    int colors_num = 128;
    vec4 *colors = o_new(a_tmp(), vec4, colors_num);
    for(int i=0; i<colors_num; i++) {
        colors[i] = vec4_random();
    }
#elif 0
    int colors_num = o_stack_array_num(DEFAULT_PALETTE);
    const vec4 *colors = DEFAULT_PALETTE;
#else
    int colors_num = 0;
    vec4 *colors = load_palette_img(view, "$mp/palette/pixilmatt.png", &colors_num);
#endif

    
    C->history = mp_history_new(view, 0, MP_HISTORY_AUTO_SAVE_PATH);
    C->surface = mp_surface_new(view, C->history);
    C->selection = mp_selection_new(view, C->history);
    C->canvas = mp_canvas_new(view, C->surface, C->selection, C->history);
    C->palette = mp_palette_new(view, C->surface, colors, colors_num);

    C->manager = mp_manager_new(view, C->history, C->selection, C->surface, C->canvas);
    C->toolbar = mp_toolbar_new(view, &C->options, C->manager);

    // load after modules add history
    mp_history_load(C->history, NULL);
    // initial history step
    mp_history_initial_commit(C->history);

    C->split = USplit_new(view, C->canvas, C->palette, USplit_V_BOTTOM, 64, 1);

     
    
    C->selected = RObjBox_new_rgba(view, 4, "$mp/selected.png", 2, 2);
    struct r_box *b = o_at(C->selected, 0);
    //b[0].rect.zw = b[1].rect.zw = b[2].rect.zw = vec2_(32);
    b[0].sprite.xy = vec2_(0, 0);
    b[1].sprite.xy = vec2_(1, 0);
    b[2].sprite.xy = vec2_(0, 1);
    b[3].sprite.xy = vec2_(1, 1);

}

static void update(oobj view, oobj tex, float dt)
{
    struct main_context *C = o_user(view);

    oobj cam = a_cam();
    vec2 size = RCam_size(cam);

    vec2 palette_needed = mp_palette_needed_size(C->palette);
    
    float palette_sep = RCam_is_portrait(cam)? PALETTE_SIZE_MAX_V : PALETTE_SIZE_MAX_H;
    palette_sep = o_min(palette_sep, palette_needed.y);
    
    // selected
    float selected_y = size.y - palette_sep - 18 - 1;
    struct r_box *b = o_at(C->selected, 0);
    b[0].rect.xy = b[2].rect.xy = vec2_(0, selected_y);
    b[1].rect.xy = b[3].rect.xy = vec2_(16, selected_y);
    b[2].fx = C->surface->color;
    b[3].fx = C->surface->color_2nd;

    vec4 p_rect = b[0].rect;
    p_rect.width = 24;

    // pointer 0 or pointer 1 (multitouch cursor fallthrough)
    for (int i=0; i<2; i++) {
        struct a_pointer p = a_pointer(i, 0);
        if(u_rect_contains(p_rect, p.pos.xy)) {
            if (a_pointer_pressed(p)) {
                // switch colors
                mp_surface_color_swap(C->surface);
                mp_palette_current_hide(C->palette);
            }
            a_pointer_handled(i, -1);
        }
    }

    
    USplit_seperator_set(C->split, palette_sep);

    mp_toolbar_update(C->toolbar);

    USplit_update(C->split, tex);

    mp_surface_update(C->surface);
    mp_manager_update(C->manager);
}

static void render(oobj view, oobj tex, float dt)
{
    struct main_context *C = o_user(view);
    USplit_render(C->split, tex);
    RObj_render(C->selected, tex);
    mp_toolbar_render(C->toolbar, tex);
    
}


//
// public
//

struct mp_options mp_options_default(void)
{
    struct mp_options ret = {0};
    ret.canvas_size = ivec2_(128);
    ret.palette = DEFAULT_PALETTE;
    ret.palette_num = o_stack_array_num(DEFAULT_PALETTE);
    return ret;
}

oobj mp_main(oobj root, struct mp_options *opt_options)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_UNSAFE);

    struct main_context *C = o_user_new0(view, *C, 1);
    C->options = opt_options? *opt_options : mp_options_default();
    // todo ref copy of palette?

    return scene;
}

oobj mp_main_splashed(oobj root, struct mp_options *opt_options)
{
    oobj view = AView_new(root, setup, update, render);
    oobj splash = u_splash_new_mia(root, view, true, AScene_UNSAFE,
                                   "HORSIMANN", "MIA PAINT", 2.0f, true);

    struct main_context *C = o_user_new0(view, *C, 1);
    C->options = opt_options? *opt_options : mp_options_default();
    // todo ref copy of palette?

    return splash;

}

#ifdef MIA_OPTION_APP_MP_MAIN
static void app_main(oobj root)
{
    mp_main_splashed(root, NULL);
}

int main(int argc, char **argv)
{
    struct a_app_run_options options = a_app_run_options_default();
    // options.log_level = O_LOG_TRACE;
    a_app_run(app_main, &options);
    return 0;
}
#endif
