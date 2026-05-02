/**
 * A simple tea timer app.
 * Choose between three times, with configurable times.
 * App structure:
 *
 * - include and definitions like the shared context
 * - helper functions
 * - state machine switch functions
 * - start text edit to change a time
 * - main AView virtual functions: setup, update, render
 * - tea's main function
 */


#include "ex/tea.h"

/**
 * For simple apps its common to just include all of mia.
 */
#include "mia.h"


/**
 * frames per second and a timer for the sound to prevent app suspend
 */
#define TEA_FPS 3.0f
#define TEA_ALARM_TIME 12.0f

/** + and - button push time while timer is running */
#define TEA_ACTION_TIME 5

/** when + - are pressed, color animation time */
#define TEA_EFFECT_TIME 0.5


/**
 * States for the state machine.
 * START    -> shows time buttons to start the TIMER and an EDIT button
 * TIMER    -> countdown of the tea timer and an abort button to get back to START
 * FINISHED -> after countdown with a rebrew button to get back to START
 * EDIT     -> shows the time buttons, but will open a number edit scene on press.
 *             Done button to get back to START
 */
enum tea_state {
    TEA_START,
    TEA_TIMER,
    TEA_FINISHED,
    TEA_EDIT,
    TEA_NUM_STATES
};

/**
 * Context installed on the AView's o_user
 */
struct context {
    /**
     * background color for RTex_clear
     */
    vec4 bg_color;

    /**
     * Current state machine state
     */
    enum tea_state state;


    /**
     * SBuf of the loaded bell sound to be played on countdown
     */
    oobj alarm;

    /**
     * RObjRect of all animated cups.
     * In total 4 boxes (3 above the time buttons, 1 big animated)
     * The texture image has 4x4 sprites
     */
    oobj cup;

    /**
     * Current selected tea sort for the big cup
     */
    int tea_sort;

    /**
     * WTheme and WBox as main gui
     */
    oobj theme;
    oobj gui;

    /**
     * Simple WObj with a fixed size as area for the animated cups, which are rendered with the "cup" RObjRect.
     * The whole WObj gui is rendered with the WTheme in a single draw call.
     * It is possible to change and add sprites into to WTheme texture, see WImg.
     * But this way (using another draw call and only calculate the area) is much more flexible.
     */
    oobj main_cup_area;
    oobj small_cup_areas[3];

    /**
     * WText for the title or time, etc.
     */
    oobj text;

    /**
     * WGrid for the small cups and there buttons
     */
    oobj grid;

    /**
     * WBtn for the time buttons, including an internal WText
     */
    oobj time_btns[3];

    /**
     * WBtn's below the time button
     */
    oobj action_btn;
    oobj action_btn_text;
    oobj action_plus_btn;
    oobj action_minus_btn;

    /**
     * Sound Attribution
     */
    oobj attribution_click;
    bool attribution_show;

    /**
     * Timer time and alarm time.
     * The alarm time starts if finished and prevents the app from suspending
     */
    float tea_time;
    float alarm_time;

    /**
     * When + or - is pressed, the time gets a shoet effect color animation
     */
    float effect_time;
    vec4 effect_color;

    /**
     * o_timer time to measure
     */
    ou64 timer;
};

//
// helper functions
//


/**
 * Loads all tea times from a saved json.
 * @param out_times3 float[3] [in seconds]
 */
static void load_times(oobj view, float *out_times3)
{
    /**
     * Stuff allocated here is alloacated on this container. So we only need to o_del that.
     */
    oobj container = OObj_new(view);

    /**
     * Default times, if the loading fails
     */
    vecn_clone(out_times3, (float[]){2 * 60.0f, 3 * 60.0f, 5 * 60.0f}, 3);

    /**
     * Read the file as json. In our case its just an array of the three times (hopefully).
     * when a function returns a struct oobj_opt instead of an oobj, it may be NULL internally.
     * Its a way to inform the user: "hey that may fail to NULL!".
     * The oobj is under ".o"
     * Using the route "&..." for save files, which should be available on next app start
     */
    struct oobj_opt json = OJson_new_read_file(container, NULL, "&ex_tea_time.json");
    if (!json.o) {
        o_log("failed to load tea_time.json");
        goto CLEAN_UP;
    }

    /**
     * We expect 3 numbers for the tea times
     */
    for (int i = 0; i < 3; i++) {
        /**
         * OJson uses operator overloading, will call OJson_at (also o_num works)
         * Another way would be to call OJson_list to get all children in a list
         * the returned void * is either an OJson* or NULL
         */
        oobj jtime = o_at(json.o, i);
        if (!jtime) {
            o_log_warn("failed to get json tea index [%i]", i);
            goto CLEAN_UP;
        }

        /**
         * If this OJson is not a number, NULL is returned
         */
        double *time = OJson_number(jtime);
        if (!time) {
            o_log_warn("failed to convert json time [%i]", i);
            goto CLEAN_UP;
        }

        out_times3[i] = (float) *time;
    }

CLEAN_UP:
    /**
     * Clean up all allocated stuff in this function.
     * o_del(json.o) would also work when not using the container...
     */
    o_del(container);
}

