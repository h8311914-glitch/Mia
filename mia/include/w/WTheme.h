#ifndef W_WTHEME_H
#define W_WTHEME_H


/**
 * @file WTheme.h
 *
 * Object
 *
 * A theme bundles a render object for boxes, its texture and an atlas for the sprites.
 * Used to render the default widgets.
 *
 * Apart from other objects in the 'w' module, WTheme is >NOT< a subobject of WObj.
 *
 * Operators:
 * o_num -> RObjBox_num
 * o_at -> RObjBox_at
 */


#include "o/OObj.h"
#include "r/box.h"
#include "r/proj.h"
#include "a/pointer.h"
#include "u/atlas.h"

/** object id */
#define WTheme_ID OObj_ID "WTheme."


#define WTheme_FONT_NUM 97

// 3x3 rows, row major from top left (lt, ct, rt; lc, cc, rc; lb, cb, rb)
#define WTheme_BTN__SPRITES 9
// 3x3 rows, row major from top left (lt, ct, rt; lc, cc, rc; lb, cb, rb)
#define WTheme_PANE__SPRITES 9
// 3 sprites, start, center, end
#define WTheme_SLIDER_BAR__SPRITES 3

#define WTheme_CUSTOM_8_NUM 8
#define WTheme_CUSTOM_16_NUM 6
#define WTheme_CUSTOM_32_NUM 6
#define WTheme_CUSTOM_64_NUM 2

enum WTheme_indices {
    WTheme_NONE,
    WTheme_FONT,
    WTheme_FONT_SHADOW = WTheme_FONT + WTheme_FONT_NUM,
    WTheme_WHITE = WTheme_FONT_SHADOW + WTheme_FONT_NUM,

    // 5x5 icons white
    WTheme_ICON_SMALL_WND_MIN,
    WTheme_ICON_SMALL_WND_WND,
    WTheme_ICON_SMALL_WND_MAX,
    WTheme_ICON_SMALL_WND_CROSS,
    WTheme_ICON_SMALL_PLUS,
    WTheme_ICON_SMALL_MINUS,
    WTheme_ICON_SMALL_DOTS,
    WTheme_ICON_SMALL_DIR_LEFT,
    WTheme_ICON_SMALL_DIR_RIGHT,
    WTheme_ICON_SMALL_DIR_UP,
    WTheme_ICON_SMALL_DIR_DOWN,
    WTheme_ICON_SMALL_ROMAN_1,
    WTheme_ICON_SMALL_ROMAN_2,
    WTheme_ICON_SMALL_CAM,

    // 7x7 icons white (same as above with a transparent border)
    WTheme_ICON_WND_MIN,
    WTheme_ICON_WND_WND,
    WTheme_ICON_WND_MAX,
    WTheme_ICON_WND_CROSS,
    WTheme_ICON_PLUS,
    WTheme_ICON_MINUS,
    WTheme_ICON_DOTS,
    WTheme_ICON_DIR_LEFT,
    WTheme_ICON_DIR_RIGHT,
    WTheme_ICON_DIR_UP,
    WTheme_ICON_DIR_DOWN,
    WTheme_ICON_ROMAN_1,
    WTheme_ICON_ROMAN_2,
    WTheme_ICON_CAM,

    // 7x7 icons white
    WTheme_ICON_CROSS,
    WTheme_ICON_CHECK,
    WTheme_ICON_ARROW_LEFT,
    WTheme_ICON_ARROW_RIGHT,
    WTheme_ICON_ARROW_UP,
    WTheme_ICON_ARROW_DOWN,
    WTheme_ICON_EXCLAMATION,
    WTheme_ICON_QUESTION,
    WTheme_ICON_TAB,
    WTheme_ICON_ENTER,
    WTheme_ICON_SHIFT,
    WTheme_ICON_CAPS,
    WTheme_ICON_SHIFT_INV,

    // 7x7 icons white
    WTheme_ICON_RECORD,
    WTheme_ICON_PLAY,
    WTheme_ICON_PAUSE,
    WTheme_ICON_STOP,
    WTheme_ICON_RFF,
    WTheme_ICON_FF,

    // 7x7 icons for various widgets
    WTheme_ICON_PICKER_X,
    WTheme_ICON_PICKER_Y,
    WTheme_ICON_PICKER_XY,

    // 7x7 icons colored
    WTheme_ICON_GEAR,
    WTheme_ICON_SAVE,
    WTheme_ICON_LOAD,
    WTheme_ICON_SEARCH,

    // 7x7 icons colored file types
    WTheme_ICON_FILE,
    WTheme_ICON_FILE_TXT,
    WTheme_ICON_FILE_MUS,
    WTheme_ICON_FILE_IMG,
    WTheme_ICON_FILE_C,
    WTheme_ICON_FILE_H,

