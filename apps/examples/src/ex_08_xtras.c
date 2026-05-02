/**
 * Stuff from module X and co.
 * What's missing here are the iui features including XWObjColor,
 * which were already shown in ex_03_iui.c
 */


#include "mia.h"

const static vec4 DEFAULT_COLOR = {{0.4, 0.4, 0.5, 1.0}};

struct context {
    
    /**
     * This example is scrollable
     */
    struct u_scroll scroll;

    /**
     * Widget theme and gui
     */
    oobj theme;
    oobj gui;

    /**
     * WText info of the viewfiles btn
     */
    oobj files_info;

    /**
     * WText info of the rumble btn
     */
    oobj rumble_info;

    /**
     * WText to show current HELLO env value
     */
    oobj env_text;

    /**
     * WText to show gamepad state
     */
    oobj gamepad_text;

    /**
     * Background / clear color, set terminal api
     */
    vec4 clear_color;
};


/**
 * This function opens a scene to view an RTex for debugging, etc.
 */
static void viewtex(oobj btn)
{
    /**
     * We do some image manipulation here.
     * All stuff will be allocated on the container
     */
    oobj container = OObj_new(btn);

    /**
     * Load the candle sprite sheet from ex_02_animations.c.
     * Create a resulting empty texture of 6x size.
     * Clear that with a random color.
     * Blend the candle with 4x size on top of that.
     * Gauss blur the resulting tex.
     */
    oobj candle = RTex_new_file(container, "$ex/candle.png");
    vec2 size = RTex_size(candle);
    vec2 scale_size = vec2_scale(size, 4);
    vec2 tex_size = vec2_scale(size, 6);
    oobj tex = RTex_new(container, NULL, m_2(tex_size));
    RTex_clear_full(tex, vec4_random());

    mat4 pose = u_pose_new(0, 0, m_2(scale_size));
    mat4 uv = RTex_pose(candle, 0, 0);
    RTex_blend_pose(tex, candle, pose, uv);

    tex = RTex_gauss(tex, ivec2_(9), vec2_(-1), RTex_format(tex));

    /**
     * Starting the XViewTex AView in a new scene to show the resulting image
     * We move tex outside the container, so the following o_del does not delete it.
     */
    oobj scene = x_viewtex_scene(btn, tex, true, "CANDLE", NULL);


    o_del(container);

    /**
     * Get XViewTex (AView) from the created AScene
     */
    oobj viewtex = AScene_view(scene);

    /**
     * gui is the WObj based gui to add widgets to
     */
    oobj gui = XViewTex_gui(viewtex);

    const char *info_text;
    if (a_pointer_touch_used()) {
        /**
         * Touch input -> multitouch to zoom and shift the image
         */
        info_text = "PINCH TO ZOOM";
    } else {
        /**
         * default cursor input -> mouse wheel to zoom and hold to shift
         */
        info_text = "WHEEL TO ZOOM";
    }

    /**
     * Injects the info text into the XViewTex
     */
    WText_new(gui, info_text);

    /**
     * Saving the RTex to disk.
     * This function is called before the scene is started!
     * Which will be happen in the next frame...
     * The XViewTex scene also has save button, which would save as "#XViewTex__CANDLE.png"
     */
    RTex_write_file(tex, "#ex_07_xtras_candle.png");
}

/**
 * This function is called when the XViewFiles scene finishes
 */
static void viewfiles_event(oobj scene)
{
    struct context *C = o_user(scene);

    oobj viewfiles = AScene_view(scene);
    if (XViewFiles_state(viewfiles) != XViewFiles_OK) {
        WText_text_set(C->files_info, "CANCELED");
        return;
    }

    /**
     * To get the absolute path, use XViewFiles_file() instead
     */
    const char *file = XViewFiles_file_name(viewfiles);
    WText_text_set(C->files_info, file);
}


/**
 * This function opens a file select scene (like a file dialog)
 */
static void viewfiles(oobj btn)
{
    oobj view = a_view();
    oobj scene = x_viewfiles_scene(view, "SELECT A FILE", viewfiles_event, viewfiles_event);
    o_user_set(scene, o_user(view));
}

/**
 * This function tries to open a url web link in the system browser.
 * may return false if failed, but that's not a guarantee.
 */
static void openurl(oobj btn)
{
    o_utils_open_url("https://github.com/renehorstmann/Mia");
}

/**
 * This function creates a toast text message for 2 seconds
 */
