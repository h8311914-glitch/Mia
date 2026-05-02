#include "r/textlayout.h"
#include "o/OArray.h"
#include "o/OObjRoot.h"
#include "o/str_range.h"
#include "o/img.h"
#include "m/vec/vec2.h"
#include "r/box.h"
#include "r/quad.h"
#include "r/RTex.h"
#include <ctype.h>

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"


o_inline void pose_x_set(mat4 *p, float x)
{
    p->m30 = x;
}

o_inline void pose_y_set(mat4 *p, float y)
{
    p->m31 = y;
}

o_inline void pose_xy_set(mat4 *p, float x, float y)
{
    pose_x_set(p, x);
    pose_y_set(p, y);
}


static void generic_boxes(struct r_textlayout *self, osize idx, vec2 xy, vec2 uv)
{
    struct r_box b = r_box_new(m_2(self->size));
    b.rect.xy = xy;
    b.uv_rect.xy = uv;
    b.fx = self->internal_generic_color;
    OArray_push(self->internal_generic_array, &b);
}

static void generic_quads(struct r_textlayout *self, osize idx, vec2 xy, vec2 uv)
{
    struct r_quad q = r_quad_new(m_2(self->size));
    pose_xy_set(&q.pose, m_2(xy));
    pose_xy_set(&q.uv_pose, m_2(uv));
    q.fx = self->internal_generic_color;
    OArray_push(self->internal_generic_array, &q);
}

static void generic_noop(struct r_textlayout *self, osize idx, vec2 xy, vec2 uv)
{
    // noop
}

// returns 0 words if textlayout does not contain any words.
// uses only space ' ' as a word break, and ignores multiple of it such as "hello  world" (treated as one)
// if the first word len exceeds max_cols, its trimmed to max_cols and returned as 1 word
static struct o_str_range_list split_words(oobj parent, const char *text, int max_cols)
{
    int glyphs = 0;
    int cols = 0;

    osize words_cap = 32;
    struct o_str_range_list words = {0};
    words.parent = parent;
    words.list = o_new(parent, struct o_str_range, words_cap);

    osize word_start = -1;

    const char *it = text;
    while (cols < max_cols && *it!='\n' && *it!='\0') {
        if (*it != ' ') {
            if (word_start<0) {
                word_start = it-text;
            }
            glyphs++;
            it++;
            cols = glyphs + words.len;
            continue;
        }

        // *it == ' '

        if (word_start>=0) {
            words.len++;
            if (words.len > words_cap) {
                words_cap = words.len*2;
                words.list = o_renew(words.parent, words.list, struct o_str_range, words_cap);
            }
            words.list[words.len-1].start = word_start;
            words.list[words.len-1].len = it-text-word_start;
            assert(words.list[words.len-1].len >0);
            cols = glyphs + words.len-1;
            word_start = -1;
        }
        it++;
    }

    if(word_start>=0 && (*it=='\n' || *it=='\0' || words.len==0)) {
        osize len = it-text-word_start;
        len = o_min(len, max_cols);
        if(len>0) {
            words.len++;
            if (words.len > words_cap) {
                words_cap = words.len;
                words.list = o_renew(words.parent, words.list, struct o_str_range, words_cap);
            }
            words.list[words.len-1].start =  word_start;
            words.list[words.len-1].len = len;
        }
    }

    return words;
}


static void push_char(struct r_textlayout *self, r_textlayout__generic_fn generic_callback_fn, char c, int col, int row, int i)
{
    if (self->casing == r_textlayout_casing_UPPER) {
        c = toupper(c);
    } else if (self->casing == r_textlayout_casing_LOWER) {
        c = tolower(c);
    }

    vec2 xy = vec2_(col * self->offset.x, row * self->offset.y);
    vec2 uv = self->v_char_uv(self, c);

    generic_callback_fn(self, i, xy, uv);
}

static void push_space_hidden(struct r_textlayout *self, r_textlayout__generic_fn generic_callback_fn, int i)
{
    char c = ' ';
    vec2 xy = vec2_(m_MAX);
    vec2 uv = self->v_char_uv(self, c);
    generic_callback_fn(self, i, xy, uv);
}

static void push_word(struct r_textlayout *self, r_textlayout__generic_fn generic_callback_fn,
    const char *text, osize line_i, struct o_str_range word, int *inout_col, int row, osize *inout_i)
{
    osize col = *inout_col;
    osize i = *inout_i;
    osize start_i = word.start + line_i;
    for (;i < start_i; i++) {
        push_space_hidden(self, generic_callback_fn, i);
    }
    osize end_i = start_i + word.len;
    for(;i < end_i; i++) {
        push_char(self, generic_callback_fn, text[i], col, row, i);
        col++;
        if (text[i] == '\n') {
            break;
        }
    }
    *inout_col = col;
    *inout_i = i;
}

