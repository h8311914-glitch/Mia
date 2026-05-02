#define XViewKeys_F__ENABLE_SHORT

#include "x/XViewKeys.h"
#include "o/OObj_builder.h"
#include "o/OStreamArray.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "o/OEvent.h"
#include "o/ODelcallback.h"
#include "o/str.h"
#include "o/utils.h"
#include "r/RTex.h"
#include "r/RCam.h"
#include "a/input.h"
#include "a/rumble.h"
#include "w/WTheme.h"
#include "w/WStyle.h"
#include "w/WAlign.h"
#include "w/WBox.h"
#include "w/WGrid.h"
#include "w/WBtn.h"
#include "w/WIcon.h"
#include "w/WText.h"

#include "w/WColor.h"

#undef O_LOG_LIB
#define O_LOG_LIB "x"
#include "o/log.h"
#include "o/timer.h"

#define KEYZ_LONG_PRESS_TIME 0.66
#define KEYZ_LONG_PRESS_NEXT_TIME 0.1


#define KEYZ_PAGE_INJECT "XViewKeys_page_context"

struct keyz_page_context {
    oobj style;
    oobj ctrl_style;
};

//
// private
//


struct keyz_btn_context {
    XViewKeys *self;
    struct XViewKeys_key *key;
    int col;
    int row;
};

static void keyz_key_event(oobj btn)
{
    struct keyz_btn_context *C = o_user(btn);
    if (C->key->print) {
        OStream_print(C->self->stream, C->key->print);
    }
    if (C->key->op) {
        C->key->op(C->self, C->self->stream, C->row, C->col, C->key);
    }
    // reset down time, so next long pressed time is at LONG_PRESS_NEXT_TIME
    ou64 timer = WBtn_down_timer_reset(btn);
    timer -= (o_timer_freq_s() * (KEYZ_LONG_PRESS_TIME - KEYZ_LONG_PRESS_NEXT_TIME));
    WBtn_down_timer_set(btn, timer);
}

static int keyz_add_key(XViewKeys *self, int row_grid, int col_grid, int row_i, int col_i, struct XViewKeys_key *key)
{
    oobj btn = WBtn_new(self->gui);
    key->gen_btn = btn;
    col_grid += key->offset_cols;
    int cols = o_max(key->cols, 1);
    int rows = o_max(key->rows, 1);
    WGrid_child_cell_set(btn, ivec4_(col_grid, row_grid, cols, rows));
    if(key->style) {
        WObj_style_set(btn, key->style, true);
    }
    WBtn_auto_mode_set(btn, WBtn_auto_CLICKED_AND_LONG_PRESSED);
    WBtn_auto_event_set(btn, keyz_key_event);
    WBtn_long_pressed_time_set(btn, KEYZ_LONG_PRESS_TIME);
    struct keyz_btn_context *C = o_user_new0(btn, struct keyz_btn_context, 1);
    C->self = self;
    C->key = key;
    C->col = col_i;
    C->row = row_i;
    oobj align = WAlign_new_center(btn);
    WObj_min_size_set(align, vec2_(7,7));
    oobj label_parent = align;
    if (key->icon) {
        label_parent = WBox_new(align, WBox_H);
        WIcon_new(label_parent, key->icon);
    }
    if (key->label) {
        oobj label = WText_new(label_parent, key->label);
    }
    return cols + key->offset_cols;
}

static ivec2 keyz_page_size(oobj page)
{
    int rows = OList_num(page);
    int cols = 0;
    for(osize r=0; r<rows; r++) {
        osize c = OArray_num(OList_at(page, r));
        cols = o_max(cols, c);
    }
    return ivec2_(cols, rows);
}


static void keyz_add_row(XViewKeys *self, int row_grid, int row_i, oobj array)
{
    int col_grid = 0;
    for (osize col_i=0; col_i<OArray_num(array); col_i++) {
        struct XViewKeys_key *key = OArray_at(array, col_i, struct XViewKeys_key);
        col_grid += keyz_add_key(self, row_grid, col_grid, row_i, col_i, key);
    }
}


//
// public
//

