/**
 * Move and hide'able windows and more examples using the mia widget library.
 * The example "ex_03_iui.c" already showed a use case for windows -> as floating debug input.
 */

#include "mia.h"

struct context {
    /**
     * As with example "ex_04_widgets.c" we need a WTheme to render the widgets
     */
    oobj theme;
    oobj gui;

    /**
     * The default WObj already stacks its children on top of each other.
     * WStack enhances this with some additional features.
     * In this case we use the "window_auto_mode" which reorders the windows if a window header is pressed.
     *      So bringing that window to the front
     */
    oobj stack;
};

/**
 * The structure of this example is:
 * - helpers and functions that create a window.
 * - AView's setup, update, render for this example
 * - main entry point function
 */


/**
 * This window creates a simple Hello World message
 */
static void wnd_hello(oobj view, float l, float t)
{
    struct context *C = o_user(view);

    /**
     * Creates a WWindow with a WText in the header.
     * Header min size (if window is hidden), defaults to vec2_(32,10) if vec2_(-1) is passed
     * A window can be dragged in the available size it gets, WStack uses the full view size, see the update function.
     */
    oobj wnd = WWindow_new_title(C->stack, vec2_(-1), "HELLO", NULL);
    WWindow_lt_set(wnd, vec2_(l, t));

    /**
     * WWindow's direct children are rendered >BEHIND< the window.
     * To add widgets in the window body, use the following widget as parent (of type WPane).
     * Same for the header with WWindow_header()
     */
    oobj body = WWindow_body(wnd);

    /**
     * Hello World with shadowed text
     */
    oobj hello = WTextShadow_new(body, "Hello World");
    WObj_tooltip_set(hello, "This is a tooltip");
    WObj_padding_set(hello, vec4_(8));
}


/**
 * This window shows how the floating WBox works
 */
static void wnd_box(oobj view, float l, float t)
{
    struct context *C = o_user(view);

    oobj wnd = WWindow_new_title(C->stack, vec2_(-1), "BOX", NULL);
    WWindow_lt_set(wnd, vec2_(l, t));
    oobj body = WWindow_body(wnd);

    /**
     * Mode H would only place children right next to each other.
     * Mode H_V will look at the available room and add a new row
     */
    oobj box = WBox_new(body, WBox_H_V);
    WObj_tooltip_set(box, "Just a demonstration of WBox");
    WBox_spacing_set(box, vec2_(2));

    /**
     * Set the (horizontal) room available for the floating box
     */
    WObj_min_size_set(box, vec2_(80, 0));

    const int NUM_ITEMS = 14;
    for (int i = 0; i < NUM_ITEMS; i++) {
        vec4 hsva = vec4_(0.66 * i/NUM_ITEMS, 1, 1, 1);
        vec4 rgba = vec4_hsv2rgb(hsva);
        oobj color = WColor_new(box, rgba);
        WObj_min_size_set(color, vec2_(16));
    }
}


//
// helpers for ex_05_wnd_options
//

/**
 * Helper that adds a WBtn in toggle auto mode.
 * Layout is:
 *      | title      btn |
 */
static oobj option_btn_toggle(oobj parent, const char *title, const char *tooltip)
{
    /**
     * The mode H_WEIGHTS places its children right next to each other, like mode H.
     * But it makes use of the given (min_size) room grow the children.
     * Each child may have a weight set to them. the bigger the more room for that child.
     * Default is turned off with -1 (<0).
     * The WText object for the title does not use the weight and will simply use its needed size.
     * The WAlign has a weight of 1.0 and so will get the remaining room left.
     * With the align mode END in horizontal, the inner WBtn child will be placed on the right side.
     */
    oobj box = WBox_new(parent, WBox_H_WEIGHTS);
    WObj_tooltip_set(box, tooltip);
    oobj text = WText_new(box, title);
    oobj align = WAlign_new(box);
    WBox_child_weight_set(align, 1.0);
    WAlign_align_set(align, WAlign_END, WAlign_CENTER);
    oobj btn = WBtn_new(align);
    WBtn_auto_mode_set(btn, WBtn_auto_TOGGLED);
    WBtn_color_set(btn, R_WHITE);
    return btn;
}

/**
 * Helper that adds a WBtn switch
 */
static oobj option_switch(oobj parent, const char *title, const char *tooltip)
{
    oobj btn = option_btn_toggle(parent, title, tooltip);
    WBtn_style_set(btn, WBtn_SWITCH);
    return btn;
}

/**
 * Helper that adds a WBtn checkbox
 */
