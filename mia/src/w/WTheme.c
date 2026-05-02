#include "w/WTheme.h"
#include "o/OObj_builder.h"
#include "o/OArray_raw.h"
#include "o/ODelcallback.h"
#include "r/RObjBox.h"
#include "w/WObj.h"
#include "w/WStack.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"

#include "o/log.h"

/**
 * A ninepatch is defined as
 * [0] lt, [1] ct, [2] rt
 * [3] lc, [4] cc, [5] rc
 * [6] lb, [7] cb, [8] rb
 */
static void set_ninepatch(struct u_atlas atlas, oobj tex, int sprite, vec2 lt, vec2 lt_size, vec2 center_size, vec2 rb_size)
{
    atlas.rects[sprite + 0] = vec4_(lt.x, lt.y,
                                    lt_size.x, lt_size.y);
    atlas.rects[sprite + 1] = vec4_(lt.x + lt_size.x, lt.y,
                                    center_size.x, lt_size.y);
    atlas.rects[sprite + 2] = vec4_(lt.x + lt_size.x + center_size.x, lt.y,
                                    rb_size.x, lt_size.y);
    atlas.rects[sprite + 3] = vec4_(lt.x, lt.y + lt_size.y,
                                    lt_size.x, center_size.y);
    atlas.rects[sprite + 4] = vec4_(lt.x + lt_size.x, lt.y + lt_size.y,
                                    center_size.x, center_size.y);
    atlas.rects[sprite + 5] = vec4_(lt.x + lt_size.x + center_size.x, lt.y + lt_size.y,
                                    rb_size.x, center_size.y);
    atlas.rects[sprite + 6] = vec4_(lt.x, lt.y + lt_size.y + center_size.y,
                                    lt_size.x, rb_size.y);
    atlas.rects[sprite + 7] = vec4_(lt.x + lt_size.x, lt.y + lt_size.y + center_size.y,
                                    center_size.x, rb_size.y);
    atlas.rects[sprite + 8] = vec4_(lt.x + lt_size.x + center_size.x, lt.y + lt_size.y + center_size.y,
                                    rb_size.x, rb_size.y);
}

//
// public
//

WTheme *WTheme_init(oobj obj, oobj parent, oobj tex, bool move_tex,
                    struct u_atlas atlas, bool move_atlas)
{
    WTheme *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, WTheme_ID);

    self->ro = RObjBox_new_tex_rgba(self, 0, tex, move_tex, 1, 1);

    // setup OArray as dynamic array (using raw mode)
    oobj boxes = RObjBox_boxes(self->ro);
    OArray_dyn_realloc_mode_set(boxes, OArray_REALLOC_DOUBLED_FRONT);
    OArray_realloc(boxes, 128, 128);

    self->atlas = atlas;
    if (move_atlas) {
        u_atlas_move(&self->atlas, self);
    }

    
    // vfuncs
    self->super.v_op_at = WTheme__v_op_at;
    self->super.v_op_num = WTheme__v_op_num;

    return self;
}