XViewKeys *XViewKeys_init(oobj obj, oobj parent, struct XViewKeys_page page, bool move_page) {

    AView *super = obj;
    XViewKeys *self = obj;
    o_clear(self, sizeof *self, 1);

    AView_init(obj, parent, XViewKeys__v_setup, XViewKeys__v_update, XViewKeys__v_render);
    OObj_id_set(self, XViewKeys_ID);


    self->theme = WTheme_new_tiny(self);

    self->swipe_distance = vec2_(16);
    self->keyboard_enabled = true;

    self->stream = OStreamArray_new(self, OArray_new_dyn(self, NULL, sizeof(char), 0, 256),
                                    true, OStreamArray_FIFO);

    if (page.rows) {
        if (move_page) {
            o_move(page.rows, self);
        }
        XViewKeys_page_set(self, page);
    }

    // vfuncs

    return self;
}


XViewKeys *XViewKeys_new_number(oobj parent, oobj style, oobj ctrl_style)
{
    struct XViewKeys_page page = XViewKeys_factory_page_number(parent, style, ctrl_style);
    XViewKeys *self = XViewKeys_new(parent, page, true);
    self->opt_keyboard_filter = XViewKeys_factory_filter_number;
    return self;
}

XViewKeys *XViewKeys_new_hex(oobj parent, oobj style, oobj ctrl_style)
{
    struct XViewKeys_page page = XViewKeys_factory_page_hex(parent, style, ctrl_style);
    XViewKeys *self = XViewKeys_new(parent, page, true);
    self->opt_keyboard_filter = XViewKeys_factory_filter_hex;
    return self;
}

//
// new_text stuff
//

static struct XViewKeys_page keyz_text_page_caps(oobj parent, oobj style, oobj ctrl_style);
static struct XViewKeys_page keyz_text_page_shift(oobj parent, oobj style, oobj ctrl_style);
static struct XViewKeys_page keyz_text_page_lower(oobj parent, oobj style, oobj ctrl_style);
static struct XViewKeys_page keyz_text_page_alt_1(oobj parent, oobj style, oobj ctrl_style);
static struct XViewKeys_page keyz_text_page_alt_2(oobj parent, oobj style, oobj ctrl_style);
static void keyz_text_to_caps(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    struct oobj_opt inject = OObj_find(viewkeys, OObj, KEYZ_PAGE_INJECT, 0);
    if (!inject.o || !o_user(inject.o)) {
        return;
    }
    struct keyz_page_context *C = o_user(inject.o);

    XViewKeys_page_set_deferred(viewkeys, keyz_text_page_caps(viewkeys, C->style, C->ctrl_style));
}
static void keyz_text_to_shift(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    struct oobj_opt inject = OObj_find(viewkeys, OObj, KEYZ_PAGE_INJECT, 0);
    if (!inject.o || !o_user(inject.o)) {
        return;
    }
    struct keyz_page_context *C = o_user(inject.o);

    XViewKeys_page_set_deferred(viewkeys, keyz_text_page_shift(viewkeys, C->style, C->ctrl_style));
}
static void keyz_text_to_lower(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    struct oobj_opt inject = OObj_find(viewkeys, OObj, KEYZ_PAGE_INJECT, 0);
    if (!inject.o || !o_user(inject.o)) {
        return;
    }
    struct keyz_page_context *C = o_user(inject.o);

    XViewKeys_page_set_deferred(viewkeys, keyz_text_page_lower(viewkeys, C->style, C->ctrl_style));
}
static void keyz_text_to_alt_1(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    struct oobj_opt inject = OObj_find(viewkeys, OObj, KEYZ_PAGE_INJECT, 0);
    if (!inject.o || !o_user(inject.o)) {
        return;
    }
    struct keyz_page_context *C = o_user(inject.o);

    XViewKeys_page_set_deferred(viewkeys, keyz_text_page_alt_1(viewkeys, C->style, C->ctrl_style));
}
static void keyz_text_to_alt_2(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    struct oobj_opt inject = OObj_find(viewkeys, OObj, KEYZ_PAGE_INJECT, 0);
    if (!inject.o || !o_user(inject.o)) {
        return;
    }
    struct keyz_page_context *C = o_user(inject.o);

    XViewKeys_page_set_deferred(viewkeys, keyz_text_page_alt_2(viewkeys, C->style, C->ctrl_style));
}