static int eat_lines(struct r_textlayout *self, r_textlayout__generic_fn generic_callback_fn,
                     const char *text, osize *inout_i)
{
    int lines = 0;
    osize i = *inout_i;
    while (text[i] == ' ' || text[i] == '\n') {
        push_space_hidden(self, generic_callback_fn, i);
        if (text[i] == '\n') {
            lines++;
        }
        i++;
    }
    // a single line is always consumed.
    // I must admit this feels a bit off :D
    // so Either we have not a single newline, but word cols broke, so we need a single line
    // or we have 1+ newlines at the end.
    // if we have the newlines, we dont need that additional one that we always have. so not simply +1
    lines = o_max(lines, 1);
    *inout_i = i;
    return lines;
}


static ivec2 textlayout_set_wrap_off(struct r_textlayout *self, r_textlayout__generic_fn generic_callback_fn, const char *text)
{
    osize text_len = o_strlen(text);
    int col = 0;
    int row = 0;
    int cols = 0;
    for (osize i = 0; i < text_len; i++) {
        char c = text[i];
        push_char(self, generic_callback_fn, c, col, row, i);
        col++;
        if (c == '\n') {
            col = 0;
            row++;
        }
        cols = o_max(cols, col);
    }
    return ivec2_(cols, row+1);
}


static ivec2 textlayout_set_wrap_empty(struct r_textlayout *self, r_textlayout__generic_fn generic_callback_fn, const char *text)
{
    osize text_len = o_strlen(text);
    for (osize i = 0; i < text_len; i++) {
        push_space_hidden(self, generic_callback_fn, i);
    }
    return ivec2_(0);
}


static ivec2 textlayout_set_wrap_any(struct r_textlayout *self, r_textlayout__generic_fn generic_callback_fn, const char *text, ivec2 max_colsrows)
{
    osize text_len = o_strlen(text);
    osize i=0;
    int col = 0;
    int row = 0;
    int cols = 0;
    for (; i<text_len; i++) {
        char c = text[i];
        push_char(self, generic_callback_fn, c, col, row, i);

        col++;
        if (text[i] == '\n' || col>=max_colsrows.x) {
            col = 0;
            row++;
            if(row >= max_colsrows.y) {
                row = max_colsrows.y-1;
                break;
            }
        }
        cols = o_max(cols, col);
    }
    // remaining will be hidden spaces
    for (; i<text_len; i++) {
        push_space_hidden(self, generic_callback_fn, i);
    }
    return ivec2_(cols, row+1);
}

static ivec2 textlayout_set_wrap_word(struct r_textlayout *self, r_textlayout__generic_fn generic_callback_fn, const char *text, ivec2 max_colsrows)
{
    // new unregistered root
    oobj container = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    osize text_len = o_strlen(text);
    osize i=0;
    int col = 0;
    int row = 0;
    int cols = 0;
    while (i<text_len) {
        osize line_i = i;
        const char *line = text+i;
        struct o_str_range_list words = split_words(container, line, max_colsrows.x);
        if(words.len == 0) {
            goto PUSH_DONE;
        }
        if(self->align == r_textlayout_align_LEFT) {
            for(osize w=0; w<words.len; w++) {
                push_word(self, generic_callback_fn, text, line_i, words.list[w], &col, row, &i);
                cols = o_max(cols, col);
                
                // space between words
                col++;
            }
            col = 0;
            row += eat_lines(self, generic_callback_fn, text, &i);
            if(row>=max_colsrows.y) {
                row = max_colsrows.y-1;
                goto PUSH_DONE;
            }
            continue;
        }

        // others need to calc room left to align
        osize needed = words.len-1;
        for (osize w=0; w<words.len; w++) {
            needed += words.list[w].len;
        }
        osize room = max_colsrows.x - needed;

        if(self->align == r_textlayout_align_RIGHT) {
            col = room;
            for(osize w=0; w<words.len; w++) {
                push_word(self, generic_callback_fn, text, line_i, words.list[w], &col, row, &i);
                cols = o_max(cols, col);
                
                // space between words
                col++;
            }
            col = 0;
            row += eat_lines(self, generic_callback_fn, text, &i);
            if(row>=max_colsrows.y) {
                row = max_colsrows.y-1;
                goto PUSH_DONE;
            }
            continue;
        }

        if(self->align == r_textlayout_align_CENTER) {
            col = room/2;
            for(osize w=0; w<words.len; w++) {
                push_word(self, generic_callback_fn, text, line_i, words.list[w], &col, row, &i);
                cols = o_max(cols, col);
                
                // space between words
                col++;
            }
            col = 0;
            row += eat_lines(self, generic_callback_fn, text, &i);
            if(row>=max_colsrows.y) {
                row = max_colsrows.y-1;
                goto PUSH_DONE;
            }
            continue;
        }

        if(self->align == r_textlayout_align_BLOCK) {
            osize seps = words.len-1;
            osize sep_room = 0;
            osize sep_mod = 0;
            if (seps >= 1) {
                sep_room = room/seps;
                sep_mod = room%seps;
            }
            for(osize w=0; w<words.len; w++) {
                push_word(self, generic_callback_fn, text, line_i, words.list[w], &col, row, &i);
                cols = o_max(cols, col);
                
                // space between words
                col += 1 + sep_room;
                if (w < sep_mod) {
                    col++; // remaining spaces from modulo
                }
            }
            col = 0;
            row += eat_lines(self, generic_callback_fn, text, &i);
            if(row>=max_colsrows.y) {
                row = max_colsrows.y-1;
                goto PUSH_DONE;
            }
            continue;
        }
    }

    PUSH_DONE:
    // remaining will be hidden spaces
    for (; i<text_len; i++) {
        push_space_hidden(self, generic_callback_fn, i);
    }
    o_del(container);
    return ivec2_(cols, row);
}


