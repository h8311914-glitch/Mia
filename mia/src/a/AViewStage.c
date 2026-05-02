#include "a/AViewStage.h"
#include "o/OObj_builder.h"
#include "o/OPtr.h"
#include "o/OArray.h"
#include "o/OEvent.h"
#include "o/timer.h"
#include "m/vec/flt.h"
#include "m/vec/int.h"
#include "m/mat/flt.h"
#include "a/app.h"
#include "a/input.h"
#include "a/pointer.h"

#undef O_LOG_LIB
#define O_LOG_LIB "a"

#include "o/log.h"


// protected
void a__scene_set(oobj opt_scene);

struct vs_scene_info {
    // if a scene got o_del'ed, it should automatically call exit
    // for that, each scene has an internal join object and the weakjoin accessor
    AScene *scene;

    // >NOT< THREADSAFE, but AScene's should not be used in threads...
    oobj ptr;
};


// returns NULL if not available
static struct vs_scene_info *vs_get_scene(AViewStage *self, int idx)
{
    if(idx<0 || idx >=o_num(self->scenes)) {
        return NULL;
    }
    struct vs_scene_info *si = o_at(self->scenes, idx);
    if(!OPtr_available(si->ptr)) {
        return NULL;
    }
    return si;
}


static void AViewStage__v_update(oobj view, oobj tex, float dt)
{
    OObj_assert(view, AViewStage);
    AViewStage *self = view;

    oobj prev_stage = a_stage_try().o;
    a_app_stage_set(self);

    // set viewports and call update
    // backwards, so handled pointer events work
    //     until the first opaque scene
    self->current_scene_start = 0;
    self->current_scene = -1;


    const ivec4 app_unsafe_viewport = a_app_viewport_unsafe();
    const ivec4 app_safe_viewport = a_app_viewport_safe();
    const ivec4 unsafe_viewport = AView_viewport(self);
    ivec4 safe_viewport;
    bool ignore = false;
    if (ivec4_equals_v(unsafe_viewport, app_unsafe_viewport)) {
        safe_viewport = app_safe_viewport;
    } else {
        mat4 display_pose = AView_display_pose(self);
        const mat4 full = mat4_eye();
        // fullscreen if display pose is full gl coords AND viewport begins at 0, 0 (makes the calcs below much easier)
        bool full_screen = vecn_equals_eps_v(display_pose.v, full.v, 0.01, 16)
                && ivec2_equals(unsafe_viewport.xy, 0);
        if (!full_screen) {
            // just ignore it, the AViewStage is somewhere but not full screen
            safe_viewport = unsafe_viewport;
            ignore = true;
        } else {
            // full screen, but different viewport sizes!
            vec4 rel = vec4_(
                (float) app_safe_viewport.left / (float) app_unsafe_viewport.width,
                (float) app_safe_viewport.top / (float) app_unsafe_viewport.height,
                (float) app_safe_viewport.width / (float) app_unsafe_viewport.width,
                (float) app_safe_viewport.height / (float) app_unsafe_viewport.height,
            );
            safe_viewport = ivec4_(
                    (int) (rel.left * unsafe_viewport.width),
                    (int) (rel.top * unsafe_viewport.height),
                    (int) (rel.width * unsafe_viewport.width),
                    (int) (rel.height * unsafe_viewport.height),
            );
        }
    }

    vec4 rel_padding;
    if (ignore) {
        rel_padding = vec4_(0);
    } else {
        rel_padding = vec4_(
            (float) safe_viewport.left / (float) unsafe_viewport.width,
            (float) safe_viewport.top / (float) unsafe_viewport.height,
            (float) (unsafe_viewport.width - (safe_viewport.width + safe_viewport.left)) / (float) unsafe_viewport.width,
            (float) (unsafe_viewport.height - (safe_viewport.height + safe_viewport.top)) / (float) unsafe_viewport.height,
        );
    }

    // traverse through the scenes list and collect infos
    AScene *app_title_scene = NULL;
    AScene *app_icon_scene = NULL;
    for (int s = (int) o_num(self->scenes) - 1; s >= 0; s--) {
        struct vs_scene_info *si = vs_get_scene(self, s);
        if(!si) {
            AViewStage_scene_exit_instant_for(self, s);
            continue;
        }
        if (self->current_scene_start == 0 && AScene_opaque(si->scene)) {
            self->current_scene_start = s;
        }
        if (self->current_scene_start>0) {
            si->scene->resuming = true;
        }
        if (!app_title_scene && si->scene->app_title) {
            app_title_scene = si->scene;
        }
        if (!app_icon_scene && si->scene->app_icon) {
            app_icon_scene = si->scene;
        }
    }

    // protected
    void a_app__title_set(const char *set);
    void a_app__icon_set(void *surface_icon);

    if (app_title_scene) {
        if (app_title_scene != self->app_title_scene || app_title_scene->app_title_set) {
            a_app__title_set(app_title_scene->app_title);
            app_title_scene->app_title_set = false;
        }
        self->app_title_scene = app_title_scene;
    } else {
        self->app_title_scene = NULL;
        // auto set to the title
        a_app__title_set(NULL);
    }

    if (app_icon_scene) {
        if (app_icon_scene != self->app_icon_scene || app_icon_scene->app_icon_set) {
            a_app__icon_set(app_icon_scene->app_icon);
            app_icon_scene->app_icon_set = false;
        }
        self->app_icon_scene = app_icon_scene;
    } else {
        self->app_icon_scene = NULL;
    }


    for (int s = (int) o_num(self->scenes) - 1; s >= self->current_scene_start; s--) {
        struct vs_scene_info *si = vs_get_scene(self, s);
        if(!si) {
            continue;
        }
        if (s == o_num(self->scenes) - 1 && si->scene->escape_event) {
            if (a_input_pressed(A_INPUT_ESCAPE)) {
                si->scene->escape_event(si->scene);
            }
        }


        ou64 scene_timer = o_timer();

        si->scene->safe_padding_rel = vec4_(0);

        if (si->scene->mode == AScene_SAFE) {
            si->scene->viewport = safe_viewport;
        } else if (si->scene->mode == AScene_UNSAFE) {
            si->scene->viewport = unsafe_viewport;
            si->scene->safe_padding_rel = rel_padding;
        }

        oobj prev_scene = a_scene_try().o;
        self->current_scene = s;
        a__scene_set(si->scene);
        AView_update(si->scene->view, tex, si->scene->viewport);
        a__scene_set(prev_scene);
        self->current_scene = -1;
    }

    a_app_stage_set(prev_stage);
}


