/**
 * Mia's own fonts.
 * Use the textlayout to set casing, wrap and align modes.
 * Switch between available fonts with a button.
 * Mia also supports ttf apart from these builtins btw. (o/OTtf.h)
 */


#include "mia.h"

struct context {
    oobj theme;
    oobj gui;

    // WText stuff

    enum WText_casing casing;
    oobj casing_btn;
    oobj casing_info;

    enum WText_wrap wrap;
    oobj wrap_btn;
    oobj wrap_info;

    enum WText_align align;
    oobj align_btn;
    oobj align_info;
    
    oobj wtext;

    // RTex_font (uses RObjText) stuff:

    /**
     * enum specifing the font to render
     */
    enum RTex_font font;
    oobj font_btn;
    oobj font_info;
    oobj text_container;

};


static void setup(oobj view)
{
    struct context *C = o_user_new0(view, *C, 1);

    /**
     * WText uses the font available in the theme.
     *     (WTheme has WText and WTextShadow)
     * RTex_font can use different fonts.
     */
    C->theme = WTheme_new_tiny(view);
    C->gui = WBox_new_v(view);
    WBox_spacing_set(C->gui, vec2_(4));


    // WText stuff

    oobj pane = WPane_new(C->gui);
    oobj pane_box = WBox_new_v(pane);
    WBox_spacing_set(pane_box, vec2_(1));

    oobj label = WTextShadow_new(pane_box, "WText");
    WText_char_scale_set(label, vec2_(2));
    WObj_padding_set(label, vec4_(2));

    oobj hbox = WBox_new_h(pane_box);

    C->casing = WText_casing_UNCHANGED;
    C->casing_btn = WBtn_new(hbox);
    WObj_min_size_ref(C->casing_btn)->x = 60;
    C->casing_info = WText_new(C->casing_btn, "UNCHANGED");
    WObj_padding_set(C->casing_info, vec4_(1));

    C->wrap = WText_wrap_WORD;
    C->wrap_btn = WBtn_new(hbox);
    WObj_min_size_ref(C->wrap_btn)->x = 30;
    C->wrap_info = WText_new(C->wrap_btn, "WORD");
    WObj_padding_set(C->wrap_info, vec4_(1));

    C->align = WText_align_BLOCK;
    C->align_btn = WBtn_new(hbox);
    WObj_min_size_ref(C->align_btn)->x = 30;
    C->align_info = WText_new(C->align_btn, "BLOCK");
    WObj_padding_set(C->align_info, vec4_(1));

    oobj wtext_bg = WColor_new(pane_box, vec4_(0.1, 0.1, 0.1, 0.5));
    /**
     * Edited version of Lorem ipsum,
     * to show some casing effects
     */
    C->wtext = WText_new(wtext_bg, "Lorem Ipsum DOLOR sit Amet, "
                         "Consectetur Adipiscing ELIT, "
                         "SED Eiusmod Tempor incidunt "
                         "ut LABORE et DOLORE Magna ALiqua.");
    WObj_fixed_size_set(wtext_bg, vec2_(120, 32));
    WObj_min_size_set(C->wtext, vec2_(120, 32));


    // RTex_font stuff

    pane = WPane_new(C->gui);
    pane_box = WBox_new_v(pane);
    WBox_spacing_set(pane_box, vec2_(1));

    label = WTextShadow_new(pane_box, "RTex_font");
    WText_char_scale_set(label, vec2_(2));
    WObj_padding_set(label, vec4_(2));

    C->font = RTex_font35;
    C->font_btn = WBtn_new(pane_box);
    C->font_info = WText_new(C->font_btn, "font35");
    WObj_min_size_ref(C->font_info)->x = 54;
    WObj_padding_set(C->font_info, vec4_(1));

    /**
     * container from which RTex_font gets the position
     */
    C->text_container = WObj_new(pane_box);
    WObj_min_size_set(C->text_container, vec2_(120, 60));


}

static void update(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    vec2 lt = vec2_(28, 8);
    WTheme_update(C->theme, C->gui, lt, vec2_(0));


    // WText stuff

    if (WBtn_clicked(C->casing_btn)) {
        C->casing++;
        C->casing %= WText_casing_ENUM_MAX;
        const char *modes[] = {
            "UNCHANGED",
            "LOWER",
            "UPPER"
        };
        assert(o_stack_array_num(modes) == WText_casing_ENUM_MAX);
        WText_text_set(C->casing_info, modes[C->casing]);
    }
    if (WBtn_clicked(C->wrap_btn)) {
        C->wrap++;
        C->wrap %= WText_wrap_ENUM_MAX;
        const char *modes[] = {
            "OFF",
            "ANY",
            "WORD"
        };
        assert(o_stack_array_num(modes) == WText_wrap_ENUM_MAX);
        WText_text_set(C->wrap_info, modes[C->wrap]);
    }
    if (WBtn_clicked(C->align_btn)) {
        C->align++;
        C->align %= WText_align_ENUM_MAX;
        const char *modes[] = {
            "LEFT",
            "RIGHT",
            "CENTER",
            "BLOCK"
        };
        assert(o_stack_array_num(modes) == WText_align_ENUM_MAX);
        WText_text_set(C->align_info, modes[C->align]);
    }

    WText_casing_set(C->wtext, C->casing);
    WText_wrap_set(C->wtext, C->wrap);
    WText_align_set(C->wtext, C->align);
    
    /** align is only used in wrap_WORD */
    WObj_hide_set(C->align_btn, C->wrap != WText_wrap_WORD);


    // RTex_font stuff

    if (WBtn_clicked(C->font_btn)) {
        C->font++;
        C->font %= RTex_font_ENUM_MAX;
        const char *modes[] = {
            "font35",
            "font35_shadow",
            "font55_shadow",
            "font55_shadow",
            "font58",
            "font58_shadow"
        };
        assert(o_stack_array_num(modes) == RTex_font_ENUM_MAX);
        WText_text_set(C->font_info, modes[C->font]);
    }
}

static void render(oobj view, oobj tex, float dt)
{
    struct context *C = o_user(view);

    RTex_clear_full(tex, vec4_(0.5, 0.4, 0.2, 1.0));
    WTheme_render(C->theme, tex);

    /**
     * The Mia fonts are ascii only and contains all ascii printable characters.
     * Also these fonts are monospaced and have all the same offset to the next char.
     * In addition to that, if a value is beyond the ascii limit (127) its printed as a full block (\xff).
     * Want to use ttf fonts instead? Have a look at "o/OTtf.h"
     */
    const char *text =
            " !\"#$%&'()*+,-./    \n"
            "0123456789:;<=>?     \n"
            "@ABCDEFGHIJKLMNO     \n"
            "PQRSTUVWXYZ[\\]^_    \n"
            "`abcdefghijklmno     \n"
            "pqrstuvwxyz{|}~\xff  \n";

    vec2 text_pos = WObj_gen_lt(C->text_container);
    RTex_text(tex, text, m_2(text_pos), C->font, R_WHITE);
}


oobj ex_06_main(oobj root)
{
    oobj view = AView_new(root, setup, update, render);
    oobj scene = AScene_new(root, view, true, AScene_SAFE);
    return scene;
}

/**
 * Summary:
 *
 * This example shows a preview of Mias built-in fonts.
 * You can use textlayout parameters such as casing, wrap and alignment.
 * The widget system only allows for two fonts in the theme "WText" and "WTextShadow"
 * In contrast to that RTex_font or the underlying RObjText supports multiple fonts.
 * You want to use ttf fonts? Have a look at o/OTtf.h which generates from ttf texts
 */