/**
 * Save all tea times into a json file
 * @param times3 float[3] [in seconds]
 */
static void save_times(oobj view, const float *times3)
{
    oobj container = OObj_new(view);

    /**
     * OJson uses the OObj tree hierarchy for the represenstation
     * NULL is an optional name, if the parent is of type OJson_TYPE_OBJECT
     */
    oobj root = OJson_new_array(container, NULL);
    for (int i = 0; i < 3; i++) {
        OJson_new_number(root, NULL, times3[i]);
    }

    /**
     * As with the loading, we create the record file path and write to it.
     * After writing we call o_file_record_sync()
     *      which is needed by Emscripten to save to the cache (db)
     * Using the route "&..." for save files, which should be available on next app start
     */
    OJson_write_file(root, "&ex_tea_time.json");

    o_del(container);
}

/**
 * @return loaded tea time of a specific sort, simple helper
 */
static float get_tea_time(oobj view, int sort)
{
    assert(sort>=0 && sort<3);
    float times[3];
    load_times(view, times);
    return times[sort];
}

/**
 * Writes into the out_buf32 string the formated time in MM:SS
 * @param out_buf32 char[32]
 * @param time [in seconds]
 */
static void time_to_str(char *out_buf32, float time)
{
    int secs = (int) m_ceil(time);
    int mins = secs / 60;
    secs %= 60;
    snprintf(out_buf32, 32, "%02i:%02i", mins, secs);
}

/**
 * Reads a string and returns the time [in seconds]
 * Either MM:SS or only MM
 * @return time [in seconds]; -1 on failure
 */
static float str_to_time(const char *time_string)
{
    int mins = 0;
    int secs = 0;
    int result = sscanf(time_string, "%02i:%02i", &mins, &secs);
    if (result == 2 && mins >= 0 && secs >= 0 && secs < 60) {
        // got "SS"
        return (float) (mins * 60 + secs);
    } else if (result == 1 && mins >= 0) {
        // got "MM"
        return (float) (mins * 60);
    }
    // failed
    return -1;
}

/**
 * Helper to update the time buttons time text
 * @param btn or sort [0:2]
 */
static void set_btn_time(oobj view, int btn)
{
    struct context *C = o_user(view);
    float tea_time = get_tea_time(view, btn);
    char buf[32];
    time_to_str(buf, tea_time);

    /**
     * We search for the WText child in the direct children list of the button (0)
     * OObj_find can also look in recursion with the provided max depth (last parameter) (>0)
     */
    struct oobj_opt text = OObj_find(C->time_btns[btn], WText, NULL, 0);
    assert(text.o);
    WText_text_set(text.o, buf);
}

/**
 * Helper which updates the app title.
 * This is also called each frame in update.
 */
static void set_app_title(const char *title)
{
    AScene_title_set(a_scene(), title);
}


//
// state machine
//


/**
 * Show start state.
 * TEA TIMER text
 * 3 cups of animated tea
 * 3 time buttons
 * EDIT button
 */
static void state_start(oobj view)
{
    struct context *C = o_user(view);
    o_log("state -> start");
    C->state = TEA_START;

    set_app_title("Tea");
    WText_text_set(C->text, "TEA TIMER");
    WText_text_set(C->action_btn_text, "EDIT");
    WObj_hide_set(C->main_cup_area, true);
    WObj_hide_set(C->grid, false);
    WObj_hide_set(C->action_plus_btn, true);
    WObj_hide_set(C->action_minus_btn, true);
    WObj_hide_set(C->attribution_click, !C->attribution_show);
    for (int i = 0; i < 3; i++) {
        WObj_hide_set(C->small_cup_areas[i], false);
        WObj_style_apply(C->time_btns[i]);
        set_btn_time(view, i);
    }
}