static struct XViewKeys_page keyz_text_page_caps(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '\b', XK_ROW,
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\n', '\n',XK_ROW,
        XK_S_I, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', XK_U, '.', XK_ROW,
        XK_ALT, ' ', ' ', ' ', ' ', ' ', ' ', ' ', XK_L, XK_D, XK_R,
        XK_END,
    };
    struct XViewKeys_page page = XViewKeys_factory_page(parent, layout, style, ctrl_style);
    struct XViewKeys_key *shift = OArray_at(OList_at(page.rows, 2), 0, struct XViewKeys_key);
    shift->op = keyz_text_to_lower;
    struct XViewKeys_key *alt = OArray_at(OList_at(page.rows, 3), 0, struct XViewKeys_key);
    alt->op = keyz_text_to_alt_1;
    return page;
}
static struct XViewKeys_page keyz_text_page_shift(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '\b', XK_ROW,
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\n', '\n',XK_ROW,
        XK_C, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', XK_U, '.', XK_ROW,
        XK_ALT, ' ', ' ', ' ', ' ', ' ', ' ', ' ', XK_L, XK_D, XK_R,
        XK_END,
    };
    struct XViewKeys_page page = XViewKeys_factory_page(parent, layout, style, ctrl_style);
    for (osize r=0; r<OList_num(page.rows); r++) {
        oobj row = OList_at(page.rows, r);
        for (osize c=0; c<OArray_num(row); c++) {
            struct XViewKeys_key *key = OArray_at(row, c, struct XViewKeys_key);
            key->op = keyz_text_to_lower;
        }
    }
    struct XViewKeys_key *shift = OArray_at(OList_at(page.rows, 2), 0, struct XViewKeys_key);
    shift->op = keyz_text_to_caps;
    struct XViewKeys_key *alt = OArray_at(OList_at(page.rows, 3), 0, struct XViewKeys_key);
    alt->op = keyz_text_to_alt_1;
    return page;
}
static struct XViewKeys_page keyz_text_page_lower(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\b', XK_ROW,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\n', '\n', XK_ROW,
        XK_S, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', XK_U, '.', XK_ROW,
        XK_ALT, ' ', ' ', ' ', ' ', ' ', ' ', ' ', XK_L, XK_D, XK_R,
        XK_END,
    };
    struct XViewKeys_page page = XViewKeys_factory_page(parent, layout, style, ctrl_style);
    struct XViewKeys_key *shift = OArray_at(OList_at(page.rows, 2), 0, struct XViewKeys_key);
    shift->op = keyz_text_to_shift;
    struct XViewKeys_key *alt = OArray_at(OList_at(page.rows, 3), 0, struct XViewKeys_key);
    alt->op = keyz_text_to_alt_1;
    return page;
}
static struct XViewKeys_page keyz_text_page_alt_1(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\b', XK_ROW,
        '!', '"', '#', '$', '%', '&', '\'', '(', ')', '\n', '\n', XK_ROW,
        XK_RO1, '\t', '*', '+', ',', '-', '.', '/', ',', XK_U, '.',XK_ROW,
        XK_TXT, XK_PAS, XK_PAS, ' ', ' ', ' ', ' ', ' ', XK_L, XK_D, XK_R,
        XK_END,
    };
    struct XViewKeys_page page = XViewKeys_factory_page(parent, layout, style, ctrl_style);
    struct XViewKeys_key *roman = OArray_at(OList_at(page.rows, 2), 0, struct XViewKeys_key);
    roman->op = keyz_text_to_alt_2;
    struct XViewKeys_key *txt = OArray_at(OList_at(page.rows, 3), 0, struct XViewKeys_key);
    txt->op = keyz_text_to_caps;
    return page;
}
static struct XViewKeys_page keyz_text_page_alt_2(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\b', XK_ROW,
        ':', ';', '<', '=', '>', '?', '@', '[', ']', '\n', '\n', XK_ROW,
        XK_RO2, '\\', '^', '_', '`', '{', '|', '}', ',', XK_U, '.',XK_ROW,
        XK_TXT, XK_PAS, XK_PAS, ' ', ' ', ' ', ' ', ' ', XK_L, XK_D, XK_R,
        XK_END,
    };
    struct XViewKeys_page page = XViewKeys_factory_page(parent, layout, style, ctrl_style);
    struct XViewKeys_key *roman = OArray_at(OList_at(page.rows, 2), 0, struct XViewKeys_key);
    roman->op = keyz_text_to_alt_1;
    struct XViewKeys_key *txt = OArray_at(OList_at(page.rows, 3), 0, struct XViewKeys_key);
    txt->op = keyz_text_to_caps;
    return page;
}


