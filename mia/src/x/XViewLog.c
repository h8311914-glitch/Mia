#include "x/XViewLog.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "r/RObjText.h"
#include "r/RCam.h"
#include "r/RTex.h"
#include "a/app.h"
#include "w/WTheme.h"
#include "w/WAlign.h"
#include "w/WBox.h"
#include "w/WTextShadow.h"
#include "w/WBtn.h"


#undef O_LOG_LIB
#define O_LOG_LIB "x"

#include "o/array.h"
#include "o/log.h"


static char log_level_names[] = {
    'T', 'D', 'I', 'W', 'E', 'F'
};
static vec4 log_level_colors[] = {
    {{0, 0, 1, 1}},
    {{0, 1, 1, 1}},
    {{0, 1, 0, 1}},
     {{1, 1, 0, 1}},
    {{1, 0, 0, 1}},
{{1, 0, 1, 1}}
};


static float text_zoom_lvls[] = {
    0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1.0, 1.1, 1.2, 1.3, 1.4
};

//
// public
//

XViewLog *XViewLog_init(oobj obj, oobj parent)
{

    AView *super = obj;
    XViewLog *self = obj;
    o_clear(self, sizeof *self, 1);

    AView_init(obj, parent, XViewLog__v_setup, XViewLog__v_update, XViewLog__v_render);
    OObj_id_set(self, XViewLog_ID);


    return self;
    
}


//
// virtual implementations
//

void XViewLog__v_setup(oobj view)
{
    OObj_assert(view, XViewLog);
    XViewLog *self = view;


    self->bg_color = R_TRANSPARENT;
    self->text_alpha = 0.8;

    self->text = RObjText_new_font35_shadow(self, NULL, NULL);
    //RObjText_layout_ref(self->text)->casing = r_textlayout_casing_UNCHANGED;
    
    self->zoom = 4;

    self->theme = WTheme_new_tiny(view);
    self->gui = WObj_new(view);
    
    oobj align = WAlign_new(self->gui);
    WAlign_align_set(align, WAlign_END, WAlign_END);
    oobj box = WBox_new_v(align);
    WObj_padding_set(box, vec4_(4));
    WBox_spacing_set(box, vec2_(4));
    
    oobj btn, lbl;
    btn = WBtn_new(box);
    WBtn_style_set(btn, WBtn_FLAT_ROUND);
    WBtn_color_set(btn, vec4_(1, 1, 1, 0.5));
    lbl = WText_new(btn, "+");
    WObj_padding_set(lbl, vec4_(2, 0, 2, 0));
    WText_color_set(lbl, vec4_(1, 0, 0, 0.33));
    self->btn_zoom_plus = btn;

    btn = WBtn_new(box);
    WBtn_style_set(btn, WBtn_FLAT_ROUND);
    WBtn_color_set(btn, vec4_(1, 1, 1, 0.5));
    lbl = WText_new(btn, "-");
    WObj_padding_set(lbl, vec4_(2, 0, 2, 0));
    WText_color_set(lbl, vec4_(0, 1, 0, 0.33));
    self->btn_zoom_minus = btn;

    btn = WBtn_new(box);
    WBtn_style_set(btn, WBtn_FLAT_ROUND);
    WBtn_color_set(btn, vec4_(1, 1, 1, 0.5));
    lbl = WText_new(btn, "<");
    WObj_padding_set(lbl, vec4_(2, 0, 2, 0));
    WText_color_set(lbl, vec4_(0, 1, 0, 0.33));
    self->btn_level_prev = btn;

    self->level_label = WTextShadow_new(box, "");

    btn = WBtn_new(box);
    WBtn_style_set(btn, WBtn_FLAT_ROUND);
    WBtn_color_set(btn, vec4_(1, 1, 1, 0.5));
    lbl = WText_new(btn, ">");
    WObj_padding_set(lbl, vec4_(2, 0, 2, 0));
    WText_color_set(lbl, vec4_(0, 1, 0, 0.33));
    self->btn_level_next = btn;

#ifndef NDEBUG
    btn = WBtn_new(box);
    WBtn_style_set(btn, WBtn_DEF_ROUND);
    WBtn_color_set(btn, vec4_(1, 1, 1, 0.5));
    lbl = WText_new(btn, "M");
    WObj_padding_set(lbl, vec4_(2, 0, 2, 0));
    WText_color_set(lbl, vec4_(1, 0, 1, 0.33));
    self->btn_metrics = btn;
#endif
}