WTheme *WTheme_new_tiny(oobj parent)
{
    oobj tex = RTex_new_file(parent, "$w/theme_tiny.png");

    struct u_atlas atlas = u_atlas_new(parent, WTheme_ENUM_MAX);
    //
    // atlas setup
    //

    // fonts
    for (int i = 0; i < WTheme_FONT_NUM; i++) {
        int col = i % 16;
        int row = i / 16;
        atlas.rects[WTheme_FONT + i] = vec4_(1 + col * 8, 1 + row * 8, 3, 5);
        atlas.rects[WTheme_FONT_SHADOW + i] = vec4_(1 + col * 8, 65 + row * 8, 4, 6);
    }

    // white (just use center of '+')
    {
        vec4 white = atlas.rects[WTheme_char_idx('+')];
        white.xy = vec2_add_v(white.xy, vec2_(1, 2));
        white.zw = vec2_(1);
        atlas.rects[WTheme_WHITE] = white;

    }

    // icons
    for (int i = 0; i < 14; i++) {
        atlas.rects[WTheme_ICON_SMALL_WND_MIN + i] = vec4_(i * 8 + 2, 128 + 2, 5, 5);
    }
    for (int i = 0; i < 14; i++) {
        atlas.rects[WTheme_ICON_WND_MIN + i] = vec4_(i * 8 + 1, 128 + 1, 7, 7);
    }
    for (int i = 0; i < 13; i++) {
        atlas.rects[WTheme_ICON_CROSS + i] = vec4_(i * 8 + 1, 136 + 1, 7, 7);
    }
    for (int i = 0; i < 6; i++) {
        atlas.rects[WTheme_ICON_RECORD+ i] = vec4_(i * 8 + 1, 144 + 1, 7, 7);
    }
    for (int i = 0; i < 3; i++) {
        atlas.rects[WTheme_ICON_PICKER_X + i] = vec4_(i * 8 + 1, 152 + 1, 7, 7);
    }
    for (int i = 0; i < 4; i++) {
        atlas.rects[WTheme_ICON_GEAR + i] = vec4_(i * 8 + 1, 160 + 1, 7, 7);
    }
    for (int i = 0; i < 6; i++) {
        atlas.rects[WTheme_ICON_FILE + i] = vec4_(i * 8 + 1, 168 + 1, 7, 7);
    }

    // btn
    set_ninepatch(atlas, tex, WTheme_BTN_FLAT,
                  vec2_(1, 192 + 1), vec2_(2, 2), vec2_(1), vec2_(2, 3));
    set_ninepatch(atlas, tex, WTheme_BTN_FLAT__PRESSED,
                  vec2_(8 + 1, 192 + 1), vec2_(2, 3), vec2_(1), vec2_(2, 2));

    set_ninepatch(atlas, tex, WTheme_BTN_FLAT_ROUND,
                  vec2_(16 + 1, 192 + 1), vec2_(2, 2), vec2_(1), vec2_(2, 3));
    set_ninepatch(atlas, tex, WTheme_BTN_FLAT_ROUND__PRESSED,
                  vec2_(24 + 1, 192 + 1), vec2_(2, 3), vec2_(1), vec2_(2, 2));

    set_ninepatch(atlas, tex, WTheme_BTN_DEF,
                  vec2_(32 + 1, 192), vec2_(3, 2), vec2_(1), vec2_(3, 5));
    set_ninepatch(atlas, tex, WTheme_BTN_DEF__PRESSED,
                  vec2_(40 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(3, 3));
                  
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_ROUND,
                  vec2_(48 + 1, 192), vec2_(3, 2), vec2_(1), vec2_(3, 5));
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_ROUND__PRESSED,
                  vec2_(56 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(3, 3));
    
    
    
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_DUAL,
                  vec2_(32 + 2, 192), vec2_(2, 2), vec2_(1), vec2_(2, 5));
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_DUAL__PRESSED,
                  vec2_(40 + 2, 192), vec2_(2, 4), vec2_(1), vec2_(2, 3));
                  
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_DUAL_L,
                  vec2_(32 + 2, 192), vec2_(2, 2), vec2_(1), vec2_(3, 5));
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_DUAL_L__PRESSED,
                  vec2_(40 + 2, 192), vec2_(2, 4), vec2_(1), vec2_(3, 3));
                  
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_DUAL_R,
                  vec2_(32 + 1, 192), vec2_(3, 2), vec2_(1), vec2_(2, 5));
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_DUAL_R__PRESSED,
                  vec2_(40 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(2, 3));
    
    // DUAL ROUND is part of ROUND AND PART OF THE DEFUALT DUAL
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_ROUND_DUAL_L,
                  vec2_(48 + 1, 192), vec2_(3, 2), vec2_(1), vec2_(3, 5));
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_ROUND_DUAL_L__PRESSED,
                  vec2_(56 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(3, 3));
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_L+0] = atlas.rects[WTheme_BTN_DEF_DUAL+0];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_L+3] = atlas.rects[WTheme_BTN_DEF_DUAL+3];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_L+6] = atlas.rects[WTheme_BTN_DEF_DUAL+6];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_L__PRESSED+0] = atlas.rects[WTheme_BTN_DEF_DUAL__PRESSED+0];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_L__PRESSED+3] = atlas.rects[WTheme_BTN_DEF_DUAL__PRESSED+3];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_L__PRESSED+6] = atlas.rects[WTheme_BTN_DEF_DUAL__PRESSED+6];
                  
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_ROUND_DUAL_R,
                  vec2_(48 + 1, 192), vec2_(3, 2), vec2_(1), vec2_(3, 5));
    set_ninepatch(atlas, tex, WTheme_BTN_DEF_ROUND_DUAL_R__PRESSED,
                  vec2_(56 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(3, 3));
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_R+2] = atlas.rects[WTheme_BTN_DEF_DUAL+2];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_R+5] = atlas.rects[WTheme_BTN_DEF_DUAL+5];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_R+8] = atlas.rects[WTheme_BTN_DEF_DUAL+8];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_R__PRESSED+2] = atlas.rects[WTheme_BTN_DEF_DUAL__PRESSED+2];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_R__PRESSED+5] = atlas.rects[WTheme_BTN_DEF_DUAL__PRESSED+5];
    atlas.rects[WTheme_BTN_DEF_ROUND_DUAL_R__PRESSED+8] = atlas.rects[WTheme_BTN_DEF_DUAL__PRESSED+8];


    set_ninepatch(atlas, tex, WTheme_BTN_BIG,
                  vec2_(64 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(3, 9));
    set_ninepatch(atlas, tex, WTheme_BTN_BIG__PRESSED,
                  vec2_(72 + 1, 192), vec2_(3, 7), vec2_(1), vec2_(3, 6));

    set_ninepatch(atlas, tex, WTheme_BTN_BIG_ROUND,
                  vec2_(80 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(3, 9));
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_ROUND__PRESSED,
                  vec2_(88 + 1, 192), vec2_(3, 7), vec2_(1), vec2_(3, 6));
                  
           
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_DUAL,
                  vec2_(64 + 2, 192), vec2_(2, 4), vec2_(1), vec2_(2, 9));
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_DUAL__PRESSED,
                  vec2_(72 + 2, 192), vec2_(2, 7), vec2_(1), vec2_(2, 6));

    set_ninepatch(atlas, tex, WTheme_BTN_BIG_DUAL_L,
                  vec2_(64 + 2, 192), vec2_(2, 4), vec2_(1), vec2_(3, 9));
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_DUAL_L__PRESSED,
                  vec2_(72 + 2, 192), vec2_(2, 7), vec2_(1), vec2_(3, 6));
              
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_DUAL_R,
                  vec2_(64 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(2, 9));
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_DUAL_R__PRESSED,
                  vec2_(72 + 1, 192), vec2_(3, 7), vec2_(1), vec2_(2, 6));

                  
    // DUAL ROUND is part of ROUND AND PART OF THE DEFUALT DUAL
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_ROUND_DUAL_L,
                  vec2_(80 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(3, 9));
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_ROUND_DUAL_L__PRESSED,
                  vec2_(88 + 1, 192), vec2_(3, 7), vec2_(1), vec2_(3, 6));
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_L+0] = atlas.rects[WTheme_BTN_BIG_DUAL+0];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_L+3] = atlas.rects[WTheme_BTN_BIG_DUAL+3];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_L+6] = atlas.rects[WTheme_BTN_BIG_DUAL+6];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_L__PRESSED+0] = atlas.rects[WTheme_BTN_BIG_DUAL__PRESSED+0];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_L__PRESSED+3] = atlas.rects[WTheme_BTN_BIG_DUAL__PRESSED+3];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_L__PRESSED+6] = atlas.rects[WTheme_BTN_BIG_DUAL__PRESSED+6];
    
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_ROUND_DUAL_R,
                  vec2_(80 + 1, 192), vec2_(3, 4), vec2_(1), vec2_(3, 9));
    set_ninepatch(atlas, tex, WTheme_BTN_BIG_ROUND_DUAL_R__PRESSED,
                  vec2_(88 + 1, 192), vec2_(3, 7), vec2_(1), vec2_(3, 6));
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_R+2] = atlas.rects[WTheme_BTN_BIG_DUAL+2];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_R+5] = atlas.rects[WTheme_BTN_BIG_DUAL+5];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_R+8] = atlas.rects[WTheme_BTN_BIG_DUAL+8];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_R__PRESSED+2] = atlas.rects[WTheme_BTN_BIG_DUAL__PRESSED+2];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_R__PRESSED+5] = atlas.rects[WTheme_BTN_BIG_DUAL__PRESSED+5];
    atlas.rects[WTheme_BTN_BIG_ROUND_DUAL_R__PRESSED+8] = atlas.rects[WTheme_BTN_BIG_DUAL__PRESSED+8];

    set_ninepatch(atlas, tex, WTheme_BTN_RADIO,
                  vec2_(0, 200), vec2_(4, 4), vec2_(1), vec2_(3, 3));
    set_ninepatch(atlas, tex, WTheme_BTN_RADIO__PRESSED,
                  vec2_(8, 200), vec2_(4, 4), vec2_(1), vec2_(3, 3));

    set_ninepatch(atlas, tex, WTheme_BTN_CHECK,
                  vec2_(16, 200), vec2_(4, 4), vec2_(1), vec2_(3, 3));
    set_ninepatch(atlas, tex, WTheme_BTN_CHECK__PRESSED,
                  vec2_(24, 200), vec2_(4, 4), vec2_(1), vec2_(3, 3));

    set_ninepatch(atlas, tex, WTheme_BTN_SWITCH,
                  vec2_(32 + 1, 200 + 1), vec2_(3, 3), vec2_(1), vec2_(10, 3));
    set_ninepatch(atlas, tex, WTheme_BTN_SWITCH__PRESSED,
                  vec2_(48 + 1, 200 + 1), vec2_(10, 3), vec2_(1), vec2_(3, 3));

    // pane
    set_ninepatch(atlas, tex, WTheme_PANE,
                  vec2_(1, 208 + 1), vec2_(2, 2), vec2_(1), vec2_(3, 3));
    set_ninepatch(atlas, tex, WTheme_PANE_ROUND_TOP,
                  vec2_(8 + 1, 208 + 1), vec2_(2, 2), vec2_(1), vec2_(3, 3));
    set_ninepatch(atlas, tex, WTheme_PANE_ROUND,
                  vec2_(16 + 1, 208 + 1), vec2_(2, 2), vec2_(1), vec2_(3, 3));
    set_ninepatch(atlas, tex, WTheme_PANE_ROUND_BOTTOM,
                  vec2_(24 + 1, 208 + 1), vec2_(2, 2), vec2_(1), vec2_(3, 3));

    // sliders
    atlas.rects[WTheme_SLIDER_H_BAR + 0] = vec4_(1, 216, 2, 7);
    atlas.rects[WTheme_SLIDER_H_BAR + 1] = vec4_(1 + 2, 216, 1, 7);
    atlas.rects[WTheme_SLIDER_H_BAR + 2] = vec4_(1 + 3, 216, 2, 7);
    atlas.rects[WTheme_SLIDER_H_BADGE] = vec4_(8, 216, 4, 7);

    atlas.rects[WTheme_SLIDER_V_BAR + 0] = vec4_(16, 216 + 1, 7, 2);
    atlas.rects[WTheme_SLIDER_V_BAR + 1] = vec4_(16, 216 + 3, 7, 1);
    atlas.rects[WTheme_SLIDER_V_BAR + 2] = vec4_(16, 216 + 4, 7, 2);
    atlas.rects[WTheme_SLIDER_V_BADGE] = vec4_(24, 216, 7, 4);


    // custom
    for (int i = 0; i < WTheme_CUSTOM_8_NUM; i++) {
        int col = i % 4;
        int row = i / 4;
        atlas.rects[WTheme_CUSTOM_8 + i] = vec4_(128 + col * 8, 0 + row * 8, 8, 8);
    }
    for (int i = 0; i < WTheme_CUSTOM_16_NUM; i++) {
        int col = i % 2;
        int row = i / 2;
        atlas.rects[WTheme_CUSTOM_16 + i] = vec4_(128 + col * 16, 16 + row * 16, 16, 16);
    }
    for (int i = 0; i < WTheme_CUSTOM_32_NUM; i++) {
        int col = i % 3;
        int row = i / 3;
        atlas.rects[WTheme_CUSTOM_32 + i] = vec4_(128 + 32 + col * 32, 0 + row * 32, 32, 32);
    }
    for (int i = 0; i < WTheme_CUSTOM_64_NUM; i++) {
        int col = i % 2;
        int row = i / 2;
        atlas.rects[WTheme_CUSTOM_64 + i] = vec4_(128 + col * 64, 64 + row * 64, 64, 64);
    }

    //
    // end atlas setup
    //
    return WTheme_new(parent, tex, true, atlas, true);
}