    WTheme_BTN_FLAT,
    WTheme_BTN_FLAT__PRESSED = WTheme_BTN_FLAT + WTheme_BTN__SPRITES,
    WTheme_BTN_FLAT_ROUND = WTheme_BTN_FLAT + WTheme_BTN__SPRITES*2,
    WTheme_BTN_FLAT_ROUND__PRESSED = WTheme_BTN_FLAT_ROUND + WTheme_BTN__SPRITES,
    WTheme_BTN_DEF = WTheme_BTN_FLAT_ROUND + WTheme_BTN__SPRITES*2,
    WTheme_BTN_DEF__PRESSED = WTheme_BTN_DEF + WTheme_BTN__SPRITES,
    WTheme_BTN_DEF_ROUND = WTheme_BTN_DEF + WTheme_BTN__SPRITES*2,
    WTheme_BTN_DEF_ROUND__PRESSED = WTheme_BTN_DEF_ROUND + WTheme_BTN__SPRITES,
    
    WTheme_BTN_DEF_DUAL = WTheme_BTN_DEF_ROUND + WTheme_BTN__SPRITES*2,
    WTheme_BTN_DEF_DUAL__PRESSED = WTheme_BTN_DEF_DUAL + WTheme_BTN__SPRITES,
    WTheme_BTN_DEF_DUAL_L = WTheme_BTN_DEF_DUAL + WTheme_BTN__SPRITES*2,
    WTheme_BTN_DEF_DUAL_L__PRESSED = WTheme_BTN_DEF_DUAL_L + WTheme_BTN__SPRITES,
    WTheme_BTN_DEF_DUAL_R = WTheme_BTN_DEF_DUAL_L + WTheme_BTN__SPRITES*2,
    WTheme_BTN_DEF_DUAL_R__PRESSED = WTheme_BTN_DEF_DUAL_R + WTheme_BTN__SPRITES,
    WTheme_BTN_DEF_ROUND_DUAL_L = WTheme_BTN_DEF_DUAL_R + WTheme_BTN__SPRITES*2,
    WTheme_BTN_DEF_ROUND_DUAL_L__PRESSED = WTheme_BTN_DEF_ROUND_DUAL_L + WTheme_BTN__SPRITES,
    WTheme_BTN_DEF_ROUND_DUAL_R = WTheme_BTN_DEF_ROUND_DUAL_L + WTheme_BTN__SPRITES*2,
    WTheme_BTN_DEF_ROUND_DUAL_R__PRESSED = WTheme_BTN_DEF_ROUND_DUAL_R + WTheme_BTN__SPRITES,
    
    WTheme_BTN_BIG = WTheme_BTN_DEF_ROUND_DUAL_R + WTheme_BTN__SPRITES*2,
    WTheme_BTN_BIG__PRESSED = WTheme_BTN_BIG + WTheme_BTN__SPRITES,
    WTheme_BTN_BIG_ROUND = WTheme_BTN_BIG + WTheme_BTN__SPRITES*2,
    WTheme_BTN_BIG_ROUND__PRESSED = WTheme_BTN_BIG_ROUND + WTheme_BTN__SPRITES,
    
