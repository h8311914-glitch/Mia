#include "x/terminal.h"
#include "o/OStream.h"
#include "o/file.h"
#include "o/OMap.h"
#include "o/ODict.h"
#include "o/str.h"
#include "r/RObjBox.h"
#include "r/RCam.h"
#include "a/AView.h"
#include "a/app.h"
#include "a/AViewStage.h"
#include "u/USplit.h"
#include "w/WStyle.h"
#include "x/cursor.h"
#include "x/viewtex.h"
#include "x/XViewKeys.h"
#include "x/XViewShell.h"
#include "x/XViewLog.h"

#ifdef MIA_OPTION_APP_MP
#  include "mp/main.h"
#endif

#include "x/iui.h"

#undef O_LOG_LIB
#define O_LOG_LIB "x"
#include "o/log.h"


#include "x/XViewKeys.h"


#define TERMINAL_POSITION_SPEED 0.75
#define CURSOR_BLINK_TIME 1.0f

static struct {
    bool installed;

    struct x_terminal_options options;
    
    oobj view_log;
    oobj view_shell;
    oobj view_keys;

    oobj split_shell;
    oobj split;

    bool active;

    // in [0:1]
    float position;

    float moving_start_y;
    float moving_start_position;
    
    oobj env_dict;

} terminal_L;


static void shell_api_exit(oobj shell, oobj user_obj, int argc, char **argv)
{
    x_terminal_close();
}

static void shell_api_clear(oobj shell, oobj user_obj, int argc, char **argv)
{
    XViewShell_print(shell, o_str_rep_a("\n", X_TERMINAL_ROWS, NULL));
}

static void shell_api_logf(oobj shell, oobj user_obj, int argc, char **argv)
{
    if(argc <= 1) {
        XViewLog_filter_set(terminal_L.view_log, NULL);
        XViewShell_print(shell, "LOG FILTER OFF");
        return;
    }
    char *filter = o_str_join_a(argv+1, -1, NULL);
    XViewLog_filter_set(terminal_L.view_log, filter);
    char *msg = o_strf_a("LOG FILTER SET: \"%s\"", XViewLog_filter(terminal_L.view_log));
}

static void shell_api_ls(oobj shell, oobj user_obj, int argc, char **argv)
{
    if (argc <= 1) {
        XViewShell_print(shell, "USAGE: LS <DIR>");
        return;
    }
    char *dir = o_str_join_a(argv+1, -1, NULL);
    oobj files = o_file_list(a_tmp(), dir, 0, NULL);
    char *text = "";
    int cnt = 0;
    for (osize i=0; i<o_num(files); i++) {
        char *file = o_at(files, i);
        if (*file == '.') {
            continue;
        }
        if (cnt < 2) {
            text = o_strf_a("%s  %s", text, file);
            cnt++;
        } else {
            text = o_strf_a("%s\n  %s", text, file);
            cnt = 0;
        }
    }
    XViewShell_print(shell, text);
}

static void shell_api_env(oobj shell, oobj user_obj, int argc, char **argv)
{
    if(argc <= 1) {
        osize num = o_num(terminal_L.env_dict);
        if(num <= 0) {
            XViewShell_print(shell, "NO KEYS AVAILABLE!");
            return;
        }
        XViewShell_print(shell, "AVAILABLE KEYS:");
        for(osize i=0;i<num; i++) {
            const char *key = ODict_key_at(terminal_L.env_dict, i);
            const char *val = ODict_value_at(terminal_L.env_dict, i);
            XViewShell_print(shell, o_strf_a("%s: %s", key, val));
        }
        return;
    }
    
    if(argc == 2) {
        const char *val = x_terminal_env(argv[1]);
        if(!val) {
            XViewShell_print(shell, "ERROR: KEY NOT FOUND");
            return;
        }
        XViewShell_print(shell, o_strf_a("%s: %s", argv[1], val));
        return;
    }
    
    char *val = o_str_join_a(argv+2, argc-2, " ");
    x_terminal_env_set(argv[1], val);
}

