#include "u/splash.h"
#include "o/OArray.h"
#include "o/OEvent.h"
#include "o/str.h"
#include "r/RObjText.h"
#include "r/RTex.h"
#include "r/RCam.h"
#include "a/app.h"
#include "a/AView.h"
#include "a/AScene.h"
#include "a/AViewStage.h"
#include "u/pose.h"
#include "m/vec/byte.h"

#undef O_LOG_LIB
#define O_LOG_LIB "u"
#include "o/log.h"

#define SPLASH_INJECT "u_splash_context"



#define MIA_MIN_UNITS 128
#define MIA_TEXT_SHIFT_X 60
#define MIA_TEXT_SHIFT_Y 60
#define MIA_STEPS 16
#define MIA_SKIP_TIME 0.25
#define MIA_SKIP_CNT 2

struct splash_context {
    oobj splash_scene;

    float time;
    oobj parent;
    oobj actual_view;
    bool move_actual_view_in_its_scene;
    enum AScene_mode actual_scene_mode;
    oobj actual_scene;
    oobj stage;
};


static void splash_event_switch(oobj event)
{
    struct splash_context *C = o_user(event);
    int scene_splash = AViewStage_scene_index_for(C->stage, C->splash_scene);
    int scene_actual = AViewStage_scene_index_for(C->stage, C->actual_scene);
    AViewStage_scene_switch_stack_position(C->stage, scene_actual, scene_splash);
}


static void splash_update(oobj view, oobj tex, float dt)
{
    struct oobj_opt inject = OObj_find(view, OObj, SPLASH_INJECT, 0);
    if (!inject.o) {
        o_log_error_s("u_splash_layer_update", "failed to find inject object");
        return;
    }
    struct splash_context *C = o_user(inject.o);
    if (C->time >= 0) {
        C->time -= dt;
        if (C->time < 0) {
            o_log_s("u_splash", "creates actual scene");
            C->actual_scene = AScene_new(C->parent, C->actual_view, C->move_actual_view_in_its_scene, C->actual_scene_mode);
            // AScene registration runs deferred, so we also create a defer which will get executed directly after that
            oobj event = OEvent_new(view, splash_event_switch, NULL);
            o_user_set(event, C);
            OEvent_post(event);
        }
    }
    
    a_app_events_handle();
}

static void splash_render(oobj view, oobj tex, float dt)
{
    // noop
}

oobj u_splash_new(oobj parent,
                  oobj splash_view, bool move_splash_view, enum AScene_mode splash_scene_mode,
                  oobj actual_view, bool move_actual_view_in_its_scene, enum AScene_mode actual_scene_mode,
                  float time_until_actual_scene_creation)
{
    struct AView_layer layer = {0};
    layer.update = splash_update;
    layer.render = splash_render;
    OArray_push_front(AView_layers(splash_view), &layer);
    oobj inject = OObj_new(splash_view);
    OObj_name_set(inject, SPLASH_INJECT);
    struct splash_context *C = o_user_new0(inject, *C, 1);
    C->time = time_until_actual_scene_creation;
    C->parent = parent;
    C->actual_view = actual_view;
    C->move_actual_view_in_its_scene = move_actual_view_in_its_scene;
    C->actual_scene_mode = actual_scene_mode;

    oobj scene = AScene_new(parent, splash_view, move_splash_view, splash_scene_mode);
    C->splash_scene = scene;
    C->stage = a_stage();

    return scene;
}

struct oobj_opt u_splash_actual_scene(oobj splash_view)
{
    struct oobj_opt inject = OObj_find(splash_view, OObj, SPLASH_INJECT, 0);
    if (!inject.o) {
        o_log_error_s(__func__, "failed to find inject object");
        return oobj_opt(NULL);
    }
    struct splash_context *C = o_user(inject.o);
    return oobj_opt(C->actual_scene);
}