//
// public
//

struct r_textlayout r_textlayout_new(vec2 size, vec2 offset, r_textlayout__char_uv_fn char_uv)
{
    struct r_textlayout self = {0};
    self.size = size;
    self.offset = offset;
    self.casing = r_textlayout_casing_UNCHANGED;
    self.v_char_uv = char_uv;
    return self;
}

struct r_textlayout r_textlayout_new_font35(void)
{
    struct r_textlayout self = r_textlayout_new(vec2_(3, 5), vec2_(4, 6), r_textlayout__v_font35_char_uv);
    self.casing = r_textlayout_casing_UPPER;
    return self;
}

struct r_textlayout r_textlayout_new_font35_shadow(void)
{
    struct r_textlayout self = r_textlayout_new(vec2_(4, 6), vec2_(4, 6), r_textlayout__v_font35_char_uv);
    self.casing = r_textlayout_casing_UPPER;
    return self;
}

struct r_textlayout r_textlayout_new_font55(void)
{
    struct r_textlayout self = r_textlayout_new(vec2_(5, 5), vec2_(6, 6), r_textlayout__v_font55_char_uv);
    self.casing = r_textlayout_casing_UPPER;
    return self;
}

struct r_textlayout r_textlayout_new_font55_shadow(void)
{
    struct r_textlayout self = r_textlayout_new(vec2_(6, 6), vec2_(6, 6), r_textlayout__v_font55_char_uv);
    self.casing = r_textlayout_casing_UPPER;
    return self;
}

struct r_textlayout r_textlayout_new_font58(void)
{
    struct r_textlayout self = r_textlayout_new(vec2_(5, 8), vec2_(6, 9), r_textlayout__v_font58_char_uv);
    self.casing = r_textlayout_casing_UNCHANGED;
    return self;
}



struct r_textlayout r_textlayout_new_font58_shadow(void)
{
    struct r_textlayout self = r_textlayout_new(vec2_(6, 9), vec2_(6, 9), r_textlayout__v_font58_char_uv);
    self.casing = r_textlayout_casing_UNCHANGED;
    return self;
}


oobj r_textlayout_font35_shadow_tex(oobj parent, bvec4 *opt_shadow_color)
{
    struct o_img img = o_img_new_file(parent, R_TEXT_FONT35_SHADOW_FILE, o_img_RGBA);
    if (opt_shadow_color) {
        for (int i = 0; i < img.cols * img.rows; i++) {
            bvec4 *rgba = (bvec4 *) o_img_at_idx(img, i);
            if (rgba->a == 128) {
                *rgba = *opt_shadow_color;
            }
        }
    }
    oobj tex = RTex_new(parent, img.data, img.cols, img.rows);
    o_img_free(&img);
    return tex;
}

oobj r_textlayout_font55_shadow_tex(oobj parent, bvec4 *opt_shadow_color)
{
    struct o_img img = o_img_new_file(parent, R_TEXT_FONT55_SHADOW_FILE, o_img_RGBA);
    if (opt_shadow_color) {
        for (int i = 0; i < img.cols * img.rows; i++) {
            bvec4 *rgba = (bvec4 *) o_img_at_idx(img, i);
            if (rgba->a == 128) {
                *rgba = *opt_shadow_color;
            }
        }
    }
    oobj tex = RTex_new(parent, img.data, img.cols, img.rows);
    o_img_free(&img);
    return tex;
}