static void AViewStage__v_render(oobj view, oobj tex, float dt)
{
    OObj_assert(view, AViewStage);
    AViewStage *self = view;

    oobj prev_stage = a_stage_try().o;
    a_app_stage_set(self);
    
    // call render
    for (int s = self->current_scene_start; s < o_num(self->scenes); s++) {
        struct vs_scene_info *si = vs_get_scene(self, s);
        if(!si) {
            continue;
        }

        ou64 scene_timer = o_timer();

        oobj prev_scene = a_scene_try().o;
        self->current_scene = s;
        a__scene_set(si->scene);
        AView_render(si->scene->view, tex);
        si->scene->resuming = false;
        a__scene_set(prev_scene);
        self->current_scene = -1;
    }

    a_app_stage_set(prev_stage);
}

AViewStage *AViewStage_init(oobj obj, oobj parent)
{
    AView *super = obj;
    AViewStage *self = obj;

    o_clear(self, sizeof *self, 1);

    // call super init
    AView_init(obj, parent, NULL, AViewStage__v_update, AViewStage__v_render);
    OObj_id_set(self, AViewStage_ID);


    self->scenes = OArray_new_dyn(self, NULL, sizeof(struct vs_scene_info), 0, 16);
    self->current_scene = -1;

    return self;
}

//
// object functions
//


struct vs_event_add_context {
    AViewStage *self;
    AScene *scene;
};

static void vs_event_add_scene(oobj defer)
{
    struct vs_event_add_context *C = o_user(defer);
    struct vs_scene_info si = {0};
    si.scene = C->scene;
    si.ptr = OPtr_new(C->self, C->scene);
    OArray_push(C->self->scenes, &si);
}

void AViewStage__scene_register(oobj obj, AScene *scene)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;

    // register deffered in the next frame
    // Using scene as parent, so if scene is deleted (instantly in the same frame...), the defer gets unregistered
    oobj event = OEvent_new(scene, vs_event_add_scene, NULL);
    struct vs_event_add_context *C = o_user_new0(event, struct vs_event_add_context, 1);
    C->self = self;
    C->scene = scene;
    OEvent_post(event);
}


int AViewStage_scenes_num(oobj obj)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;
    return (int) o_num(self->scenes);
}

struct oobj_opt AViewStage_scene_at(oobj obj, int index)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;
    if (index < 0 || index >= o_num(self->scenes)) {
        return oobj_opt(NULL);
    }
    return oobj_opt(OArray_at(self->scenes, index, struct vs_scene_info)->scene);
}

int AViewStage_scene_index_for(oobj obj, oobj scene)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;
    for (int i = 0; i < OArray_num(self->scenes); i++) {
        if (OArray_at(self->scenes, i, struct vs_scene_info)->scene == scene) {
            return i;
        }
    }
    return -1;
}