    WTheme_BTN_BIG_DUAL = WTheme_BTN_BIG_ROUND + WTheme_BTN__SPRITES*2,
    WTheme_BTN_BIG_DUAL__PRESSED = WTheme_BTN_BIG_DUAL + WTheme_BTN__SPRITES,
    WTheme_BTN_BIG_DUAL_L = WTheme_BTN_BIG_DUAL + WTheme_BTN__SPRITES*2,
    WTheme_BTN_BIG_DUAL_L__PRESSED = WTheme_BTN_BIG_DUAL_L + WTheme_BTN__SPRITES,
    WTheme_BTN_BIG_DUAL_R = WTheme_BTN_BIG_DUAL_L + WTheme_BTN__SPRITES*2,
    WTheme_BTN_BIG_DUAL_R__PRESSED = WTheme_BTN_BIG_DUAL_R + WTheme_BTN__SPRITES,
    WTheme_BTN_BIG_ROUND_DUAL_L = WTheme_BTN_BIG_DUAL_R + WTheme_BTN__SPRITES*2,
    WTheme_BTN_BIG_ROUND_DUAL_L__PRESSED = WTheme_BTN_BIG_ROUND_DUAL_L + WTheme_BTN__SPRITES,
    WTheme_BTN_BIG_ROUND_DUAL_R = WTheme_BTN_BIG_ROUND_DUAL_L + WTheme_BTN__SPRITES*2,
    WTheme_BTN_BIG_ROUND_DUAL_R__PRESSED = WTheme_BTN_BIG_ROUND_DUAL_R + WTheme_BTN__SPRITES,
    
    
    WTheme_BTN_RADIO = WTheme_BTN_BIG_ROUND_DUAL_R + WTheme_BTN__SPRITES*2,
    WTheme_BTN_RADIO__PRESSED = WTheme_BTN_RADIO + WTheme_BTN__SPRITES,
    WTheme_BTN_CHECK = WTheme_BTN_RADIO + WTheme_BTN__SPRITES*2,
    WTheme_BTN_CHECK__PRESSED = WTheme_BTN_CHECK + WTheme_BTN__SPRITES,
    WTheme_BTN_SWITCH = WTheme_BTN_CHECK + WTheme_BTN__SPRITES*2,
    WTheme_BTN_SWITCH__PRESSED = WTheme_BTN_SWITCH + WTheme_BTN__SPRITES,
    WTheme_PANE = WTheme_BTN_SWITCH + WTheme_BTN__SPRITES*2,
    WTheme_PANE_ROUND = WTheme_PANE + WTheme_PANE__SPRITES,
    WTheme_PANE_ROUND_TOP = WTheme_PANE_ROUND + WTheme_PANE__SPRITES,
    WTheme_PANE_ROUND_BOTTOM = WTheme_PANE_ROUND_TOP + WTheme_PANE__SPRITES,
    WTheme_SLIDER_H_BAR = WTheme_PANE_ROUND_BOTTOM + WTheme_PANE__SPRITES,
    WTheme_SLIDER_H_BADGE = WTheme_SLIDER_H_BAR + WTheme_SLIDER_BAR__SPRITES,
    WTheme_SLIDER_V_BAR,
    WTheme_SLIDER_V_BADGE = WTheme_SLIDER_V_BAR + WTheme_SLIDER_BAR__SPRITES,
    WTheme_CUSTOM_8,
    WTheme_CUSTOM_16 = WTheme_CUSTOM_8 + WTheme_CUSTOM_8_NUM,
    WTheme_CUSTOM_32 = WTheme_CUSTOM_16 + WTheme_CUSTOM_16_NUM,
    WTheme_CUSTOM_64 = WTheme_CUSTOM_32 + WTheme_CUSTOM_32_NUM,
    WTheme_ENUM_MAX = WTheme_CUSTOM_64 + WTheme_CUSTOM_64_NUM
};

/**
 * @param c character
 * @return WTheme atlas idx
 */
o_inline int WTheme_char_idx(char glyph)
{
    ou8 c = *((ou8*)&glyph);
    if (c < ' ') {
        c = ' ';
    } else if (c > '~') {
        c = '~'+1;
    }
    return WTheme_FONT + c - ' ';
}

/**
 * @param c character
 * @return WTheme atlas idx
 */
o_inline int WTheme_char_shadow_idx(char glyph)
{
    ou8 c = *((ou8*)&glyph);
    if (c < ' ') {
        c = ' ';
    } else if (c > '~') {
        c = '~'+1;
    }
    return WTheme_FONT_SHADOW + c - ' ';
}


typedef struct {
    OObj super;

    // RObjBox with _color shader (and internal OArray changed for dyn front + using raw mode)
    oobj ro;
    struct u_atlas atlas;
} WTheme;


/**
 * Initializes the object
 * @param obj WTheme object
 * @param parent to inherit from
 * @param tex theme batch atlas texture
 * @param move_tex if true, tex is o_move'd into the internal ro
 * @param atlas atlas for the tex, according to the WTheme atlas indices
 * @param move_atlas if true, atlas is move'd into this object
 * @return obj casted as WTheme
 */
WTheme *WTheme_init(oobj obj, oobj parent, oobj tex, bool move_tex,
                    struct u_atlas atlas, bool move_atlas);

/**
 * Creates a new WTheme object
 * @param parent to inherit from
 * @param tex theme batch atlas texture
 * @param move_tex if true, tex is o_move'd into the internal ro
 * @param atlas atlas for the tex, according to the WTheme atlas indices
 * @param move_atlas if true, atlas is move'd into this object
 * @return The new object
 */
o_inline WTheme *WTheme_new(oobj parent, oobj tex, bool move_tex,
                   struct u_atlas atlas, bool move_atlas)
{
    OObj_DECL_IMPL_NEW(WTheme, parent, tex, move_tex, atlas, move_atlas);
}

/**
 * @return default tiny theme
 */
WTheme *WTheme_new_tiny(oobj parent);


//
// virtual implementations:
//

/**
 * virtual operator function
 * @param obj WTheme object
 * @return number of r_boxes
 */
osize WTheme__v_op_num(oobj obj);

