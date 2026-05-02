#ifdef MIA_OPTION_APP_EX_MAIN
#define MIA_MAIN
#include "o/common.h"
#endif


#include "ex/main.h"
#include "o/img.h"
#include "o/log.h"
#include "o/str.h"
#include "m/utils/color.h"
#include "r/RTex.h"
#include "r/RCam.h"
#include "a/app.h"
#include "a/AScene.h"
#include "a/AViewStage.h"
#include "u/scroll.h"
#include "u/splash.h"
#include "w/WTheme.h"
#include "w/WBox.h"
#include "w/WBtn.h"
#include "w/WTextShadow.h"
#include "ex/EXViewOverlay.h"
#include "ex/tea.h"


#ifdef MIA_OPTION_APP_MP
#  include "mp/main.h"
#  define NUM_EXAMPLES 18
#else
#  define NUM_EXAMPLES 17
#endif

#define URL_HOST "https://github.com/renehorstmann/Roja/blob/main/apps/examples/src/"
#define URL_HOST_MP "https://github.com/renehorstmann/Roja/blob/main/apps/MiaPaint/"

// protected example functions
oobj ex_00_main(oobj root);
oobj ex_01_main(oobj root);
oobj ex_02_main(oobj root);
oobj ex_03_main(oobj root);
oobj ex_04_main(oobj root);
oobj ex_05_main(oobj root);
oobj ex_06_main(oobj root);
oobj ex_07_main(oobj root);
oobj ex_08_main(oobj root);
oobj ex_09_main(oobj root);
oobj ex_10_main(oobj root);
oobj ex_11_main(oobj root);
oobj ex_12_main(oobj root);
oobj ex_13_main(oobj root);
oobj ex_14_main(oobj root);

oobj ex_tea_main(oobj root);

oobj ex_thunder_main(oobj root);

#ifdef MIA_OPTION_APP_MP
static oobj start_mia_paint(oobj root)
{
    struct mp_options options = mp_options_default();
    options.show_exit_btn = true;
    return mp_main(root, &options);
}
#endif

static const char *example_titles[] = {
    "00_hello_world",
    "01_pointers",
    "02_animations",
    "03_iui",
    "04_widgets",
    "05_windows",
    "06_fonts",
    "07_sound",
    "08_xtras",
    "09_fetching",
    "10_upndownload",
    "11_benchmark",
    "12_drawing",
    "13_mixer",
    "14_webcam",
    "~ tea app",
    "~ thunder app",
#ifdef MIA_OPTION_APP_MP
    "~ mia paint",
#endif
};

// list all functions in an array
static oobj (*example_functions[])(oobj root) = {
    ex_00_main,
    ex_01_main,
    ex_02_main,
    ex_03_main,
    ex_04_main,
    ex_05_main,
    ex_06_main,
    ex_07_main,
    ex_08_main,
    ex_09_main,
    ex_10_main,
    ex_11_main,
    ex_12_main,
    ex_13_main,
    ex_14_main,
    ex_tea_main,
    ex_thunder_main,
#ifdef MIA_OPTION_APP_MP
    start_mia_paint,
#endif
};

// list all functions in an array
static const char *example_urls[] = {
    URL_HOST "ex_00_hello_world.c",
    URL_HOST "ex_01_pointers.c",
    URL_HOST "ex_02_animations.c",
    URL_HOST "ex_03_iui.c",
    URL_HOST "ex_04_widgets.c",
    URL_HOST "ex_05_windows.c",
    URL_HOST "ex_06_fonts.c",
    URL_HOST "ex_07_sound.c",
    URL_HOST "ex_08_xtras.c",
    URL_HOST "ex_09_fetching.c",
    URL_HOST "ex_10_upndownload.c",
    URL_HOST "ex_11_benchmark.c",
    URL_HOST "ex_12_drawing.c",
    URL_HOST "ex_13_mixer.c",
    URL_HOST "ex_14_webcam.c",
    URL_HOST "tea.c",
    URL_HOST "thunder.c",
#ifdef MIA_OPTION_APP_MP
    NULL,
#endif
};


static struct {
    oobj root;

    oobj example_scene;

    oobj theme;
    oobj gui;

    struct u_scroll scroll;

    oobj ex01btn;
} L;