//
// vfuncs
//

osize WTheme__v_op_num(oobj obj)
{
    return WTheme_num(obj);
}

void *WTheme__v_op_at(oobj obj, osize idx)
{
    return WTheme_at(obj, (int) idx);
}


//
// object functions
//

void WTheme_clear(oobj obj)
{
    oobj boxes = RObjBox_boxes(WTheme_ro(obj));
    OArray_clear(boxes);
}

void WTheme_reset_to(oobj obj, int num)
{
    oobj boxes = RObjBox_boxes(WTheme_ro(obj));
    OArray_raw_resize_front(boxes, num);
}

int WTheme_alloc(oobj obj, int num)
{
    OObj_assert(obj, WTheme);
    WTheme *self = obj;
    oobj boxes = RObjBox_boxes(self->ro);
    osize back_idx = OArray_num(boxes) + num;

    // Raw mode is unchecked, inlined and creates empty memory (not cleared)
    // Use with caution.
    // Speeds up appending a little.
    OArray_raw_append_front(boxes, NULL, num);

    struct r_box *boxes_v = OArray_data(boxes, struct r_box);
    for (int i = 0; i < num; i++) {
        struct r_box *b = &boxes_v[i];
        *b = r_box_new(m_2(RTex_size(RObjBox_tex(self->ro))));
    }
    return (int) back_idx;
}

