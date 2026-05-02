#ifndef X_XVIEWKEYS_H
#define X_XVIEWKEYS_H

/**
 * @file XViewKeys.h
 *
 * object.
 *
 * AView which shows a virtual keyboard.
 *
 * Subclass of the AView object.
 */

#include "m/types/flt.h"
#include "a/AView.h"
#include "w/WTheme.h"


/** object id */
#define XViewKeys_ID AView_ID "XViewKeys."

// forward
struct XViewKeys_key;

/**
 * Callback for special keys, may be set (optional) in struct XViewKeys_key
 * @param viewkeys XViewKeys object
 * @param stream OStream to print into
 * @param col, row key position in OList_at and OArray_at
 * @param key reference to the const key
 */
typedef void (*XViewKeys_key_opt_op_fn)(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key);

struct XViewKeys_key {
    // will print the string to the stream if not NULL
    char *print;
    // if available, called after printing "print" to the stream
    XViewKeys_key_opt_op_fn op;
    // WText text if not NULL
    char *label;    
    // if not WTheme_NONE, used as icon left of the label text
    enum WTheme_indices icon;
    // optional WStyle for that key
    oobj style;
    // used to span the key above multiple cells.
    // <= 0 defaults to 1
    int cols, rows;
    // shifts current grid position
    int offset_cols;
    
    // the generated WBtn after the call to XViewKeys_page_set
    oobj gen_btn;
};

struct XViewKeys_page {
    // OList of OArray of struct XViewKeys_key
    oobj rows;
    // optional WStyle (allocated on rows)
    oobj style;
    // for each key, pass <= 0 to auto calc
    vec2 key_size;
};


/**
 * For an optional filter for the real key stream (not for the virtual!).
 * Say your page only accepts numbers, that filter can filter the real keyboard input for adequate user input.
 * @param viewkeys XViewKeys object
 * @param out_buffer to be set to the filtered output, allocated as o_strlen(in_buffer)+1
 * @param in_buffer current input as string to be filtered
 */
typedef void (*XViewKeys_filter_fn)(oobj viewkeys, char *out_buffer, const char *in_buffer);

typedef struct {
    AView super;

    // To update, call XViewKeys_page_set().
    struct XViewKeys_page page;
    struct XViewKeys_page deferred_page;

    vec2 swipe_distance;
    vec2 swipe_start;

    // for the real physical keyboard
    bool keyboard_enabled;
    XViewKeys_filter_fn opt_keyboard_filter;

    // WTheme and WBox for gui
    oobj theme;
    oobj gui;
    
    // resulting FIFO OStreamArray to read on (writing pushs back new keys...)
    oobj stream;
} XViewKeys;


/**
 * Initializes the object.
 * Creates an AView that renders a virtual keyboard.
 * @param obj XViewKeys object
 * @param parent to inherit from
 * @param page The page to show.
 *             To update, call XViewKeys_page_set().
 *             NULL safe.
 * @param move_page if true, page will get o_move'd into this object
 * @return obj casted as XViewKeys
 */
XViewKeys *XViewKeys_init(oobj obj, oobj parent, struct XViewKeys_page page, bool move_page);

/**
 * Creates a new XViewKeys object.
 * Creates an AView that renders a virtual keyboard.
 * @param parent to inherit from
 * @param page The page to show.
 *             To update, call XViewKeys_page_set().
 * @param move_page if true, page will get o_move'd into this object
 * @return The new object
 */
o_inline XViewKeys *XViewKeys_new(oobj parent, struct XViewKeys_page page, bool move_page)
{
    OObj_DECL_IMPL_NEW(XViewKeys, parent, page, move_page);
}

/**
 * Creates a new XViewKeys object.
 * Creates an AView that renders a virtual keyboard.
 * Sets the page to XViewKeys_factory_page_number and filter to XViewKeys_factory_filter_number.
 * @param parent to inherit from
 * @param style NULL or a shared WStyle for the keys.
 *              If NULL, XViewKeys_factory_style is used
 * @param ctrl_style NULL or a shared WStyle for ctrl keys like '\n' or XViewKeys_FACTORY_CURSOR_UP.
 *               If NULL, XViewKeys_factory_style_ctrl is used
 * @return The new object
 */
XViewKeys *XViewKeys_new_number(oobj parent, oobj style, oobj ctrl_style);

/**
 * Creates a new XViewKeys object.
 * Creates an AView that renders a virtual keyboard.
 * Sets the page to XViewKeys_factory_page_hex and filter to XViewKeys_factory_filter_hex.
 * @param parent to inherit from
 * @param style NULL or a shared WStyle for the keys.
 *              If NULL, XViewKeys_factory_style is used
 * @param ctrl_style NULL or a shared WStyle for ctrl keys like '\n' or XViewKeys_FACTORY_CURSOR_UP.
 *               If NULL, XViewKeys_factory_style_ctrl is used
 * @return The new object
 */