XViewKeys *XViewKeys_new_text(oobj parent, oobj style, oobj ctrl_style)
{
    struct XViewKeys_page page = keyz_text_page_caps(parent, style, ctrl_style);
    XViewKeys *self = XViewKeys_new(parent, page, true);
    oobj inject = OObj_new(self);
    OObj_name_set(inject, KEYZ_PAGE_INJECT);
    struct keyz_page_context *C = o_user_new0(inject, *C, 1);
    C->style = style;
    C->ctrl_style = ctrl_style;
    return self;
}

//
// new_terminal stuff
//



static struct XViewKeys_page keyz_terminal_page_upper(oobj parent, oobj style, oobj ctrl_style);
static struct XViewKeys_page keyz_terminal_page_lower(oobj parent, oobj style, oobj ctrl_style);
static struct XViewKeys_page keyz_terminal_page_alt(oobj parent, oobj style, oobj ctrl_style);
static void keyz_terminal_to_upper(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    struct oobj_opt inject = OObj_find(viewkeys, OObj, KEYZ_PAGE_INJECT, 0);
    if (!inject.o || !o_user(inject.o)) {
        return;
    }
    struct keyz_page_context *C = o_user(inject.o);

    XViewKeys_page_set_deferred(viewkeys, keyz_terminal_page_upper(viewkeys, C->style, C->ctrl_style));
}
static void keyz_terminal_to_lower(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    struct oobj_opt inject = OObj_find(viewkeys, OObj, KEYZ_PAGE_INJECT, 0);
    if (!inject.o || !o_user(inject.o)) {
        return;
    }
    struct keyz_page_context *C = o_user(inject.o);

    XViewKeys_page_set_deferred(viewkeys, keyz_terminal_page_lower(viewkeys, C->style, C->ctrl_style));
}
static void keyz_terminal_to_alt(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    struct oobj_opt inject = OObj_find(viewkeys, OObj, KEYZ_PAGE_INJECT, 0);
    if (!inject.o || !o_user(inject.o)) {
        return;
    }
    struct keyz_page_context *C = o_user(inject.o);

    XViewKeys_page_set_deferred(viewkeys, keyz_terminal_page_alt(viewkeys, C->style, C->ctrl_style));
}

static struct XViewKeys_page keyz_terminal_page_upper(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\b', XK_ROW,
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '\t', XK_ROW,
        'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', '\n', '\n',XK_ROW,
        XK_S_I, 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '_', XK_U, '.', XK_ROW,
        XK_ALT, XK_PAS, XK_PAS, ' ', ' ', ' ', ' ', ' ', XK_L, XK_D, XK_R,
        XK_END,
    };
    struct XViewKeys_page page = XViewKeys_factory_page(parent, layout, style, ctrl_style);
    struct XViewKeys_key *shift = OArray_at(OList_at(page.rows, 3), 0, struct XViewKeys_key);
    shift->op = keyz_terminal_to_lower;
    struct XViewKeys_key *alt = OArray_at(OList_at(page.rows, 4), 0, struct XViewKeys_key);
    alt->op = keyz_terminal_to_alt;
    return page;
}
static struct XViewKeys_page keyz_terminal_page_lower(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\b', XK_ROW,
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '\t', XK_ROW,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', '\n', '\n', XK_ROW,
        XK_C, 'z', 'x', 'c', 'v', 'b', 'n', 'm', '_', XK_U, '.', XK_ROW,
        XK_ALT, XK_PAS, XK_PAS, ' ', ' ', ' ', ' ', ' ', XK_L, XK_D, XK_R,
        XK_END,
    };
    struct XViewKeys_page page = XViewKeys_factory_page(parent, layout, style, ctrl_style);
    struct XViewKeys_key *shift = OArray_at(OList_at(page.rows, 3), 0, struct XViewKeys_key);
    shift->op = keyz_terminal_to_upper;
    struct XViewKeys_key *alt = OArray_at(OList_at(page.rows, 4), 0, struct XViewKeys_key);
    alt->op = keyz_terminal_to_alt;
    return page;
}
static struct XViewKeys_page keyz_terminal_page_alt(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '\b', XK_ROW,
        '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', XK_ROW,
        ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', XK_ROW,
        '[', '\\', ']', '^', '_', '`', '{', '|', '}', XK_U, '~', XK_ROW,
        XK_TXT, XK_PAS, XK_PAS, ' ', ' ', ' ', ' ', ' ', XK_L, XK_D, XK_R,
        XK_END,
    };
    struct XViewKeys_page page = XViewKeys_factory_page(parent, layout, style, ctrl_style);
    for (osize r=0; r<OList_num(page.rows); r++) {
        oobj row = OList_at(page.rows, r);
        for (osize c=0; c<OArray_num(row); c++) {
            struct XViewKeys_key *key = OArray_at(row, c, struct XViewKeys_key);
            key->op = keyz_terminal_to_upper;
        }
    }
    return page;
}