struct r_box *WTheme_at(oobj obj, int back_idx)
{
    oobj boxes = RObjBox_boxes(WTheme_ro(obj));
    osize num = OArray_num(boxes);
    return OArray_at_void(boxes, num - back_idx);
}

int WTheme_num(oobj obj)
{
    oobj boxes = RObjBox_boxes(WTheme_ro(obj));
    return (int) OArray_num(boxes);
}

oobj WTheme_tex(oobj obj)
{
    return RObjBox_tex(WTheme_ro(obj));
}

vec2 WTheme_update_ex(oobj obj, oobj wobj, vec2 lt, vec2 min_size, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WTheme);
    WTheme *self = obj;
    vec2 size;
    WTheme_clear(self);
    size = WObj_update(wobj, lt, min_size, true, self, pointer_fn);
    return size;
}

vec2 WTheme_update(oobj obj, oobj wobj, vec2 lt, vec2 min_size)
{
    return WTheme_update_ex(obj, wobj, lt, min_size, a_pointer);
}


vec2 WTheme_update_full_tex(oobj obj, oobj wobj, oobj tex)
{
    OObj_assert(obj, WTheme);
    WTheme *self = obj;
    WTheme_clear(self);
    vec2 size = WObj_update_full_tex(wobj, tex, true, self, a_pointer);
    return size;
}

vec2 WTheme_update_full_scene(oobj obj, oobj wobj, bool use_safe_zone)
{
    OObj_assert(obj, WTheme);
    WTheme *self = obj;
    WTheme_clear(self);
    vec2 size = WObj_update_full_scene(wobj, use_safe_zone, true, self, a_pointer);
    return size;
}

void WTheme_render_ex(oobj obj, oobj tex, const struct r_proj *opt_proj, bool update)
{
    OObj_assert(obj, WTheme);
    WTheme *self = obj;
    RObj_render_ex(self->ro, tex, opt_proj, update);
}