static void showtoast(oobj btn)
{
    x_toast_text(btn, "HELLO WORLD", 2.0f);
}

/**
 * This function creates an info dialog
 */
static void showdialog(oobj btn)
{
    x_dialog_info(btn, "HELLO WORLD");
}

/**
 * This function starts a rumble effect if possible.
 * intensity 1.0 [0:1]
 * time 0.5 [seconds]
 */
static void rumble(oobj btn)
{
    a_rumble(1.0, 0.5);
}


/**
 * This function is called if COLOR (or color) is called in the mia shell terminal.
 * Swipe up from the bottom border to open logs and another swipe up for the full terminal.
 */
static void api_color(oobj shell, oobj user_obj, int argc, char **argv)
{
    struct context *C = o_user(user_obj);
    if (argc != 2) {
        XViewShell_print(shell, "SETTING COLOR: DEFAULT");
        C->clear_color = DEFAULT_COLOR;
        return;
    }

    char *color = o_str_toupper_a(argv[1]);
    if (o_str_equals(color, "RED")) {
        C->clear_color = R_RED;
    } else if (o_str_equals(color, "GREEN")) {
        C->clear_color = R_GREEN;
    } else if (o_str_equals(color, "BLUE")) {
        C->clear_color = R_BLUE;
    } else if (o_str_equals(color, "YELLOW")) {
        C->clear_color = R_YELLOW;
    } else if (o_str_equals(color, "MAGENTA")) {
        C->clear_color = R_MAGENTA;
    } else if (o_str_equals(color, "CYAN")) {
        C->clear_color = R_CYAN;
    } else {
        XViewShell_print(shell, o_strf_a("USAGE: %s <COLOR>", argv[0]));
        return;
    }

    XViewShell_print(shell, o_strf_a("SETTING COLOR: %s", color));
}


/**
 * This is an optional helper function for the api_color shell api.
 * If present, it will get called to get a list of possible arguments, for auto completion using TAB.
 */
bool api_color_autocomplete(oobj shell, oobj user_obj, OList *completions, int argc, char **argv)
{
    /**
     * OList is newly created (so cleared) before calling
     * The strings pushed into the list are clones after this fn call.
     */
    OList_push(completions, "RED");

    /**
     * We can also use the list to allocate on.
     * Will get o_del'ed after the call.
     *
     * Isn't that a cool Mia feature :D
     * Both just run fine :)
     */
    OList_push(completions, o_str_clone(completions, "GREEN"));
    
    /** 
     * Or append a compound list in a single call
     */
    OList_append(completions, o_list_compound(void *, 
            "BLUE", "YELLOW", "MAGENTA", "CYAN"
    ), -1);
    
    /**
     * This example terminal api has a simple constant list of arguments.
     * So we return true to let the terminal know: "no need to call again"
     * Other apis like "ls" may need to be called again (nestee trees) (return false).
     * btw: the completions list is expected to list all arguments in order.
     *      the argv vector contains the current input.
     */
    return true;
}

/**
 * Helper to create some framed WBtn + WText fields
 */
static void add_framed_btn(oobj parent, const char *btn_label, const char *info_text, OObj__event_fn event,
                           oobj *opt_out_info_text)
{
    oobj frame = WFrame_new(parent);
    oobj box = WBox_new_h(frame);
    WObj_padding_set(box, vec4_(4));
    oobj btn = WBtn_new(box);
    WBtn_auto_event_set(btn, event);
    WBtn_auto_mode_set(btn, WBtn_auto_CLICKED);
    oobj label = WText_new(btn, btn_label);
    WObj_padding_set(label, vec4_(1));
    oobj info = WTextShadow_new(box, info_text);
    WObj_padding_set(info, vec4_(4, 4, 0));

    /**
     * o_opt_set is a simple macro that checks if opt_out_info_text!=NULL,
     *     and if so, sets it as *opt_out_info_text=info
     */
    o_opt_set(opt_out_info_text, info);
}

//
// view functions
//

