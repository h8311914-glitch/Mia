#include "x/toast.h"
#include "o/OPtr.h"
#include "r/RTex_blit.h"
#include "r/RCam.h"
#include "a/AView.h"
#include "a/app.h"
#include "a/AViewStage.h"
#include "w/WTheme.h"
#include "w/WAlign.h"
#include "w/WPane.h"
#include "w/WTextShadow.h"


#undef O_LOG_LIB
#define O_LOG_LIB "x"
#include "o/log.h"

#define TOAST_HEIGHT_WEIGHT 0.8f
#define TOAST_FADE_TIME 0.5f

static struct {
    bool installed;

    oobj view;
    oobj theme;

    oobj opt_parent_ptr;
    oobj tex;
    float time;

} toast_L;


static void toast_update(oobj view, oobj tex, float dt)
{
    // noop
}

static void toast_render(oobj view, oobj tex, float dt)
{
    if (toast_L.time<=0) {
        return;
    }

    if (toast_L.opt_parent_ptr && !OPtr_available(toast_L.opt_parent_ptr)) {
        toast_L.time = 0;
        // also clears to NULL...
        o_del(toast_L.opt_parent_ptr);
    }
    toast_L.time-=dt;
    if (toast_L.time<=0) {
        toast_L.time = 0;
        o_del(toast_L.tex);
        return;
    }
    vec2 toast_size = RTex_size(toast_L.tex);
    vec2 cam_size = RCam_size(a_cam());

    vec2 room = vec2_max(vec2_sub_v(cam_size,toast_size), 0);

    vec2 pos = {{room.x*0.5, room.y * TOAST_HEIGHT_WEIGHT}};

    float alpha = m_clamp(toast_L.time / TOAST_FADE_TIME, 0.0f, 1.0f);

    vec4 color = {{1, 1, 1, alpha}};
    RTex_blend_color(tex, toast_L.tex, m_2(pos), color, vec4_(0));
}

//
// public
//



void x_toast_install(void)
{
    o_thread_assert_main();

    if(toast_L.installed) {
        o_log_s(__func__, "already installed");
        return;
    }
    toast_L.installed = true;
    
    oobj view = a_app_main_view();
    OObj_assert(view, AViewStage);
    struct AView_layer layer = {NULL, toast_update, toast_render};
    AView_push_layer_deferred(view, layer, true);

    toast_L.view = view;
    toast_L.theme = WTheme_new_tiny(toast_L.view);

}

bool x_toast_installed(void)
{
    return toast_L.installed;
}


float x_toast_current_time(void)
{
    return toast_L.time;
}

float x_toast_current_time_set(float set)
{
    if(!toast_L.installed) {
        return 0;
    }
    o_thread_assert_main();
    if (toast_L.tex) {
        toast_L.time = set;
    } else {
        o_log_warn_s(__func__, "failed, nothing is displayed");
    }
    return toast_L.time;
}

struct oobj_opt x_toast_current_tex(void)
{
    return oobj_opt(toast_L.tex);
}


void x_toast_tex(oobj opt_parent, oobj tex, float time)
{
    if(!toast_L.installed) {
        return;
    }
    o_thread_assert_main();
    OObj_assert(tex, RTex);
    assert(time>0);
    o_del(toast_L.opt_parent_ptr);
    o_del(toast_L.tex);
    if (opt_parent) {
        toast_L.opt_parent_ptr = OPtr_new(toast_L.view,  opt_parent);
    }
    toast_L.tex = tex;
    o_move(tex, toast_L.view);
    toast_L.time = time;
}

void x_toast_wobj(oobj opt_parent, oobj wobj, float time)
{
    if(!toast_L.installed) {
        return;
    }
    vec2 size = WTheme_update(toast_L.theme, wobj, vec2_(0), vec2_(0));
    size = vec2_ceil(size);
    oobj tex = RTex_new(toast_L.view, NULL, m_2(size));
    RTex_clear_full(tex, R_TRANSPARENT);
    WTheme_render(toast_L.theme, tex);
    x_toast_tex(opt_parent, tex, time);
}

void x_toast_text_ex(oobj opt_parent, const char *text, float time,
        const vec4 *opt_pane_color, const vec4 *opt_text_color)
{
    if(!toast_L.installed) {
        return;
    }

    oobj wobj_pane = WPane_new(a_tmp());
    if (opt_pane_color) {
        WPane_color_set(wobj_pane, *opt_pane_color);
    }
    WPane_style_set(wobj_pane, WPane_ROUND);

    oobj wobj_text = WText_new(wobj_pane, text);
    if (opt_text_color) {
        WText_color_set(wobj_text, *opt_text_color);
    }
    WObj_padding_set(wobj_text, vec4_(4));
    x_toast_wobj(opt_parent, wobj_pane, time);
}