static oobj option_check(oobj parent, const char *title, const char *tooltip)
{
    oobj btn = option_btn_toggle(parent, title, tooltip);
    WBtn_style_set(btn, WBtn_CHECK);
    return btn;
}

/**
 * Helper event that creates the radio selection in setting other radio buttons to false
 */
static void radio_event(oobj btn)
{
    oobj group = o_user(btn);
    WBtn_set_group(btn, OList_list(group), -1, false);
}

/**
 * Helper that adds a WBtn radiobutton
 */
static oobj option_radio(oobj parent, const char *title, oobj group, const char *tooltip)
{
    oobj btn = option_btn_toggle(parent, title, tooltip);
    o_user_set(btn, group);
    OList_push(group, btn);
    WBtn_style_set(btn, WBtn_RADIO);
    WBtn_auto_event_set(btn, radio_event);
    return btn;
}

/**
 * This window shows some different option buttons
 */
static void wnd_options(oobj view, float l, float t)
{
    struct context *C = o_user(view);

    oobj wnd = WWindow_new_title(C->stack, vec2_(-1), "OPTNS", NULL);
    WWindow_lt_set(wnd, vec2_(l, t));
    oobj body = WWindow_body(wnd);
    WPane_color_set(body, vec4_(0.3, 0.3, 0.2, 1.0));

    oobj box = WBox_new_v(body);
    WBox_spacing_set(box, vec2_(10));
    WObj_min_size_set(box, vec2_(80, 0));

    /**
     * Default colors came from shared WStyle objects (not a widget).
     * A new WObj will use its parent WStyle, or create a new one.
     * The default WBtn color is gray. We change that to white for the switches, checkboxes, etc.
     */
    oobj style = WStyle_new(box);
    WStyle_btn_color_set(style, R_WHITE);
    WObj_style_set(box, style, true);

    option_switch(box, "MAGIC", "Turn on magic");
    option_check(box, "SOUND", "Enable sound effects");
    oobj debug = option_check(box, "DEBUG", "debug mode");
    WBtn_down_set(debug, true);

    oobj frame = WFrame_new_title(box, "MODE", NULL);
    oobj frame_box = WBox_new_v(frame);
    WBox_spacing_set(frame_box, vec2_(2));
    WObj_padding_set(frame_box, vec4_(2));

    oobj group = OList_new(frame_box, NULL, 0);
    option_radio(frame_box, "FUN", group, "set to fun mode");
    option_radio(frame_box, "WORK", group, "set to work mode");
    option_radio(frame_box, "SLEEP", group, "set to sleep mode");
    oobj chill = option_radio(frame_box, "CHILL", group, "set to chill mode");
    WBtn_down_set(chill, true);
}

//
// helper for ex_05_wnd_slider
//

/**
 * Helper event that is called when the slider value is changed by the user
 */
static void slider_event(oobj slider)
{
    oobj progress = o_user(slider);
    float val = WSlider_progress(slider);

    /**
     * We set the progress to the inverse of the slider (just for fun...)
     */
    WProgress_progress_set(progress, 1.0f - val);
}


/**
 * This window shows different slider widgets
 */
static void wnd_slider(oobj view, float l, float t)
{
    struct context *C = o_user(view);

    oobj wnd = WWindow_new_title(C->stack, vec2_(-1), "SLIDR", NULL);
    WWindow_lt_set(wnd, vec2_(l, t));
    oobj body = WWindow_body(wnd);
    WPane_color_set(body, vec4_(0.2, 0.2, 0.4, 1.0));

    oobj box = WBox_new_v(body);
    WBox_spacing_set(box, vec2_(10));
    WObj_min_size_set(box, vec2_(80, 0));

    oobj slider_frame = WFrame_new_title(box, "SLIDER", NULL);
    WObj_tooltip_set(slider_frame, "Slide the slider to change the progress");
    oobj slider_frame_box = WBox_new_v(slider_frame);
    WBox_spacing_set(slider_frame_box, vec2_(2));
    WObj_padding_set(slider_frame_box, vec4_(2));

    /**
     * When the user drags the WSlider,
     * the resulting number is exactly (absolut) on the dragged position.
     * The WProgress just shows a colored bar.
     * Both are also available in Y Mode (vertical).
     */
    oobj slider = WSlider_new(slider_frame_box);
    oobj progress = WProgress_new(slider_frame_box);
    WProgress_color_set(progress, vec4_(0.2, 0.5, 0.2, 1.0));
    WObj_min_size_ref(progress)->y = 8;
    WSlider_change_event_set(slider, slider_event);
    o_user_set(slider, progress);
    WSlider_progress_set(slider, 0.33);
    /**
     * Call to init the progress, the event is only automatically called on user change
     */
    slider_event(slider);


    /**
     * A WDrag is different from a WSlider and more like a notebook cursor touchpad.
     * It uses a relative dragging to change the result.
     * If dragged slowly, only a fraction is changed.
     * Also available in only X or Y mode.
     */
    oobj drag_frame = WFrame_new_title(box, "DRAG", NULL);
    WObj_tooltip_set(drag_frame, "Drag the trackpad to change the number\n"
                     "slow for small changes\n"
                     "fast for large\n"
                     "Double click to set\n"
                     "Also available in X only mode (see below)");
    oobj drag = WDrag_new(drag_frame);
    WDrag_mode_set(drag, WDrag_XY);
    WObj_min_size_set(drag, vec2_(48));

    /**
     * Combines a titled WFrame, a WDrag in X mode and a WText for the resulting number.
     * Have a look at x/num.h for an addition in which a number edit scene is opened, when the number is clicked.
     */
    oobj num = WNum_new(box, 10, 50, 0.1, "NUM");
    WObj_tooltip_set(num, 
            "Drag the num trackpad for precise number changes\n"
            "Double click to set");
}