void XViewLog__v_update(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewLog);
    XViewLog *self = view;
    
    int lvl = o_log_level();

    struct o_log_ring_entry logs[O_LOG_RING_SIZE];
    int logs_last;
    o_log_ring_copy(logs, &logs_last);

    struct {
        int log_idx;
        int char_idx;
    } log_positions[O_LOG_RING_SIZE];
    int log_positions_num = 0;

    char *dst = self->text_buf;
    for (int i=0; i<O_LOG_RING_SIZE; i++) {
        int log_idx = (logs_last + i) % O_LOG_RING_SIZE;
        if (*logs[log_idx].msg == '\0') {
            continue;
        }
        if (logs[log_idx].level < lvl) {
            continue;
        }
        
        log_positions[log_positions_num].log_idx = log_idx;
        log_positions[log_positions_num].char_idx = dst - self->text_buf;
        log_positions_num++;
#define base_fmt "%c %s < %s >\n  %s"
        char *fmt = i<O_LOG_RING_SIZE-1 ? base_fmt "\n" : base_fmt;
#undef base_fmt
        int written = snprintf(dst, XViewLog_LOG_BUF_SIZE, fmt,
            log_level_names[logs[log_idx].level], logs[log_idx].tag, logs[log_idx].func, logs[log_idx].msg);
        
        if (self->filter && *self->filter!='\0') {
            if(o_str_find(dst, self->filter)<0) {
                continue;
            }
        }
        
        dst += written;
        assert(dst - self->text_buf <= sizeof(self->text_buf));
    }

    if (log_positions_num == 0 || dst == self->text_buf) {
        o_strf_buf(self->text_buf, "  -  LOGS EMPTY  -");
    }

    vec2 max_size = RCam_size(AView_cam(view));
    RObjText_layout_ref(self->text)->max_size.x = max_size.x;
    vec4 text_color = R_GRAY_X(0.8);
    text_color.a = self->text_alpha;
    vec2 text_size = RObjText_text_set(self->text, self->text_buf, text_color);

    // coloring
    oobj ro_boxes = RObjText_boxes_ro(self->text);
    osize boxes_num = o_num(ro_boxes);
    assert(boxes_num > 0);
    struct r_box *boxes = o_at(ro_boxes, 0);
    for (int i=0; i<log_positions_num; i++) {
        int log_idx = log_positions[i].log_idx;
        int pos = log_positions[i].char_idx;
        struct r_box *box = &boxes[pos];
        assert(pos >=0 && pos < boxes_num);
        int level = logs[log_idx].level;
        assert(level>=0 && level < 6);
        box->fx = log_level_colors[level];
        box->fx.a = self->text_alpha;
        pos++;
        while (self->text_buf[pos] != '\n') {
            box = &boxes[pos];
            assert(pos >=0 && pos < boxes_num);
            box->fx = R_WHITE;
            box->fx.a = self->text_alpha;
            pos++;
        }
    }

    int text_zoom_lvls_num = o_stack_array_num(text_zoom_lvls);
    self->zoom = o_clamp(self->zoom, 0, text_zoom_lvls_num-1);
    float zoom = text_zoom_lvls[self->zoom];
    text_size = vec2_scale(text_size, zoom);
    mat4 *pose = RObjText_pose_ref(self->text);
    if (text_size.y > max_size.y) {
        
        // move the text field up in a way that the text is aligned to text rows
        float diff = text_size.y - max_size.y;
        float off = zoom * RObjText_layout_ref(self->text)->offset.y;
        float move = off - m_mod(diff, off);
        // -0.1 to avoid glitchings from rounding (case we may have none integer text scaling...)
        pose->axis_t.y = -diff -move - 0.1;
    } else {
        pose->axis_t.y = 0;
    }
    pose->axis_x.x = zoom;
    pose->axis_y.y = zoom;

    
    char lvl_txt[4] = {'[', log_level_names[lvl], ']', '\0'};
    WText_text_set(self->level_label, lvl_txt);
    vec4 lvl_color = log_level_colors[lvl];
    lvl_color.a = self->text_alpha*0.8;
    WText_color_set(self->level_label, lvl_color);

    // WText_text_set(self->gui, self->text_buf);
    WTheme_update_full_tex(self->theme, self->gui, tex);
    
    if(WBtn_clicked(self->btn_zoom_plus)) {
        self->zoom++;
    }
    if(WBtn_clicked(self->btn_zoom_minus)) {
        self->zoom--;
    }
    if (WBtn_clicked(self->btn_level_prev)) {
        lvl--;
    }
    if (WBtn_clicked(self->btn_level_next)) {
        lvl++;
    }
    lvl = o_clamp(lvl, 0, O_LOG_ENUM_MAX-1);

#ifndef NDEBUG
    WBtn_down_set(self->btn_metrics, a_app_auto_log_metrics());
    if(WBtn_toggled(self->btn_metrics)) {
        a_app_auto_log_metrics_set(WBtn_down(self->btn_metrics));
    }
#endif

    o_log_level_set(lvl);

    
}

void XViewLog__v_render(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewLog);
    XViewLog *self = view;

    RTex_clear_ex(tex, self->bg_color, true);

    RTex_ro(tex, self->text);
    
    WTheme_render(self->theme, tex);

}

//
// object functions:
//

const char *XViewLog_filter_set(oobj obj, const char *filter)
{
    OObj_assert(obj, XViewLog);
    XViewLog *self = obj;
    o_free(self, self->filter);
    self->filter = o_str_clone(self, filter);
    return self->filter;
}