static void shell_api_envrm(oobj shell, oobj user_obj, int argc, char **argv)
{
    if(argc <= 1) {
        osize num = o_num(terminal_L.env_dict);
        if(num <= 0) {
            XViewShell_print(shell, "NO KEYS AVAILABLE!");
            return;
        }
        XViewShell_print(shell, "AVAILABLE KEYS:");
        for(osize i=0;i<num; i++) {
            const char *key = ODict_key_at(terminal_L.env_dict, i);
            const char *val = ODict_value_at(terminal_L.env_dict, i);
            XViewShell_print(shell, o_strf_a("%s: %s", key, val));
        }
        return;
    }
    x_terminal_env_set(argv[1], NULL);
}

// for both env and envrm
bool shell_env_autocomplete(oobj shell, oobj user_obj, OList *completions, int argc, char **argv)
{
    osize num = o_num(terminal_L.env_dict);
    for(osize i=0;i<num; i++) {
        const char *key = ODict_key_at(terminal_L.env_dict, i);
        OList_push(completions, (char *) key);
    }
    return true;
}


static void shell_api_viewtex(oobj shell, oobj user_obj, int argc, char **argv)
{
    if (argc <= 1) {
        XViewShell_print(shell, "USAGE: VIEWTEX <TEX>");
        return;
    }
    char *file = o_str_join_a(argv+1, -1, NULL);
    oobj tex = RTex_new_file(a_tmp(), file);
    if (RTex_size_int(tex).x == 0) {
        XViewShell_print(shell, o_strf_a("FAILED TO LOAD: \"%s\"", file));
        return;
    }
    XViewShell_print(shell, o_strf_a("OPENING TEXTURE: \"%s\"", file));
    x_viewtex_scene(user_obj, tex, true, "viewtex", NULL);
    x_terminal_close();
}


#ifdef MIA_OPTION_APP_MP
static void shell_api_mia_paint(oobj shell, oobj user_obj, int argc, char **argv)
{
    char *msg = "OPENING PAINT";
    char *file = NULL;
    oobj tex = NULL;
    if (argc >= 2) {
        file = o_str_join_a(argv+1, -1, NULL);
        tex = RTex_new_file(a_tmp(), file);
        if (RTex_size_int(tex).x == 0) {
            XViewShell_print(shell, o_strf_a("FAILED TO LOAD: \"%s\"", file));
            return;
        }
        msg = o_strf_a("OPENING PAINT: \"%s\"", file);
    }

    XViewShell_print(shell, msg);
    struct mp_options options = mp_options_default();
    options.opt_save_path = file;
    options.opt_canvas_init_tex = tex;
    if (tex) {
        options.canvas_size = RTex_size_int(tex);
    }
    options.show_exit_btn = true;
    mp_main(user_obj, &options);
    x_terminal_close();
}
#endif