/**
 * Starts the timer state.
 * Big animated cup of the selected tea.
 * Time running down
 * ABORT button
 */
static void state_timer(oobj view, int sort, float time)
{
    struct context *C = o_user(view);
    o_log("state -> timer # sort|time: %i|%f", sort, time);
    C->state = TEA_TIMER;

    // C->text and app title will be updated on each frame to the current timer
    WText_text_set(C->action_btn_text, "ABORT");
    WObj_hide_set(C->main_cup_area, false);
    WObj_hide_set(C->grid, true);
    WObj_hide_set(C->action_plus_btn, false);
    WObj_hide_set(C->action_minus_btn, false);
    WObj_hide_set(C->attribution_click, true);
    for (int i = 0; i < 3; i++) {
        // also hide these, so that there rect will have a size of 0
        WObj_hide_set(C->small_cup_areas[i], true);
    }


    C->tea_sort = sort;
    C->tea_time = time;
    C->timer = o_timer();
}

/**
 * Shows tea is ready finish state
 * Big animated cup of the selected tea.
 * TEA IS READY text
 * REBREW button
 */
static void state_finished(oobj view)
{
    struct context *C = o_user(view);
    o_log("state -> finished");
    C->state = TEA_FINISHED;

    set_app_title("TEA READY");
    WText_text_set(C->text, "TEA IS READY!");
    WText_text_set(C->action_btn_text, "REBREW?");
    WObj_hide_set(C->main_cup_area, false);
    WObj_hide_set(C->grid, true);
    WObj_hide_set(C->action_plus_btn, true);
    WObj_hide_set(C->action_minus_btn, true);
    WObj_hide_set(C->attribution_click, true);
    for (int i = 0; i < 3; i++) {
        // also hide these, so that there rect will have a size of 0
        WObj_hide_set(C->small_cup_areas[i], true);
    }

    C->alarm_time = TEA_ALARM_TIME;
    s_play(C->alarm, 0, 1);
    C->attribution_show = true;
}

/**
 * Show edit state.
 * EDIT TIMES text
 * 3 cups of animated tea
 * 3 time buttons for editing
 * DONE button
 */
static void state_edit(oobj view)
{
    struct context *C = o_user(view);
    o_log("state -> edit");
    C->state = TEA_EDIT;

    set_app_title("Tea");
    WText_text_set(C->text, "EDIT TIMES");
    WText_text_set(C->action_btn_text, "DONE");
    WObj_hide_set(C->main_cup_area, true);
    WObj_hide_set(C->grid, false);
    WObj_hide_set(C->action_plus_btn, true);
    WObj_hide_set(C->action_minus_btn, true);
    WObj_hide_set(C->attribution_click, true);
    for (int i = 0; i < 3; i++) {
        WObj_hide_set(C->small_cup_areas[i], false);
        WBtn_color_set(C->time_btns[i], vec4_(0.6, 0.3, 0.0, 1.0));
        set_btn_time(view, i);
    }
}


//
// tea edit scene
//

/**
 * Context for the edit scene
 */
struct edit_context {
    // this is "our" tea AView, as with the other functions
    oobj view;
    // clicked btn [0:2]
    int btn_edit;
};

/**
 * This event is called when the scene finished, either cancel or ok
 */
static void time_edit_done(oobj scene)
{
    struct edit_context *eC = o_user(scene);

    /**
     * XViewText is the AView of the edit scene, which shows the text edit
     */
    oobj viewtext = AScene_view(scene);
    if (XViewText_state(viewtext) != XViewText_OK) {
        /**
         * simple noop if canceled
         */
        return;
    }

    /**
     * The input field uses an OArray of char
     * text_string will be the C like start of the string (OArray is always element 0 terminated)
     */
    oobj text_array = XViewText_text_array(viewtext);
    char *text_string = o_at(text_array, 0);

    /**
     * Replace '.' with ':'
     * The NUM mode of XViewText does have a '.' button, so we change it while editing
     */
    o_str_replace_char_this(text_string, '.', ':');
    float time = str_to_time(text_string);
    if (time > 0 && time <= 99 * 60 + 59) {
        /**
         * Reading the user time succeeded.
         * So we load all 3 times, change the edited one and save back
         */
        float times[3];
        load_times(eC->view, times);
        times[eC->btn_edit] = time;
        save_times(eC->view, times);

        /**
         * On success we switch back to the start state.
         * Else the AScene just finishes and the edit state keeps running.
         */
        state_start(eC->view);
    } else {
        o_log("failed to parse time from user string: <%s>", text_string);
    }
}