//
// helper for ex_05_wnd_click
//

/**
 * Helper event that is called when the WClick is updated
 */
static void click_update(oobj click)
{
    oobj click_text = o_user(click);

    /**
     * True if the pointer is down on the widget
     */
    bool down = WClick_down(click);
    if (!down) {
        return;
    }

    /**
     * Relative to the WClick lt.
     * .y is mostly negative (+y is up...)
     */
    vec2 pos = WClick_pos(click);

    /**
     * Buffer version of o_strf (others are default o_strf (needs a parent OObj) or o_strf_a (arena allocator))
     */
    char txt[32];
    o_strf_buf(txt, "POS: %2i %2i",
               (int) pos.x, (int) pos.y);

    WText_text_set(click_text, txt);
}


/**
 * This window shows the abstract WClick object, which defines a click or pressable area.
 * Useful for image pickers or custom buttons, etc.
 */
static void wnd_click(oobj view, float l, float t)
{
    struct context *C = o_user(view);

    oobj wnd = WWindow_new_title(C->stack, vec2_(-1), "CLICK", NULL);
    WWindow_lt_set(wnd, vec2_(l, t));
    oobj body = WWindow_body(wnd);
    WPane_color_set(body, vec4_(0.2, 0.4, 0.2, 1.0));

    oobj box = WBox_new_v(body);
    WBox_spacing_set(box, vec2_(10));
    WObj_padding_set(box, vec4_(4));

    oobj click_frame = WFrame_new_title(box, "CLICK", NULL);
    WObj_tooltip_set(click_frame, "Tip the widget to see the position");
    oobj click = WClick_new(click_frame);
    oobj click_color = WColor_new(click, vec4_(0.5, 0.5, 0.5, 1.0));
    WObj_fixed_size_set(click_color, vec2_(64, 64));

    oobj click_text = WText_new(box, "POS:");

    /**
     * Each WObj may have an optional update callback event.
     * We use it to update the clicktext with the pressed (down) position relative to the WClick
     */
    WObj_update_event_fn_set(click, click_update);
    o_user_set(click, click_text);
}

static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    C->theme = WTheme_new_tiny(view);

    /**
     * This time we use a WTooltip as root gui object.
     * It automatically shows a tooltip on top of the rendered stuff.
     * Tooltips can be set via WObj_tooltip_set.
     * When using a cursor it displays the tooltip after 1.0 seconds hovering.
     * For touch displays it needs a press down 1.0 seconds.
     */
    C->gui = WTooltip_new(view);

    /**
     * For window managment, uses WTooltip as parent so tooltips work out of the box
     */
    C->stack = WStack_new(C->gui);
    WStack_window_auto_mode_set(C->stack, true);

    /**
     * Create windows:
     */
    wnd_click(view, 64, 32);
    wnd_slider(view, 56, 48);
    wnd_options(view, 48, 64);
    wnd_box(view, 40, 80);
    wnd_hello(view, 32, 96);
}


//
// view functions
//

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    /**
     * A special update function that uses the full available projected size of the RTex
     */
    WTheme_update_full_tex(C->theme, C->gui, tex);
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    WTheme_render(C->theme, tex);
}


oobj ex_05_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}

/**
 * Summary:
 *
 * This example shows some more advanced Mia widgets and windows.
 * We learned more about events and how to make use of OObj's user data for these.
 */