oobj r_textlayout_font58_shadow_tex(oobj parent, bvec4 *opt_shadow_color)
{
    struct o_img img = o_img_new_file(parent, R_TEXT_FONT58_SHADOW_FILE, o_img_RGBA);
    if (opt_shadow_color) {
        for (int i = 0; i < img.cols * img.rows; i++) {
            bvec4 *rgba = (bvec4 *) o_img_at_idx(img, i);
            if (rgba->a == 128) {
                *rgba = *opt_shadow_color;
            }
        }
    }
    oobj tex = RTex_new(parent, img.data, img.cols, img.rows);
    o_img_free(&img);
    return tex;
}


vec2 r_textlayout_set_generic(struct r_textlayout *self, const char *text, r_textlayout__generic_fn generic_callback_fn)
{
    bool wrap = self->max_size.x > 0 && self->wrap != r_textlayout_wrap_OFF;

    ivec2 colsrows;

    if (!wrap) {
        colsrows = textlayout_set_wrap_off(self, generic_callback_fn, text);
        goto SET_DONE;
    }
    // wrap mode

    // maximal cols and optional rows
    vec2 max_size = vec2_sub_v(self->max_size, self->size);
    if (max_size.x < 0 || (self->max_size.y > 0 && max_size.y < 0)) {
        colsrows = textlayout_set_wrap_empty(self, generic_callback_fn, text);
        goto SET_DONE;
    }
    oi32 max_cols = 1 + m_floor(max_size.x / self->offset.x);
    oi32 max_rows = oi32_MAX;
    if (self->max_size.y > 0) {
        max_rows = 1 + m_floor(max_size.y / self->offset.y);
    }
    ivec2 max_colsrows = ivec2_(max_cols, max_rows);
    assert(max_colsrows.x > 0 && max_colsrows.y > 0);

    if (self->wrap == r_textlayout_wrap_ANY) {
        colsrows = textlayout_set_wrap_any(self, generic_callback_fn, text, max_colsrows);
        goto SET_DONE;
    }

    // wrap mode == r_textlayout_wrap_WORD (or smth stupid which is just ignored...)
    colsrows = textlayout_set_wrap_word(self, generic_callback_fn, text, max_colsrows);


    SET_DONE:
    
    if (colsrows.x == 0) {
        self->text_size = vec2_(0);
    } else {
        self->text_size = vec2_(
            (colsrows.x - 1) * self->offset.x + self->size.x,
            (colsrows.y - 1) * self->offset.y + self->size.y
        );
    }
    self->cols = colsrows.x;
    self->rows = colsrows.y;
    
    return self->text_size;
}

vec2 r_textlayout_set_boxes(struct r_textlayout *self, oobj boxes_array, const char *text, vec4 textlayout_color)
{
    self->internal_generic_array = boxes_array;
    self->internal_generic_color = textlayout_color;
    return r_textlayout_set_generic(self, text, generic_boxes);
}

vec2 r_textlayout_set_quads(struct r_textlayout *self, oobj quads_array, const char *text, vec4 textlayout_color)
{
    self->internal_generic_array = quads_array;
    self->internal_generic_color = textlayout_color;
    return r_textlayout_set_generic(self, text, generic_quads);
}

vec2 r_textlayout_size(struct r_textlayout *self, const char *text)
{
    return r_textlayout_set_generic(self, text, generic_noop);
}


//
// virtual implementations:
//

vec2 r_textlayout__v_font35_char_uv(struct r_textlayout *self, char glyph)
{
    ou8 c = *((ou8*)&glyph);
    const int columns = 16;
    const int rows = 6;
    const vec2 base_offset = {{8, 8}};
    const vec2 stride = {{16, 16}};

    if (c < ' ') {
        c = ' ';
    } else if(c > '~') {
        c = '~'+1;
    }
    c -= ' ';
    int col = c % columns;
    int row = c / columns;
    return vec2_add_scaled_v(base_offset,
                             vec2_(col, row), stride);
}

vec2 r_textlayout__v_font55_char_uv(struct r_textlayout *self, char glyph)
{
    ou8 c = *((ou8*)&glyph);
    const int columns = 16;
    const int rows = 6;
    const vec2 base_offset = {{8, 8}};
    const vec2 stride = {{16, 16}};

    if (c < ' ') {
        c = ' ';
    } else if(c > '~') {
        c = '~'+1;
    }
    c -= ' ';
    int col = c % columns;
    int row = c / columns;
    return vec2_add_scaled_v(base_offset,
                             vec2_(col, row), stride);
}

vec2 r_textlayout__v_font58_char_uv(struct r_textlayout *self, char glyph)
{
    ou8 c = *((ou8*)&glyph);
    const int columns = 16;
    const int rows = 6;
    const vec2 base_offset = {{4, 4}};
    const vec2 stride = {{16, 16}};

    if (c < ' ') {
        c = ' ';
    } else if(c > '~') {
        c = '~'+1;
    }
    c -= ' ';
    int col = c % columns;
    int row = c / columns;
    return vec2_add_scaled_v(base_offset,
                             vec2_(col, row), stride);
}