static void terminal_setup(oobj view)
{
    terminal_L.view_log = XViewLog_new(view);
    terminal_L.view_keys = XViewKeys_new_terminal(view, NULL, NULL);
    oobj stream = XViewKeys_stream(terminal_L.view_keys);

    oobj shell = XViewShell_new(view, stream, X_TERMINAL_COLS, X_TERMINAL_ROWS);
    terminal_L.view_shell = shell;
    
    terminal_L.split_shell = USplit_new( view,
            terminal_L.view_shell,
            terminal_L.view_keys,
            USplit_V_TOP, -0.5, 0);
            
    terminal_L.split = USplit_new( view,
            terminal_L.view_log,
            terminal_L.split_shell,
            USplit_V_TOP, -0.5, 0);
            
            
    XViewShell_api_add(shell, shell, "exit", shell_api_exit, NULL,
        "CLOSES THE TERMINAL");
    XViewShell_api_add(shell, shell, "by", shell_api_exit, NULL,
        "ALIAS TO EXIT\nCLOSES THE TERMINAL");
    XViewShell_api_add(shell, shell, "clear", shell_api_clear, NULL,
        "CLEARS THE TERMINAL SHELL");
    XViewShell_api_add(shell, shell, "logf", shell_api_logf, NULL,
        "SETS A LOG FILTER\n"
        "LOGF <FILTER> -> LOG FILTER SET\n"
        "LOGF -> FILTER OFF");
    XViewShell_api_add(shell, shell, "ls", shell_api_ls, XViewShell_helper_api_ls_autocomplete,
        "LS <DIR> LISTS FILES IN A DIR");
    XViewShell_api_add(shell, shell, "env", shell_api_env, shell_env_autocomplete,
        "ENV -> LIST ALL KEY-VARS\n"
        "ENV <KEY> -> SHOW KEY-VAR\n"
        "ENV <KEY> <VAR> -> SET IT");
    XViewShell_api_add(shell, shell, "envrm", shell_api_envrm, shell_env_autocomplete,
        "ENVRM -> LIST ALL KEY-VARS\n"
        "ENVRM <KEY> -> Remove KEY-VAR");
    XViewShell_api_add(shell, shell,  "viewtex", shell_api_viewtex, XViewShell_helper_api_ls_autocomplete,
        "VIEWTEX <TEX_FILE> -> SHOW TEXTURE FILE\n"
        "VIEWTEX $logo16.png -> SHOW res/logo16.png");

#ifdef MIA_OPTION_APP_MP
    XViewShell_api_add(shell, shell, "paint", shell_api_mia_paint, XViewShell_helper_api_ls_autocomplete,
        "PAINT -> OPEN MIA PAINT\n"
        "PAINT <IMAGE_FILE> -> EDIT GIVEN IMAGE\n"
        "PAINT $logo16.png -> EDIT res/logo16.png");
#endif

    XViewShell_print(shell, " ~ MIA TERMINAL ~ \n");
}

static void terminal_update(oobj view, oobj tex, float dt)
{
    if(!terminal_L.options.enable) {
        x_terminal_close();
        return;
    }
    
    bool grabbing = false;
    float grab = NAN;

    struct a_pointer p = a_pointer(0, 0);
    oobj cam = AView_cam(view);
    vec2 cam_size = RCam_size(cam);

    if (a_pointer_pressed(p)) {
        if (!terminal_L.active) {
            if (p.pos.y >= cam_size.y-terminal_L.options.start_border) {
                terminal_L.moving_start_y = p.pos.y;
                terminal_L.moving_start_position = 0;
            }
        } else {
            float pos_y = 1.0f - p.pos.y / cam_size.y;
            bool in_keys;
            if (RCam_is_portrait(cam)) {
                in_keys = pos_y <= terminal_L.position - 0.75;
            } else {
                float pos_x = p.pos.x / cam_size.x;
                in_keys = pos_y <= terminal_L.position - 0.5 && pos_x >= 0.5;
            }
            if (!in_keys && pos_y <= terminal_L.position) {
                terminal_L.moving_start_y = p.pos.y;
                terminal_L.moving_start_position = terminal_L.position;
            }
        }
    }
    if (!p.down) {
        terminal_L.moving_start_y = -1;
    } else if (terminal_L.moving_start_y>=0) {
        if (!terminal_L.active) {
            float dist = terminal_L.moving_start_y - p.pos.y;
            if (dist >= terminal_L.options.start_distance) {
                grab = dist - terminal_L.options.start_distance;
                grabbing = true;
            }
        } else {
            grab = terminal_L.moving_start_y - p.pos.y;
            grabbing = true;
        }
    }

    if (!grabbing) {
        if (terminal_L.position<0.25) {
            terminal_L.position -= TERMINAL_POSITION_SPEED*dt;
        } else if (terminal_L.position<0.5) {
            terminal_L.position += TERMINAL_POSITION_SPEED*dt;
            terminal_L.position = m_min(terminal_L.position, 0.5);
        } else if (terminal_L.position<0.75) {
            terminal_L.position -= TERMINAL_POSITION_SPEED*dt;
            terminal_L.position = m_max(terminal_L.position, 0.5);
        } else {
            terminal_L.position += TERMINAL_POSITION_SPEED*dt;
        }
    } else {
        grab /= cam_size.y;
        terminal_L.position = terminal_L.moving_start_position + grab;
    }


    terminal_L.position = m_clamp(terminal_L.position, 0, 1);
    terminal_L.active = terminal_L.position>0.001;

    if (!terminal_L.active) {
        return;
    }

    // log color is full transparent up to position of 50%
    // beyond that its [0% : 75%]
    vec4 logs_bg = vec4_(0, 0, 0, m_max(0, terminal_L.position-0.5)*2*0.75);
    XViewLog_bg_color_set(terminal_L.view_log, logs_bg);

    // portrait: All below each other
    // landscape: Left logs, right shell out and keys
    bool portrait = RCam_is_portrait(AView_cam(view));
    USplit_mode_set(terminal_L.split, portrait? USplit_V_TOP : USplit_H_LEFT);

    ivec4 viewport = RTex_viewport(tex);
    viewport.v1 += viewport.v3 * (1.0f-terminal_L.position);
    USplit_update_ex(terminal_L.split, tex, viewport);
    
    if(terminal_L.position > 0.6) {
        a_pointer_handled(-1, -1);
    }
}