/**
 * virtual operator function
 * @param obj WTheme object
 * @return r_box at given idx
 */
void *WTheme__v_op_at(oobj obj, osize idx);


//
// object functions
//


/**
 * @param obj WTheme object
 * @return internal render object RObjBox (with _color shader)
 */
OObj_DECL_GET(WTheme, oobj, ro)

/**
 * @param obj WTheme object
 * @return theme atlas
 */
OObj_DECL_GETSETREF(WTheme, struct u_atlas, atlas)

/**
 * Clears / resets all internal boxes
 * @param obj WTheme object
 */
void WTheme_clear(oobj obj);

/**
 * Resets to a specific number of allocated boxes.
 * Useful for internal rezupdating.
 *     First save num with WTheme_num
 *     On reupdate call this and reupdate
 * @param obj WTheme object
 * @param num to reset to
 */
void WTheme_reset_to(oobj obj, int num);

/**
 * Allocates some new boxes
 * @param obj WTheme object
 * @param num to allocate
 * @return back_idx for the first allocated
 */
int WTheme_alloc(oobj obj, int num);

/**
 * @param obj WTheme object
 * @return r_box at back_idx
 */
struct r_box *WTheme_at(oobj obj, int back_idx);

/**
 * @param obj WTheme object
 * @return number of r_box'es allocated
 */
int WTheme_num(oobj obj);


/**
 * @param obj WTheme object
 * @return RTex object, just a wrapper to RObjBox_tex(WTheme_ro(obj))
 */
oobj WTheme_tex(oobj obj);

/**
 * @param obj WTheme object
 * @param btn_idx atlas index
 * @return the offset between pressed and unpressed for the inner field (the children)
 */
o_inline vec2 WTheme_btn_press_offset(oobj obj, int btn_idx)
{
    struct u_atlas atlas = WTheme_atlas(obj);
    return vec2_sub_v(u_atlas_size(atlas, btn_idx+WTheme_BTN__SPRITES),
                      u_atlas_size(atlas, btn_idx));
}


/**
 * Calls WTheme_clear and then WObj_update
 * @param obj WTheme object
 * @param wobj WObj object to update (root widget)
 * @param lt left top
 * @param min_size minimal size that may be used for this widget
 * @param pointer_fn pointer function to use, may get overridden with self->pointer_fn
 *                   in most cases "a_pointer" (or "a_pointer_p" if rendered onto the p matrix of the cam)
 *                   asserts not NULL
 * @return used wobj size
 */
vec2 WTheme_update_ex(oobj obj, oobj wobj, vec2 lt, vec2 min_size, a_pointer__fn pointer_fn);

/**
 * Calls WTheme_clear and then WObj_update
 * @param obj WTheme object
 * @param wobj WObj object to update (root widget)
 * @param lt left top
 * @param min_size minimal size that may be used for this widget
 * @return used wobj size
 * @note uses the default a_pointer
 */
vec2 WTheme_update(oobj obj, oobj wobj, vec2 lt, vec2 min_size);

/**
 * Calls WTheme_clear and then WObj_update_full_tex
 * @param obj WTheme object
 * @param wobj WObj object to update (root widget)
 * @param tex RTex object to gain r_proj from
 * @return used stack size
 * @note uses the default a_pointer
 */
vec2 WTheme_update_full_tex(oobj obj, oobj wobj, oobj tex);

/**
 * Calls WTheme_clear and then WObj_update_full_scene
 * @param obj WTheme object
 * @param wobj WObj object to update (root widget)
* @param use_safe_zone if true, calls AScene_safe_padding(a_scene()) and pads into the safe zone (if unsafe)
 * @return used stack size
 * @note uses the default a_pointer
 * @note uses the safe zone
 */
vec2 WTheme_update_full_scene(oobj obj, oobj wobj, bool use_safe_zone);


/**
 * Renders the whole WTheme.
 * Extended version.
 * @param obj WTheme object
 * @param tex RTex to render in, or NULL to use the back buffer
 * @param opt_proj the camera projection to use (vp), if NULL: RTex_proj(tex) is used instead
 * @param update if true: calls RObj_update first on the internal render object 
 */
void WTheme_render_ex(oobj obj, oobj tex, const struct r_proj *opt_proj, bool update);

/**
 * Renders the whole WTheme.
 * Simpler default version.
 * Renders the object to the bound frame buffer
 * @param obj RObj object
 * @param tex RTex to render in, or NULL to use the back buffer
 * @note just calls WTheme_render_ex(obj, tex, NULL, true, post_clear)
 */
o_inline void WTheme_render(oobj obj, oobj tex)
{
    WTheme_render_ex(obj, tex, NULL, true);
}

#endif //W_WTHEME_H