static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);


    C->theme = WTheme_new_tiny(view);
    C->gui = WBox_new_v(view);
    WBox_spacing_set(C->gui, vec2_(4));

    oobj box = C->gui;

    WTextShadow_new(box, "STUFF FROM MODULE X AND CO");


    add_framed_btn(box, "VIEWTEX", "VIEW AN IMAGE", viewtex, NULL);
    add_framed_btn(box, "VIEWFILES", "SELECT A FILE", viewfiles, &C->files_info);
    add_framed_btn(box, "OPEN URL", "TO GITHUB MIA", openurl, NULL);
    add_framed_btn(box, "TOAST", "SHOW TOAST", showtoast, NULL);
    add_framed_btn(box, "DIALOG", "SHOW DIALOG", showdialog, NULL);
    add_framed_btn(box, "RUMBLE", "", rumble, &C->rumble_info);

    oobj gamepad_frame = WFrame_new(box);
    C->gamepad_text = WTextShadow_new(gamepad_frame, "");
    WObj_padding_set(C->gamepad_text, vec4_(4));

    /**
     * Virtual cursor, installed in the main.c file.
     * Swipe from the side borders to the center of the screen for a multitouch cursor.
     * Toggle start on the gamepad (if available) to show the cursor.
     *   And press the south button (A on some gamepads) to press down the pointer
     * Terminal, installed in the main.c file.
     * Swipe up from the bottom border to the center twice to open it.
     */
    const char *info = "VIRTUAL CURSOR:\n"
    "SWIPE FROM LEFT OR RIGHT BORDER\n"
    "  INTO THE APP\n"
    "USE ANOTHER FINGER\n"
    "  TO PRESS THE CURSOR DOWN\n\n"
#ifdef MIA_OPTION_GAMEPAD
    "IF USING A GAMEPAD,\n"
    "  PRESS START TO TOGGLE THE CURSOR\n\n"
#endif
    "SWIPE UP FOR LOGGING\n"
    "SWIPE AGAIN TO OPEN THE TERMINAL\n"
    "IN THE TERMINAL, TRY COMMANDS:\n"
    "$ COLOR <>\n"
    "$ ENV HELLO <>"
    ;

    WTextShadow_new(box, info);

    C->env_text = WTextShadow_new(box, "");


    C->clear_color = DEFAULT_COLOR;

    /**
     * Adds a new api to the terminal shell
     */
    oobj shell = x_terminal_shell().o;
    if(shell) {
        XViewShell_api_add(shell, view, "color", 
            api_color, api_color_autocomplete,
            "USAGE: COLOR <COLOR>\n"
            "ONE OF RED GREEN BLUE YELLOW MAGENTA CYAN\n"
            "OR JUST CALL \"COLOR\" FOR DEFAULT");
    }
        
    /**
     * Adds a new env variablr to the terminal
     * Set with terminal api cmd env:
     * $ ENV HELLO MIA
     */
    x_terminal_env_set("HELLO", "WORLD");
      
    C->scroll = u_scroll_new(u_scroll_Y, AView_cam(view));
}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    vec2 size = WTheme_update(C->theme, C->gui, vec2_(32, 4), vec2_(0));
    
    /**
     * Set scroll limits to used gui size
     */
    C->scroll.cam_limits_rect = u_rect_new(0, -32, 0, size.y + 64);
    u_scroll_update(&C->scroll, dt);

    WText_text_set(C->rumble_info, o_strf_a("AVAILABLE: %s", a_rumble_available()? "YES" : "NO"));


#ifdef MIA_OPTION_GAMEPAD
    // This call is always fine to do, but will return false if not MIA_OPTION_GAMEPAD
    WText_text_set(C->gamepad_text, a_input_gamepad_available()? "GAMEPAD AVAILABLE" : "GAMEPAD NOT AVAILABLE");
#else
    WText_text_set(C->gamepad_text, "GAMEPAD NOT BUILT");
#endif

    /**
     * Get the string of an env field set by the terminal, or NULL if not set
     */
    const char *env = x_terminal_env("HELLO");

    /**
     * o_or is a simple macro that returns env if its not NULL, else "(NOT SET)"
     */
    WText_text_set(C->env_text, o_strf_a("ENV HELLO: %s", o_or(env, "(NOT SET)")));
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    RTex_clear_full(tex, C->clear_color);
    WTheme_render(C->theme, tex);
}


oobj ex_08_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_UNSAFE);
    return scene;
}

/**
 * Summary:
 *
 * This example shows some features of the X module (and co).
 * - How to show an RTex for debugging.
 * - Open a file dialog scene.
 * - Try to open an url in the system browser.
 * - Show a toast and a dialog prompt.
 * - Rumble if possible.
 * - Add an api to the mia terminal shell.
 * - And in general the swipt in terminal and virtual cursor
 * The X module also contains the iui features
 * and a color picker called XWObjColor
 * or a virtual keyboard called XViewKeys,
 * which is used in the XViewText line edit.
 */