static void terminal_render(oobj view, oobj tex, float dt)
{
    if (!terminal_L.active) {
        return;
    }
    USplit_render(terminal_L.split, tex);
}

//
// public
//

struct x_terminal_options x_terminal_options_default(void)
{
    struct x_terminal_options opt = {0};
    opt.enable = true;
    opt.start_border = 8.0f;
    opt.start_distance = 32.0f;
    return opt;
}

void x_terminal_install(const struct x_terminal_options *opt_options)
{
    if(terminal_L.installed) {
        o_log_s(__func__, "already installed");
        return;
    }
    if (x_cursor_installed()) {
        o_log_warn_s(__func__, "Consider installing cursor afterwards to let it work with the terminal");
    }

    terminal_L.installed = true;
    if (opt_options) {
        terminal_L.options = *opt_options;
    } else {
        terminal_L.options = x_terminal_options_default();
    }

    o_log_ring_enabled_set(true);

    oobj view = a_app_main_view();
    OObj_assert(view, AViewStage);
    struct AView_layer layer = {terminal_setup, terminal_update, terminal_render};
    AView_push_layer_deferred(view, layer, true);


    terminal_L.env_dict = ODict_new(view, 256);
    OMap_remove_able_set(ODict_map(terminal_L.env_dict), true);
}

bool x_terminal_installed(void)
{
    return terminal_L.installed;
}

struct x_terminal_options *x_terminal_options_ref(void)
{
    assert(terminal_L.installed && "not installed?");
    return &terminal_L.options;
}

void x_terminal_open(bool full)
{
    if(!terminal_L.installed) {
        return;
    }
    terminal_L.position = full? 1 : 0.5;
    terminal_L.active = true;
}

void x_terminal_close(void)
{
    if(!terminal_L.installed) {
        return;
    }
    terminal_L.position = 0;
    terminal_L.active = false;
}

struct oobj_opt x_terminal_shell(void)
{
    return oobj_opt(terminal_L.view_shell);
}


const char *x_terminal_env(const char *key)
{
    if(!terminal_L.installed) {
        return NULL;
    }
    return ODict_get(terminal_L.env_dict, key);
}

bool x_terminal_env_equals(const char *key, const char *equals)
{
    return o_str_equals(x_terminal_env(key), equals);
}

const char *x_terminal_env_set(const char *key, const char *value)
{
    if(!terminal_L.installed) {
        return NULL;
    }
    if(value) {
        ODict_set(terminal_L.env_dict, key, o_str_clone(terminal_L.env_dict, value));
    } else {
        char *val = ODict_get(terminal_L.env_dict, key);
        if(val) {
            o_free(terminal_L.env_dict, val);
            ODict_remove(terminal_L.env_dict, key);
        }
    }
    return x_terminal_env(key);
}