void u_splash_invoke_creation(oobj splash_view)
{
    struct oobj_opt inject = OObj_find(splash_view, OObj, SPLASH_INJECT, 0);
    if (!inject.o) {
        o_log_error_s(__func__, "failed to find inject object");
        return;
    }
    struct splash_context *C = o_user(inject.o);
    // so next update invokes it, only if >=0
    C->time = 0;
}

//
// mia splash
//

struct mia_splash_context {
    char *author;
    char *app;
    vec4 bg_color;
    vec4 shell_color;
    vec4 cmd_color;
    vec4 text_color;
    float time;
    float min_time;
    bool skip_prevented;
    oobj ro;
    int steps[MIA_STEPS];
    
    bool skipable;
    int skip_cnt;
    float skip_time;
    bool skipping;
};

static void mia_splash_setup(oobj view)
{
    AView_cam_min_units_set(view, vec2_(MIA_MIN_UNITS));

    // use own tex to get fade options
    AView_use_own_tex_set(view, true);
    AView_render_colored_set(view, true);
    AView_scale_auto_set(view, true);

    struct mia_splash_context *C = o_user(view);
    bvec4 shadow = bvec4_cast_float_1(R_GRAY_X(0.10).v);
    C->ro = RObjText_new_font35_shadow(view, NULL, &shadow);
    RObjText_casing_set(C->ro, r_textlayout_casing_UNCHANGED);
    for (int i = 0; i < MIA_STEPS; i++) {
        int step = i * 100 / (MIA_STEPS - 1) - 2 + o_rand() % 4;
        C->steps[i] = o_clamp(step, 1, 99);
    }

    // As we often change the text, use dyn mode
    oobj boxes = RObjText_boxes(C->ro);
    OArray_dyn_realloc_mode_set(boxes, OArray_REALLOC_DOUBLED);
}

static void mia_splash_update(oobj view, oobj tex, float dt)
{
    struct mia_splash_context *C = o_user(view);
    if(!C->skipable) {
        return;
    }
    if(C->skipping && u_splash_actual_scene(view).o) {
        C->time = C->min_time;
        C->skipable = false;
        return;
    }
    C->skip_time-=dt;
    if(C->skip_time<=0) {
        C->skip_cnt = 0;
    }
    bool pressed = a_pointer_pressed(a_pointer(0, 0));
    if(!pressed) {
        return;
    }
    C->skip_cnt++;
    C->skip_time = MIA_SKIP_TIME;
    if(C->skip_cnt>=MIA_SKIP_CNT) {
        o_log_s(__func__, "skipping");
        C->skipping = true;
        u_splash_invoke_creation(view);
    }
}