XViewKeys *XViewKeys_new_hex(oobj parent, oobj style, oobj ctrl_style);

/**
 * Creates a new XViewKeys object.
 * Creates an AView that renders a virtual keyboard.
 * Keyboard for general ascii text input usage. Swaps pages internally
 * @param parent to inherit from
 * @param style NULL or a shared WStyle for the keys.
 *              If NULL, XViewKeys_factory_style is used
 * @param ctrl_style NULL or a shared WStyle for ctrl keys like '\n' or XViewKeys_FACTORY_CURSOR_UP.
 *               If NULL, XViewKeys_factory_style_ctrl is used
 * @return The new object
 */
XViewKeys *XViewKeys_new_text(oobj parent, oobj style, oobj ctrl_style);

/**
 * Creates a new XViewKeys object.
 * Creates an AView that renders a virtual keyboard.
 * Keyboard specialized for terminal usage. Swaps pages internally
 * @param parent to inherit from
 * @param style NULL or a shared WStyle for the keys.
 *              If NULL, XViewKeys_factory_style is used
 * @param ctrl_style NULL or a shared WStyle for ctrl keys like '\n' or XViewKeys_FACTORY_CURSOR_UP.
 *               If NULL, XViewKeys_factory_style_ctrl is used
 * @return The new object
 */
XViewKeys *XViewKeys_new_terminal(oobj parent, oobj style, oobj ctrl_style);


//
// virtual implementations
//

void XViewKeys__v_setup(oobj view);

void XViewKeys__v_update(oobj view, oobj tex, float dt);

void XViewKeys__v_render(oobj view, oobj tex, float dt);


//
// object functions:
//


/**
 * @param obj XViewKeys object
 * @return distance for a swipe cursor print (touch down a key, move that amount of units for a cursor print).
 *         default = vec2_(16.0).
 *         pass <= 0 to turn off (only X: vec2_(16, 0))
 *        "\x1B[A" Cursor Up
 *        "\x1B[B" Cursor Down
 *        "\x1B[C" Cursor Right
 *        "\x1B[D" Cursor Left
 */
OObj_DECL_GETSET(XViewKeys, vec2, swipe_distance)

/**
 * @param obj XViewKeys object
 * @return enable input to stream from the real physical keyboard (defaults to true)
 */
OObj_DECL_GETSET(XViewKeys, bool, keyboard_enabled)

/**
 * @param obj XViewKeys object
 * @return enable input to stream from the real physical keyboard
 */
OObj_DECL_SET(XViewKeys, XViewKeys_filter_fn, opt_keyboard_filter)

/**
 * @param obj XViewKeys object
 * @return currently shown page
 * @note To update the keys, call XViewKeys_page_set()
 */
OObj_DECL_GET(XViewKeys, struct XViewKeys_page, page)

/**
 * Updates the internal keyboard
 * @param obj XViewKeys object
 * @param page The new page to show, OList of OArray of struct XViewKeys_key.
 *             OList is rows from top, OArray is cols from left.
 * @note will call o_del(self->page.rows) to delete the old page
 * @return == page
 */
struct XViewKeys_page XViewKeys_page_set(oobj obj, struct XViewKeys_page page);

/**
 * Updates the internal keyboard.
 * Defers the update to the next frame.
 * Useful to switch pages by a key press of an old page
 * @param obj XViewKeys object
 * @note will call o_del(self->page.row) to delete the old page and also o_del(self->defered_page.row) to delete multiple calls
 */
void XViewKeys_page_set_deferred(oobj obj, struct XViewKeys_page page);

/**
 * @param obj XViewKeys object
 * @return resulting FIFO OStreamArray to read on (writing pushs back new keys...)
 * @note stream may contain "\b" for backspace or escape codes like "\x1B[A" for cursor Up
 *       escaped codes may be of:
 *       "\x1B[A" Cursor Up
 *       "\x1B[B" Cursor Down
 *       "\x1B[C" Cursor Right
 *       "\x1B[D" Cursor Left
 */
OObj_DECL_GET(XViewKeys, oobj, stream)



//
// Factory
//

// special key codes for none visible ascii chars
// if paste is not available on the system, it gets replaced with ' ' (space)
enum XViewKeys_factory_code {
    XViewKeys_F_END = '\0',
    XViewKeys_F_NEXT_ROW = 0xff01,
    XViewKeys_F_PASTE = 0xff02,
    XViewKeys_F_EMPTY = 0xff03,
    XViewKeys_F_UP = 0xff10,
    XViewKeys_F_DOWN = 0xff11,
    XViewKeys_F_LEFT = 0xff12,
    XViewKeys_F_RIGHT = 0xff13,
    XViewKeys_F_SHIFT = 0xff20,
    XViewKeys_F_CAPS = 0xff21,
    XViewKeys_F_SHIFT_INV = 0xff22,
    XViewKeys_F_ALT = 0xff25,
    XViewKeys_F_TXT = 0xff26,
    XViewKeys_F_ROMAN_1 = 0xff30,
    XViewKeys_F_ROMAN_2 = 0xff31,
};