XViewKeys *XViewKeys_new_terminal(oobj parent, oobj style, oobj ctrl_style)
{
    struct XViewKeys_page page = keyz_terminal_page_upper(parent, style, ctrl_style);
    XViewKeys *self = XViewKeys_new(parent, page, true);
    oobj inject = OObj_new(self);
    OObj_name_set(inject, KEYZ_PAGE_INJECT);
    struct keyz_page_context *C = o_user_new0(inject, *C, 1);
    C->style = style;
    C->ctrl_style = ctrl_style;
    return self;
}

//
// virtual implementations
//

void XViewKeys__v_setup(oobj view)
{
    // noop
}

void XViewKeys__v_update(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewKeys);
    XViewKeys *self = view;

    if (self->deferred_page.rows) {
        XViewKeys_page_set(self, self->deferred_page);
        self->deferred_page.rows = NULL;
    }

    // swipe for cursor movement
    if (self->swipe_distance.x > 0 || self->swipe_distance.y > 0) {
        struct a_pointer p = a_pointer(0, 0);
        if(a_pointer_pressed(p)) {
            self->swipe_start = p.pos.xy;
        } else if(p.down) {
            vec2 diff = vec2_sub_v(p.pos.xy, self->swipe_start);
            bool reset = true;
            if(self->swipe_distance.x > 0 && diff.x >= +self->swipe_distance.x) {
                OStream_print(self->stream, A_INPUT_ESCAPED_RIGHT);
            } else if(self->swipe_distance.x > 0 && diff.x <= -self->swipe_distance.x) {
                OStream_print(self->stream, A_INPUT_ESCAPED_LEFT);
            } else if(self->swipe_distance.y > 0 && diff.y >= +self->swipe_distance.y) {
                OStream_print(self->stream, A_INPUT_ESCAPED_DOWN);
            } else if(self->swipe_distance.y > 0 && diff.y <= -self->swipe_distance.y) {
                OStream_print(self->stream, A_INPUT_ESCAPED_UP);
            } else {
                reset = false;
            }

            if(reset) {
                o_log_debug_s("XViewKeys", "swiped");
                a_rumble(0.3, 0.05);
                self->swipe_start = p.pos.xy;
            }
        }
    }
     
    vec2 gui_size = WTheme_update_full_tex(self->theme, self->gui, tex);
    
    
    // even if full_tex is used, grid may be a bit smaller, we center that by cam (only x)
    oobj cam = AView_cam(view);
    vec2 cam_size = RCam_size(cam);
    float offset_x = cam_size.x - gui_size.x;
    offset_x = m_floor(offset_x/2);
    RCam_pos_set(cam, vec2_(-offset_x, 0), true);
    

    if (self->keyboard_enabled) {
        const char *input = a_input_key_string();
        char *filtered = NULL;
        if (self->opt_keyboard_filter) {
            filtered = o_alloc0(self, 1, o_strlen(input) + 1);
            self->opt_keyboard_filter(self, filtered, input);
            input = filtered;
        }
        OStream_print(self->stream, input);
        o_free(self, filtered);
    }

}

void XViewKeys__v_render(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewKeys);
    XViewKeys *self = view;

    RTex_clear(tex, R_GRAY_X(0.2));

    WTheme_render(self->theme, tex);

}

