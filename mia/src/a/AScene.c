#include "a/AScene.h"
#include "o/OObj_builder.h"
#include "o/ODelcallback.h"
#include "o/img.h"
#include "a/app.h"
#include "a/AViewStage.h"
#include "o/str.h"
#include "r/RCam.h"



//
// public
//

AScene *AScene_init_ex(oobj obj, oobj parent, oobj stage, oobj view, bool move_view, enum AScene_mode mode)
{
    AScene *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, AScene_ID);

    self->stage = stage;

    // protected
    void AViewStage__scene_register(oobj, AScene *);
    AViewStage__scene_register(stage, self);

    self->opaque = true;
    self->mode = mode;

    if(view && move_view) {
        o_move(view, self);
    }
    AScene_view_set(self, view, false);

    return self;
}

AScene *AScene_new(oobj parent, oobj view, bool move_view, enum AScene_mode mode)
{
    return AScene_new_ex(parent, a_stage(), view, move_view, mode);
}

//
// object functions
//

AView *AScene_view_set(oobj obj, oobj view, bool del_old)
{
    OObj_assert(obj, AScene);
    AScene *self = obj;
    if(del_old) {
        o_del(self->view);
    }
    self->view = view;
    return self->view;
}

vec4 AScene_safe_padding(oobj obj)
{
    OObj_assert(obj, AScene);
    AScene *self = obj;
    vec2 size = RCam_proj_size(AView_cam(self->view));
    return vec4_(
        self->safe_padding_rel.v0 * size.x,
        self->safe_padding_rel.v1 * size.y,
        self->safe_padding_rel.v2 * size.x,
        self->safe_padding_rel.v3 * size.y
    );
}
int AScene_index(oobj obj)
{
    return AViewStage_scene_index_for(a_stage(), obj);
}

void AScene_exit(oobj obj)
{
    AViewStage_scene_exit_for(a_stage(), AScene_index(obj));
}

void AScene_exit_pop(oobj obj)
{
    AViewStage_scene_exit_pop_for(a_stage(), AScene_index(obj));
}

void AScene_escape_event_set_exit(oobj obj)
{
    AScene_escape_event_set(obj, AScene_exit);
}

void AScene_title_set(oobj obj, const char *opt_title)
{
    OObj_assert(obj, AScene);
    AScene *self = obj;
    o_free(self, self->app_title);
    self->app_title = o_str_clone(obj, opt_title);
    self->app_title_set = true;
}

void AScene_icon_set(oobj obj, struct o_img *opt_icon)
{
    OObj_assert(obj, AScene);
    AScene *self = obj;

    self->app_icon_set = true;

    // protected
    void *a_app__icon_create(const struct o_img *set);
    void a_app__icon_free(void *surface_icon);

    if (self->app_icon) {
        a_app__icon_free(self->app_icon);
        self->app_icon = NULL;
    }
    if (!opt_icon) {
        return;
    }
    self->app_icon = a_app__icon_create(opt_icon);
}

void AScene_icon_set_file(oobj obj, const char *opt_file)
{
    struct o_img icon = o_img_new_file(obj, opt_file, o_img_RGBA);
    if (!icon.data) {
        AScene_icon_set(obj, NULL);
    } else {
        AScene_icon_set(obj, &icon);
        o_img_free(&icon);
    }
}