static void mia_splash_render(oobj view, oobj tex, float dt)
{
    struct mia_splash_context *C = o_user(view);
    vec4 bg_color = C->bg_color;
    float view_alpha = 1.0;
    if (C->time < C->min_time) {
        float t = C->time / C->min_time;
        C->time += dt;
        
        // prevent skipping last 20%
        float next_t = C->time / C->min_time;
        if(t<0.8 && next_t>=0.9 && !C->skip_prevented) {
            // reset to 90% fps to prevent skipping all
            C->time = C->min_time*0.8;
            C->skip_prevented = true;
        }
       
        char *text = " ";
        if (t < 0.05) {
            text = " ";
        } else if (t < 0.125) {
            text = ">";
        } else if (t < 0.175) {
            text = "> |";
        } else if (t < 0.200) {
            text = ">";
        } else if (t < 0.225) {
            text = "> BOOT MIA\n>";
        } else if (t < 0.275) {
            text = "> BOOT MIA\n> |";
        } else if (t < 0.300) {
            text = "> BOOT MIA\n>";
        } else if (t < 0.325) {
            text = o_strf_a("> BOOT MIA\n> DEV  %s\n>", C->author);
        } else if (t < 0.375) {
            text = o_strf_a("> BOOT MIA\n> DEV  %s\n> |", C->author);
        } else if (t < 0.400) {
            text = o_strf_a("> BOOT MIA\n> DEV  %s\n>", C->author);
        } else {
            float percentage_raw = (t - 0.400) / (1.000 - 0.400);
            int step = percentage_raw * MIA_STEPS;
            step = o_clamp(step, 0, MIA_STEPS-1);
            int percentage = C->steps[step];
            text = o_strf_a("> BOOT MIA\n> DEV  %s\n> LOAD %s  [ %02d%% ]", C->author, C->app, percentage);

            if (t >= 0.95) {
                float fade_t = (t - 0.95f) / 0.05;
                fade_t = o_min(1.0f, fade_t);
                view_alpha = m_mix(1.0, 0.0, fade_t);
            }
        }
        RObjText_text_set(C->ro, text, C->text_color);
        oobj boxes = RObjText_boxes_ro(C->ro);
        osize boxes_num = o_num(boxes);
        struct r_box *vboxes = o_at(boxes, 0);
        int line = 0;
        int line_col = 0;
        int start_percent = 9 + o_strlen(C->app);
        for (osize i = 0; i < boxes_num; i++, line_col++) {
            struct r_box *box = vboxes + i;
            if (text[i] == '\n') {
                line_col = -1;
                line++;
                continue;
            }
            if (line_col == 0) {
                box->fx = C->shell_color;
            } else if (line_col <= 6) {
                box->fx = C->cmd_color;
            }

            if (line == 2 && line_col >= start_percent) {
                if ((line_col == start_percent) || (line_col == start_percent + 6)) {
                    box->fx = C->shell_color;
                } else {
                    box->fx = C->cmd_color;
                }
            }
        }
        // update should be queued, just to be sure
        RObjText_queue_update_set(C->ro, true);
    }
    
    if (C->time >= C->min_time && u_splash_actual_scene(view).o) {
        a_scene_exit();
    }

    RTex_clear_full(tex, bg_color);
    vec2 center = RCam_center(AView_cam(view));
    RObjText_pose_set(C->ro, u_pose_new(center.x - MIA_TEXT_SHIFT_X, center.y - MIA_TEXT_SHIFT_Y, 1, 1));
    RTex_ro(tex, C->ro);

    AView_render_colored_rgba_ref(view)->a = view_alpha;
}


oobj u_splash_new_mia_ex(oobj parent,
                         oobj actual_view, bool move_actual_view_in_its_scene, enum AScene_mode actual_scene_mode,
                         vec4 bg_color, vec4 shell_color, vec4 cmd_color, vec4 text_color,
                         const char *author, const char *app,
                         float min_time,
                         bool skipable)
{
    assert(o_str_find(author, "\n") < 0);
    assert(o_str_find(app, "\n") < 0);

    assert(o_strlen(author) <= (MIA_TEXT_SHIFT_X*2/4-8) && "out of range");
    assert(o_strlen(app) <= (MIA_TEXT_SHIFT_X*2/4-16) && "out of range");

    oobj splash = AView_new(parent, mia_splash_setup, mia_splash_update, mia_splash_render);
    struct mia_splash_context *C = o_user_new0(splash, *C, 1);
    C->author = o_str_clone(splash, author);
    C->app = o_str_clone(splash, app);
    C->bg_color = bg_color;
    C->shell_color = shell_color;
    C->cmd_color = cmd_color;
    C->text_color = text_color;
    C->min_time = min_time;
    C->skipable = skipable;
    float creation_time = min_time * 0.5f;
    oobj scene = u_splash_new(parent, splash, true, AScene_UNSAFE,
        actual_view, move_actual_view_in_its_scene, actual_scene_mode, creation_time);
    AScene_opaque_set(scene, false);
    return scene;
}

oobj u_splash_new_mia(oobj parent,
                      oobj actual_view, bool move_actual_view_in_its_scene, enum AScene_mode actual_scene_mode,
                      const char *author, const char *app,
                      float min_time,
                      bool skipable)
{
    return u_splash_new_mia_ex(parent, actual_view, move_actual_view_in_its_scene, actual_scene_mode,
                               R_BLACK, vec4_(0.9, 1.0, 0.8, 0.7), vec4_(0.9, 1.0, 0.8, 0.9), R_WHITE,
                               author, app, min_time, skipable);
}