//
// object functions:
//

struct XViewKeys_page XViewKeys_page_set(oobj obj, struct XViewKeys_page page)
{
    OObj_assert(obj, XViewKeys);
    XViewKeys *self = obj;
    o_del(self->page.rows);
    self->page = page;

    o_del(self->gui);
    
    ivec2 page_size = keyz_page_size(page.rows);
    
    self->gui = WGrid_new(self, m_2(page_size), vec2_(-1));
    
    if(page.style) {
        WObj_style_set(self->gui, page.style, true);
    }
    for (osize row_i=0; row_i<OList_num(page.rows); row_i++) {
        oobj array = OList_at(page.rows, row_i);
        keyz_add_row(self, row_i, row_i, array);
    }
    
    // calc max cell_size
    vec2 cell_size = vec2_(0);
    WObj **list = WObj_list(self->gui, NULL);
    for(WObj **it=list; *it; it++) {
        vec2 size = WObj_update(*it, vec2_(0), vec2_(0), true, self->theme, a_pointer);
        ivec2 span = WGrid_child_cell(*it).zw;
        size.x/=span.x;
        size.y/=span.y;
        cell_size = vec2_max_v(cell_size, size);
    }
    o_free(self->gui, list);
    
    vec2 size = vec2_scale_v(cell_size, vec2_(m_2(page_size)));
    
    o_log_debug_s(__func__, "cell size: %f %f; size: %f %f", m_2(cell_size), m_2(size));
    
    // use the minimal units needed for the camera
    AView_cam_min_units_set(self, size);
    
    // setup grid to fit a available size
    WGrid_align_set(self->gui, WGrid_align_FIT, WGrid_align_FIT);
    vecn_set(WGrid_col_weights(self->gui), 1, page_size.x);
    vecn_set(WGrid_row_weights(self->gui), 1, page_size.y);

    return page;
}

void XViewKeys_page_set_deferred(oobj obj, struct XViewKeys_page page)
{
    OObj_assert(obj, XViewKeys);
    XViewKeys *self = obj;
    o_del(self->deferred_page.rows);
    self->deferred_page = page;
    
}




//
// factory private
//

static ivec2 keyz_page_layout_size(const int *layout)
{
    ivec2 size = {{0, 0}};
    int current_cols = 0;
    for(const int *it=layout;*it;it++) {
        int c = *it;
        if(c==XViewKeys_F_NEXT_ROW) {
            current_cols = 0;
            size.y++;
            continue;
        }
        current_cols++;
        size.x = o_max(size.x, current_cols);
    }
    return size;
}

static oobj keyz_page_row_new(oobj page)
{
    oobj row = OArray_new_dyn(page, NULL, sizeof(struct XViewKeys_key), 0, 16);
    OList_push(page, row);
    return row;
}

static struct XViewKeys_key *keyz_page_add_ascii(oobj row, int c, int col_offset)
{
    struct XViewKeys_key key = {0};
    key.print = key.label = o_strf(row, "%c", c);
    key.offset_cols = col_offset;
    return OArray_push(row, &key);
}

static void keyz_page_paste_fn(oobj viewkeys, oobj stream, int row, int col, const struct XViewKeys_key *key)
{
    char *paste = o_utils_clipboard_get(viewkeys);
    if (!paste) {
        return;
    }
    OStream_print(stream, paste);
    o_free(viewkeys, paste);
}


//
// factory public
//

struct XViewKeys_page XViewKeys_factory_page(oobj parent, const int *layout, oobj style, oobj ctrl_style)
{
    ivec2 layout_size = keyz_page_layout_size(layout);

    // needs to be o_del'd
    oobj container = OObj_new(parent);
    int *layout_mat = o_new0(container, int, layout_size.x * layout_size.y);
    struct XViewKeys_key **key_mat = o_new0(container, struct XViewKeys_key *, layout_size.y * layout_size.x);