/**
 * Starts an AScene with an XViewText (as AView)
 */
static void start_time_edit(oobj view, int btn)
{
    /**
     * Starts the text input scene.
     * 5: maximal characters (MM:SS)
     */
    oobj scene = x_viewtext_scene(view, "TEA TIME IN:   \'MM\'  OR  \'MM.SS\'", 5, time_edit_done, time_edit_done);

    /**
     * Installing the edit context for the callback event
     */
    struct edit_context *eC = o_user_new0(scene, *eC, 1);
    eC->view = view;
    eC->btn_edit = btn;

    /**
     * XViewText (AView) of the scene
     */
    oobj viewtext = AScene_view(scene);

    /**
     * XViewKeys (also an AView) is the used virtual keyboard.
     * We switch the page from text to number for easier user input
     */
    oobj keys = XViewText_viewkeys(viewtext);
    XViewKeys_page_set(keys, XViewKeys_factory_page_number(keys, NULL, NULL));
    XViewKeys_opt_keyboard_filter_set(keys, XViewKeys_factory_filter_number);

    /**
     * The XViewText let's us define a couple of custom replace buttons.
     * If one of these is pressed, its text is cloned into the input field.
     * So we create simple minute buttons from 1-8 mins as "MM".
     */
    for (int i = 1; i <= 8; i++) {
        char repl[16];
        o_strf_buf(repl, "%02i", i);
        XViewText_custom_replace(viewtext, repl, &vec4_(0.6, 0.3, 0.0, 1.0));
    }

    /**
     * Current buttons tea time as MM:SS string.
     * But we replace the ':' with '.', because the num keyboard has a button for that
     */
    char buf[32];
    time_to_str(buf, get_tea_time(view, btn));
    o_str_replace_char_this(buf, ':', '.');

    /**
     * XViewText uses an OArray of char for the input line.
     * We clear it first and append the string of the current time (MM.SS)
     */
    oobj text_array = XViewText_text_array(viewtext);
    OArray_clear(text_array);
    OArray_append_string(text_array, buf);
}

//
// view functions
//

/**
 * Called once at the start of the app
 * @param view AView object to render to
 */