int AViewStage_scene_index_try(oobj obj)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;
    return self->current_scene;
}

void AViewStage_scene_switch_stack_position(oobj obj, int scene_index_a, int scene_index_b)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;
    o_thread_assert_main();

    o_log_debug_s(__func__, "switching scene stack position of %i <> %i", scene_index_a, scene_index_b);

    osize num = o_num(self->scenes);
    if(scene_index_a<0 || scene_index_a>=num) {
        o_log_error_s(__func__, "index a is out of bounds");
        return;
    }
    if(scene_index_b<0 || scene_index_b>=num) {
        o_log_error_s(__func__, "index b is out of bounds");
        return;
    }
    if(scene_index_a == scene_index_b) {
        o_log_warn_s(__func__, "index a == b, ignoring");
        return;
    }

    struct vs_scene_info *a = o_at(self->scenes, scene_index_a);
    struct vs_scene_info *b = o_at(self->scenes, scene_index_b);
    struct vs_scene_info tmp = *a;
    *a = *b;
    *b = tmp;
}

void AViewStage_scene_exit_instant_for(oobj obj, int scene_index)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;
    o_thread_assert_main();

    // prevent a recursion (needed?)
    if (self->scene_exit_active) {
        return;
    }
    self->scene_exit_active = true;

    if (self->current_scene >= scene_index) {
        o_log_warn_s(__func__,
                     "deleted in active scene?: %i>=%i", self->current_scene, scene_index);
    }

    if (scene_index < 0 || scene_index >= o_num(self->scenes)) {
        o_log_debug_s(__func__, "invalid scene index: %i", scene_index);
        return;
    }

    // pop all back until scene_index (incl.)
    // deletor will call this function, that's why the flag .scene_exit_active is set to prevent recursion
    // if the deletor of AScene is called instead of this function, the call to AViewStage__scene_exit_ignore
    //      sets the ignore index (+1)
    for (int i = (int) o_num(self->scenes) - 1; i >= scene_index; i--) {
        struct vs_scene_info *si = OArray_at(self->scenes, i, struct vs_scene_info);
        if (OPtr_get(si->ptr).o) {
            o_del(si->scene);
        }
        o_del(si->ptr);
    }
    OArray_resize(self->scenes, scene_index);

    self->current_scene = o_min(self->current_scene, o_num(self->scenes) - 1);

    self->scene_exit_active = false;

    // protected: reset touch on scene exit...
    void a_pointer__reset_touch_instantly(void);
    a_pointer__reset_touch_instantly();
}


struct vs_event_exit_context {
    AViewStage *self;
    int scene_index;
};

static void event_exit_scene(oobj event)
{
    struct vs_event_exit_context *C = o_user(event);
    o_log_s("AViewStage", "exiting scene %i", C->scene_index);
    AViewStage_scene_exit_instant_for(C->self, C->scene_index);
}

void AViewStage_scene_exit_for(oobj obj, int scene_index)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;

    o_thread_assert_main();

    if(scene_index<0) {
        scene_index = AViewStage_scene_index(self);
    }
    oobj event = OEvent_new(self, event_exit_scene, NULL);
    struct vs_event_exit_context *C = o_user_new0(event, struct vs_event_exit_context, 1);
    C->self = self;
    C->scene_index = scene_index;
    OEvent_post(event);
}

void AViewStage_scene_exit_pop_instant_for(oobj obj, int scene_index)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;
    o_thread_assert_main();

    int scenes = AViewStage_scenes_num(self);
    if (scene_index<0 || scene_index>=scenes) {
        o_log_warn_s(__func__, "scene_index %i is out of range (%i)", scene_index, scenes);
        return;
    }

    // switch to the stack end (instant)
    for (int i=scene_index; i<scenes-1; i++) {
        AViewStage_scene_switch_stack_position(self, i, i+1);
    }
    // exit the moved scene
    AViewStage_scene_exit_instant_for(self, scenes-1);
}

static void event_exit_pop_scene(oobj event)
{
    struct vs_event_exit_context *C = o_user(event);
    o_log_s("AViewStage", "popping scene %i", C->scene_index);
    AViewStage_scene_exit_pop_instant_for(C->self, C->scene_index);
}

void AViewStage_scene_exit_pop_for(oobj obj, int scene_index)
{
    OObj_assert(obj, AViewStage);
    AViewStage *self = obj;

    o_thread_assert_main();

    if(scene_index<0) {
        scene_index = AViewStage_scene_index(self);
    }
    oobj event = OEvent_new(self, event_exit_pop_scene, NULL);
    struct vs_event_exit_context *C = o_user_new0(event, struct vs_event_exit_context, 1);
    C->self = self;
    C->scene_index = scene_index;
    OEvent_post(event);
}