#ifdef XViewKeys_F__ENABLE_SHORT
# define XK_END  XViewKeys_F_END
# define XK_ROW  XViewKeys_F_NEXT_ROW
# define XK_PAS  XViewKeys_F_PASTE
# define XK_EMP  XViewKeys_F_EMPTY
# define XK_U    XViewKeys_F_UP
# define XK_D    XViewKeys_F_DOWN
# define XK_L    XViewKeys_F_LEFT
# define XK_R    XViewKeys_F_RIGHT
# define XK_S    XViewKeys_F_SHIFT
# define XK_C    XViewKeys_F_CAPS
# define XK_S_I  XViewKeys_F_SHIFT_INV
# define XK_ALT  XViewKeys_F_ALT
# define XK_TXT  XViewKeys_F_TXT
# define XK_RO1  XViewKeys_F_ROMAN_1
# define XK_RO2  XViewKeys_F_ROMAN_2
#endif

/**
 * @param parent to inherit from
 * @param layout an array of keys like:
 *               int layout[] = {
 *                    'Q', 'W', 'E', 'R', 'T', 'Y', ..., XViewKeys_F_NEXT_ROW,
 *                    'A', ... '\n', XViewKeys_F_NEXT_ROW,
 *                    ' ', ' ', ' ', XViewKeys_F_LEFT, XViewKeys_F_RIGHT, XViewKeys_F_END
 *               };
 *               Using multiple same layout codes side by side (like space ' ' above), enlarges those keys.
 *               This works horizontal, but also vertical and as aabb, but not as say + or smth.
 *                   (doing an enlargment like + is not checked, just dont do it :D)
 *               The factory codes _SHIFT and below DO NOT CALL ANYTHING YET.
 *               Create XViewKeys_key_opt_op_fn functions and set em manually,
 *               by accessing the page OList and OArray at those positions.
 * @param style NULL or a shared WStyle for the keys.
 *              If NULL, XViewKeys_factory_style is used
 * @param ctrl_style NULL or a shared WStyle for ctrl keys like '\n' or XViewKeys_FACTORY_CURSOR_UP.
 *               If NULL, XViewKeys_factory_style_ctrl is used
 * @return page
 */
struct XViewKeys_page XViewKeys_factory_page(oobj parent, const int *layout, oobj style, oobj ctrl_style);

/**
 * Creates a simple single key page for number input.
 * Contains also backspace, cursors and math symbols like + - * /
 * @param parent to inherit from
 * @param style NULL or a shared WStyle for the keys.
 *              If NULL, XViewKeys_factory_style is used
 * @param ctrl_style NULL or a shared WStyle for ctrl keys like '\n' or XViewKeys_FACTORY_CURSOR_UP.
 *               If NULL, XViewKeys_factory_style_ctrl is used
 * @return page
 */
struct XViewKeys_page XViewKeys_factory_page_number(oobj parent, oobj style, oobj ctrl_style);

/**
 * Filter to be used as number filter for XViewKeys_opt_keyboard_filter
 */
void XViewKeys_factory_filter_number(oobj viewkeys, char *out_buffer, const char *in_buffer);

/**
 * Creates a simple single key page for hex number input.
 * Contains also backspace, cursors and '#'
 * @param parent to inherit from
 * @param style NULL or a shared WStyle for the keys.
 *              If NULL, XViewKeys_factory_style is used
 * @param ctrl_style NULL or a shared WStyle for ctrl keys like '\n' or XViewKeys_FACTORY_CURSOR_UP.
 *               If NULL, XViewKeys_factory_style_ctrl is used
 * @return page
 */
struct XViewKeys_page XViewKeys_factory_page_hex(oobj parent, oobj style, oobj ctrl_style);

/**
 * Filter to be used as hex filter for XViewKeys_opt_keyboard_filter
 */
void XViewKeys_factory_filter_hex(oobj viewkeys, char *out_buffer, const char *in_buffer);

/**
 * @param parent to inherit from
 * @return WStyle as default style for the keys, passed in XViewKeys_page_set
 */
oobj XViewKeys_factory_style(oobj parent);

/**
 * @param parent to inherit from
 * @return WStyle for ctrl keys, to be passed to XViewKeys_factory_page as default
 */
oobj XViewKeys_factory_style_ctrl(oobj parent);


#endif //X_XVIEWKEYS_H