static void close_event(oobj close_view)
{
    AScene_exit(L.example_scene);
}

static void view_del(oobj view)
{
    OObj_del(a_scene());
}

static void start_example(oobj btn)
{
    int *idx = o_user(btn);

    o_log_s(__func__, "%i", *idx);

    // starting the example
    L.example_scene = example_functions[*idx](L.root);

    const char *url = example_urls[*idx];

    if (url) {
        // creating a transparent AScene with an exit button at top left and open source link on top right
        oobj close_view = EXViewOverlay_new(L.root, close_event, url);
        oobj close_scene = AScene_new(L.root, close_view, true, AScene_SAFE);
        AScene_opaque_set(close_scene, false);
        AScene_escape_event_set(close_scene, close_event);
    }
}


static void setup(oobj view)
{
    // window title + icon
    AScene_title_set(a_scene(), "Mia Examples");
    AScene_icon_set_file(a_scene(), "$ex/icon16.png");

    L.root = OObj_new(view);
    L.theme = WTheme_new_tiny(view);

    L.gui = WBox_new_v(view);
    WBox_spacing_set(L.gui, vec2_(4));

    oobj title = WTextShadow_new(L.gui, "EXAMPLES:");
    WText_char_scale_set(title, vec2_(2, 3));
    WObj_padding_set(title, vec4_(0, 0, 0, 16));


    float hue = 0.33;
    for (int i = 0; i < NUM_EXAMPLES; i++) {
        oobj btn = WBtn_new(L.gui);
        if (i == 0) {
            L.ex01btn = btn;
        }
        int *idx = o_user_new0(btn, int, 1);
        *idx = i;
        WBtn_auto_mode_set(btn, WBtn_auto_CLICKED);
        WBtn_auto_event_set(btn, start_example);
        oobj example_text = WText_new(btn, example_titles[i]);
        WText_casing_set(example_text, WText_casing_UPPER);

        vec4 hsva = vec4_(hue, 0.3, 0.5, 1.0);
        hue = m_mod(hue + 0.1, 1.0);
        vec4 rgba = vec4_hsv2rgb(hsva);
        WBtn_color_set(btn, rgba);
    }

    L.scroll = u_scroll_new(u_scroll_Y, AView_cam(view));
}

static void update(oobj view, oobj tex, float dt)
{
    // update scroll before WTheme widget stuff, so it's able to use the active unhandled pointer
    u_scroll_update(&L.scroll, dt);

    vec2 center = RCam_center(AView_cam(view));
    vec2 lt = vec2_(center.x-80, 0);
    WTheme_update(L.theme, L.gui, lt, vec2_(160));

    vec2 gui_size = WObj_gen_size(L.gui);
    L.scroll.cam_limits_rect = u_rect_new(0, -32, 0, gui_size.y + 2 * 32);
}

static void render(oobj view, oobj tex, float dt)
{
    RTex_clear_full(tex, vec4_(0.294118, 0.164706, 0.164706, 1));

    WTheme_render(L.theme, tex);
}


static void ex_init(oobj root)
{
    o_clear(&L, sizeof L, 1);
}

oobj ex_main(oobj root)
{
    ex_init(root);
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_UNSAFE);
    return scene;
}

oobj ex_main_splashed(oobj root)
{
    ex_init(root);
    oobj view = AView_new(root, setup, update, render);
    oobj splash = u_splash_new_mia(root, view, true, AScene_UNSAFE,
                                   "HORSIMANN", "EXAMPLES", 2.0f, true);
    return splash;
}

oobj ex_root(void)
{
    return L.root;
}


#ifdef MIA_OPTION_APP_EX_MAIN
static void app_main(oobj root)
{

#  ifdef MIA_OPTION_APP_EX_MAIN_TEA
    ex_tea_main_splashed(root);
#  else
    // install terminal, toast and virtual cursor with default options
    x_install();
    ex_main_splashed(root);
#  endif
}

int main(int argc, char **argv)
{
    struct a_app_run_options options = a_app_run_options_default();

#  ifndef MIA_OPTION_APP_EX_MAIN_TEA
    options.mic_enable = true;
#endif

    // options.log_level = O_LOG_TRACE;
    a_app_run(app_main, &options);
    return 0;
}
#endif