static void setup(oobj view)
{
    /**
     * We create our context installed on the AView
     */
    struct context *C = o_user_new0(view, *C, 1);

    /**
     * Exit this scene on escape
     */
    AScene_escape_event_set_exit(a_scene());

    /**
     * The default camera has a minimal unit size of 180x180.
     * We use a smaller one. 96 would exaclty fit our 3x32 animated cups.
     * So we add a small border around that.
     * Normally, an AView automatically sets the minimal unit size to the parents camera scale.
     * So that it fits the whole app. That will be turned off using this function.
     */
    AView_cam_min_units_set(view, vec2_(96+8));

    /**
     * Background color with random hue angle. m_random() is already in [0:1]
     */
    vec4 hsva = {{m_random(), 0.66, 0.33, 1.0}};
    C->bg_color = vec4_hsv2rgb(hsva);


    /**
     * Alarm sound (ring bell)
     * Returns a struc oobj_opt, but we assume it to succeed.
     * o_assume is like an assert, but for runtime checks
     *
     * The sound is licensed under CC BY-NC 4.0, see "$ex/tea_alarm_license.txt"
     * https://orangefreesounds.com/clock-sound-effect/
     * Artist: Alexander
     */
    C->alarm = s_ogg_load_buf(view, "$ex/tea_alarm.ogg", NULL).o;
    o_assume(C->alarm, "failed to load alarm file");

    /**
     * Load and setup the cup render object with its 4x4 sprite
     */
    oobj cup_tex = RTex_new_file(view, "$ex/tea_cup.png");
    C->cup = RObjBox_new_tex(view, 4, cup_tex, true, 4, 4);

    /**
     * WTheme on which the gui is rendered
     */
    C->theme = WTheme_new_tiny(view);

    /**
     * Root WObj as vertical WBox
     */
    C->gui = WBox_new_v(view);

    /**
     * main_cup is the big (double sized) animated cup for the timer and finish state.
     * We make use of WAlign to center it in the row.
     */
    oobj main_cup_align = WAlign_new_center_h(C->gui);
    C->main_cup_area = WObj_new(main_cup_align);
    WObj_fixed_size_set(C->main_cup_area, vec2_(64, 64));
    WObj_hide_set(C->main_cup_area, true);

    /**
     * Main text which will also show the count down.
     * Also centered with a WAlign.
     * Char size is doubled and the text is padded on top and bottom with 4 units/pixels
     */
    oobj text_align = WAlign_new_center_h(C->gui);
    C->text = WTextShadow_new(text_align, "");
    WText_char_scale_set(C->text, vec2_(2));
    WObj_padding_set(C->text, vec4_(0, 4, 0, 4));

    /**
     * aligned cup and time button grid
     */
    oobj grid_align = WAlign_new_center_h(C->gui);
    C->grid = WGrid_new(grid_align, 3, 2, vec2_(-1, -1));
    WGrid_align_set(C->grid, WGrid_align_CENTER, WGrid_align_CENTER);

    // row 0: optional small cups
    // row 1: time buttons

    /**
     * Create the areas for the small cups and the time buttons
     */
    for (int i = 0; i < 3; i++) {
        C->small_cup_areas[i] = WObj_new(C->grid);
        WGrid_child_cell_set(C->small_cup_areas[i], ivec4_(i, 0, 1, 1));
        WObj_fixed_size_set(C->small_cup_areas[i], vec2_(32, 32));
        C->time_btns[i] = WBtn_new(C->grid);
        WGrid_child_cell_set(C->time_btns[i], ivec4_(i, 1, 1, 1));
        WObj_padding_set(C->time_btns[i], vec4_(0, 2, 0, 2));
        oobj text = WTextShadow_new(C->time_btns[i], "");
        //        WText_color_set(text, R_GRAY_X(0.2));
        WObj_padding_set(text, vec4_(1));
    }

    /**
     * Action buttons (edit; abort; rebrew; done)
     */
    oobj action_align = WAlign_new_center_h(C->gui);
    oobj action_box = WBox_new_h(action_align);
    WBox_spacing_set(action_box, vec2_(2));

    C->action_minus_btn = WBtn_new(action_box);
    WBtn_style_set(C->action_minus_btn, WBtn_FLAT_ROUND);
    WObj_padding_set(C->action_minus_btn, vec4_(0, 1, 0));
    WBtn_color_set(C->action_minus_btn, vec4_(0.5));
    oobj minus_text = WTextShadow_new(C->action_minus_btn, "-" O_TO_STRING(TEA_ACTION_TIME));
    WText_color_set(minus_text, vec4_(1, 0.8, 0.8, 0.5));
    WObj_padding_set(minus_text, vec4_(1));

    C->action_btn = WBtn_new(action_box);
    WBtn_color_set(C->action_btn, vec4_(0.66));
    C->action_btn_text = WTextShadow_new(C->action_btn, "");
    WText_color_set(C->action_btn_text, vec4_(1, 1, 1, 0.66));
    WObj_padding_set(C->action_btn_text, vec4_(1));

    C->action_plus_btn = WBtn_new(action_box);
    WBtn_style_set(C->action_plus_btn, WBtn_FLAT_ROUND);
    WObj_padding_set(C->action_plus_btn, vec4_(0, 1, 0));
    WBtn_color_set(C->action_plus_btn, vec4_(0.5));
    oobj plus_text = WTextShadow_new(C->action_plus_btn, "+" O_TO_STRING(TEA_ACTION_TIME));
    WText_color_set(plus_text, vec4_(0.8, 1, 0.8, 0.5));
    WObj_padding_set(plus_text, vec4_(1));

    C->attribution_click = WClick_new(WAlign_new_center_h(C->gui));
    oobj attribution_text = WTextShadow_new(C->attribution_click, "Clock Sound Effect\nAlexander CC BY-NC 4.0");
    WText_color_set(attribution_text, vec4_(1,1,1,0.66));

    /**
     * Updates the app icon
     */
    AScene_icon_set_file(a_scene(), "$ex/tea_icon.png");

    /**
     * Start of the state machine
     */
    state_start(view);
}

/**
 * Called each frame to update
 */