    oobj page = OList_new(parent, NULL, 0);
    if (!style) {
        style = XViewKeys_factory_style(page);
    }
    if (!ctrl_style) {
        ctrl_style = XViewKeys_factory_style_ctrl(page);
    }
    oobj row = keyz_page_row_new(page);
    ivec2 pos = {-1, 0};
    int col_offset = 0;
    for(const int *it=layout;*it;it++) {
        int c = *it;
        pos.x++;

        if(c == XViewKeys_F_PASTE && !o_utils_clipboard_system_available()) {
            // paste is not available on this system
            // replacing with space
            c = ' ';
        }

        layout_mat[pos.y*layout_size.x+pos.x] = c;
        struct XViewKeys_key **key_mat_pos = &key_mat[pos.y*layout_size.x+pos.x];

        if(c == XViewKeys_F_NEXT_ROW) {
            row = keyz_page_row_new(page);
            pos.x=-1;
            pos.y++;
            col_offset = 0;
            continue;
        }

        if(c == XViewKeys_F_EMPTY) {
            col_offset++;
            continue;
        }


        // enlarge
        if(pos.y>=1 && (layout_mat[(pos.y-1)*layout_size.x+pos.x] == c)) {
            assert(OArray_num(row)>=1);
            struct XViewKeys_key *key = key_mat[(pos.y-1)*layout_size.x+pos.x];
            *key_mat_pos = key;
            key->rows = o_max(key->rows, 1) + 1;
            col_offset++;
            continue;
        }
        if(pos.x>=1 && (layout_mat[pos.y*layout_size.x+(pos.x-1)] == c)) {
            assert(OArray_num(row)>=1);
            struct XViewKeys_key *key = key_mat[pos.y*layout_size.x+(pos.x-1)];
            *key_mat_pos = key;
            key->cols = o_max(key->cols, 1) + 1;
            continue;
        }

        if(c>='!' && c<='~') {
            // printable ascii chars
            *key_mat_pos = keyz_page_add_ascii(row, c, col_offset);
            col_offset = 0;
            continue;
        }
        if(c==' ') {
            struct XViewKeys_key key = {0};
            key.print = " ";
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if(c=='\n') {
            struct XViewKeys_key key = {0};
            key.print = "\n";
            key.icon = WTheme_ICON_ENTER;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if(c=='\t') {
            struct XViewKeys_key key = {0};
            key.print = "\t";
            key.icon = WTheme_ICON_TAB;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if(c=='\b') {
            struct XViewKeys_key key = {0};
            key.print = "\b";
            key.icon = WTheme_ICON_ARROW_LEFT;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }

        if (c==XViewKeys_F_PASTE) {
            struct XViewKeys_key key = {0};
            key.label = "PASTE";
            key.op = keyz_page_paste_fn;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_UP) {
            struct XViewKeys_key key = {0};
            key.print = A_INPUT_ESCAPED_UP;
            key.icon = WTheme_ICON_DIR_UP;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_DOWN) {
            struct XViewKeys_key key = {0};
            key.print = A_INPUT_ESCAPED_DOWN;
            key.icon = WTheme_ICON_DIR_DOWN;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_LEFT) {
            struct XViewKeys_key key = {0};
            key.print = A_INPUT_ESCAPED_LEFT;
            key.icon = WTheme_ICON_DIR_LEFT;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_RIGHT) {
            struct XViewKeys_key key = {0};
            key.print = A_INPUT_ESCAPED_RIGHT;
            key.icon = WTheme_ICON_DIR_RIGHT;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_SHIFT) {
            struct XViewKeys_key key = {0};
            key.icon = WTheme_ICON_SHIFT;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_CAPS) {
            struct XViewKeys_key key = {0};
            key.icon = WTheme_ICON_CAPS;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_SHIFT_INV) {
            struct XViewKeys_key key = {0};
            key.icon = WTheme_ICON_SHIFT_INV;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_ALT) {
            struct XViewKeys_key key = {0};
            key.label = "ALT";
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_TXT) {
            struct XViewKeys_key key = {0};
            key.label = "TXT";
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_ROMAN_1) {
            struct XViewKeys_key key = {0};
            key.icon = WTheme_ICON_ROMAN_1;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
        if (c==XViewKeys_F_ROMAN_2) {
            struct XViewKeys_key key = {0};
            key.icon = WTheme_ICON_ROMAN_2;
            key.style = ctrl_style;
            key.offset_cols = col_offset;
            col_offset = 0;
            *key_mat_pos = OArray_push(row, &key);
            continue;
        }
    }

    o_del(container);
    return (struct XViewKeys_page) {page, style, vec2_(16)};
}

struct XViewKeys_page XViewKeys_factory_page_number(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        '7', '8', '9', '-', '*', XK_ROW,
        '4', '5', '6', '+', '/', XK_ROW,
        '1', '2', '3', XK_U, '\b', XK_ROW,
        '0', '.', XK_L, XK_D,
        XK_R, XK_END,
    };
    return XViewKeys_factory_page(parent, layout, style, ctrl_style);
}

void XViewKeys_factory_filter_number(oobj viewkeyz, char *out_buffer, const char *in_buffer)
{
    char *out_it = out_buffer;
    for (;*in_buffer; in_buffer++){
        // check for escaped cursor input
        if (o_str_begins(in_buffer, "\x1B[")) {
            if (o_str_begins(in_buffer, A_INPUT_ESCAPED_LEFT)) {
                out_it += o_str_copy(out_it, A_INPUT_ESCAPED_LEFT)-1;
            } else if (o_str_begins(in_buffer, A_INPUT_ESCAPED_RIGHT)) {
                out_it += o_str_copy(out_it, A_INPUT_ESCAPED_RIGHT)-1;
            } else if (o_str_begins(in_buffer, A_INPUT_ESCAPED_UP)) {
                out_it += o_str_copy(out_it, A_INPUT_ESCAPED_UP)-1;
            } else if (o_str_begins(in_buffer, A_INPUT_ESCAPED_DOWN)) {
                out_it += o_str_copy(out_it, A_INPUT_ESCAPED_DOWN)-1;
            }
            continue;
        }

        // filter numnber input
        if(o_str_find_char("0123456789.+-*/\b", *in_buffer) < 0) {
            continue;
        }
        *out_it++ = *in_buffer;
    }
}

struct XViewKeys_page XViewKeys_factory_page_hex(oobj parent, oobj style, oobj ctrl_style)
{
    int layout[] = {
        '0', '1', '2', '3', XK_U, '\b', XK_ROW,
        '4', '5', '6', '7', XK_D, '#', XK_ROW,
        '8', '9', 'A', 'B', XK_ROW,
        'C', 'D', 'E', 'F', XK_L, XK_R, XK_END
    };
    return XViewKeys_factory_page(parent, layout, style, ctrl_style);
}
void XViewKeys_factory_filter_hex(oobj viewkeys, char *out_buffer, const char *in_buffer)
{
    char *out_it = out_buffer;
    for (;*in_buffer; in_buffer++){
        // check for escaped cursor input
        if (o_str_begins(in_buffer, "\x1B[")) {
            if (o_str_begins(in_buffer, A_INPUT_ESCAPED_LEFT)) {
                out_it += o_str_copy(out_it, A_INPUT_ESCAPED_LEFT)-1;
            } else if (o_str_begins(in_buffer, A_INPUT_ESCAPED_RIGHT)) {
                out_it += o_str_copy(out_it, A_INPUT_ESCAPED_RIGHT)-1;
            } else if (o_str_begins(in_buffer, A_INPUT_ESCAPED_UP)) {
                out_it += o_str_copy(out_it, A_INPUT_ESCAPED_UP)-1;
            } else if (o_str_begins(in_buffer, A_INPUT_ESCAPED_DOWN)) {
                out_it += o_str_copy(out_it, A_INPUT_ESCAPED_DOWN)-1;
            }
            continue;
        }

        // filter numnber input
        if(o_str_find_char("0123456789ABCDEF/\b", *in_buffer) < 0) {
            continue;
        }
        *out_it++ = *in_buffer;
    }
}

oobj XViewKeys_factory_style(oobj parent)
{
    oobj style = WStyle_new(parent);
    WStyle_btn_style_set(style, WBtn_FLAT_ROUND);
    return style;
}

oobj XViewKeys_factory_style_ctrl(oobj parent)
{
    oobj style = WStyle_new(parent);
    WStyle_btn_style_set(style, WBtn_FLAT_ROUND);
    WStyle_text_color_set(style,  vec4_(0.7, 0.8, 1.0, 1.0));
    WStyle_icon_color_set(style,  vec4_(0.7, 0.8, 1.0, 1.0));
    return style;
}