static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /**
     * Update the gui
     */
    vec2 center = RCam_proj_center_unit(AView_cam(view));
    vec2 min_size = {{96, 96}};
    vec2 lt = {{center.x - min_size.x / 2, center.y - min_size.y / 2}};
    WTheme_update(C->theme, C->gui, lt, min_size);


    /**
     * Check for button presses.
     * WBtn_clicked must be called to let the button "work".
     * Else WBtn_auto_mode can be used
     */
    for (int i = 0; i < 3; i++) {
        if (WBtn_clicked(C->time_btns[i])) {
            if (C->state == TEA_START) {
                float tea_time = get_tea_time(view, i);
                state_timer(view, i, tea_time);
            } else if (C->state == TEA_EDIT) {
                start_time_edit(view, i);
            }

            osize objects, resources, depth;
            ou64 timer = o_timer();
            OObj_tree_metrics(a_root(), &objects, &resources, &depth);
            double time_millis = o_timer_elapsed_millis(timer);
            o_log("obj's: %i, res's: %i, depth: %i, time: %f ms", objects, resources, depth, time_millis);
        }
    }

    if (WBtn_clicked(C->action_btn)) {
        if (C->state == TEA_START) {
            state_edit(view);
        } else {
            state_start(view);
        }
    }

    if (WBtn_clicked(C->action_minus_btn)) {
        C->tea_time -= TEA_ACTION_TIME;
        C->effect_time = TEA_EFFECT_TIME;
        C->effect_color = vec4_(1, 0.8, 0.8, 1);
    }
    if (WBtn_clicked(C->action_plus_btn)) {
        C->tea_time += TEA_ACTION_TIME;
        C->effect_time = TEA_EFFECT_TIME;
        C->effect_color = vec4_(0.8, 1, 0.8, 1);
    }

    if (WClick_pressed(C->attribution_click)) {
        o_log("attribution click");
        o_utils_open_url("https://orangefreesounds.com/clock-sound-effect/");
    }

    /**
     * applying + - effect color to the text
     */
    C->effect_time = o_max(0, C->effect_time-dt);
    float effect_t = m_clamp(C->effect_time / TEA_EFFECT_TIME, 0, 1);
    vec4 effect_col = vec4_mix(R_WHITE, C->effect_color, effect_t);
    WText_color_set(C->text, effect_col);

    /**
     * Runs the timer and displays the text
     */
    if (C->state == TEA_TIMER) {
        float elapsed = (float) o_timer_elapsed_s(C->timer);
        float remaining = C->tea_time - elapsed;

        if (remaining > 0) {
            char buf[32];
            time_to_str(buf, remaining);
            WText_text_set(C->text, buf);
            set_app_title(buf);
        } else {
            // this block is called once at the end of the countdown
            state_finished(view);
        }
    }

    /**
     * cound down the alarm time. see below
     */
    if (C->state == TEA_FINISHED) {
        if (C->alarm_time > 0) {
            C->alarm_time -= dt;
        }
    }


    /**
     * While timer state is running or the alarm is played, we want the app to ignore the app pause
     */
    bool ignore_pause = C->state == TEA_TIMER || (C->state == TEA_FINISHED && C->alarm_time > 0);
    a_app_suspend_paused_set(!ignore_pause);
}

/**
 * Called each frame to render
 */
static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /**
     * Render the backgrond color
     */
    vec4 bg = C->bg_color;
    if (C->state == TEA_EDIT) {
        // darken the rgb part of the color by half
        bg.xyz = vec3_scale(bg.xyz, 0.5);
    }
    RTex_clear_full(tex, bg);

    /**
     * Render all buttons and text, etc.
     */
    WTheme_render(C->theme, tex);

    /**
     * Update the RObjBox boxes.
     * box 0 is the big main cup
     * boxes 1-3 are the smaller cups above the time buttons
     */
    struct r_box *cup_box = o_at(C->cup, 0);
    cup_box->sprite.x = m_mod(cup_box->sprite.x + dt * TEA_FPS, 4);
    cup_box->sprite.y = C->tea_sort;
    cup_box->rect = WObj_gen_rect(C->main_cup_area);

    for (int i = 0; i < 3; i++) {
        struct r_box *c = o_at(C->cup, i + 1);
        c->sprite.x = cup_box->sprite.x;
        c->sprite.y = i;
        c->rect = WObj_gen_rect(C->small_cup_areas[i]);
    }

    /**
     * Render the animated cups
     */
    RTex_ro(tex, C->cup);
}


/**
 * Starting point of the tea app
 * @param root parent to allocate on
 * @return AScene created of the tea app
 */
oobj ex_tea_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}

/**
 * Another Starting point of the tea app. This time the scene is created during a splash screen
 * @param root parent to allocate on
 * @return AScene of the splash scene that creates the mia app
 */
oobj ex_tea_main_splashed(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj splash = u_splash_new_mia(root, view, true, AScene_SAFE,
                                   "HORSIMANN", "TEA", 2.0f, true);
    return splash;
}
