#include "x/XViewTex.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "r/RObjQuad.h"
#include "r/RObjText.h"
#include "r/RCam.h"
#include "a/app.h"
#include "a/input.h"
#include "u/pose.h"
#include "w/WTheme.h"
#include "w/WBox.h"
#include "w/WBtn.h"
#include "w/WIcon.h"
#include "w/WTextShadow.h"

#undef O_LOG_LIB
#define O_LOG_LIB "x"
#include "o/log.h"


#define MOVE_TIMEOUT 1.5f
#define WHEEL_ZOOM 1.1f


static void tex_view_cam_home(XViewTex *self)
{
    vec4 rect = vec4_(0);
    rect.zw = RTex_size(self->tex);
    RCam_fit(AView_cam(self->tex_view), rect, false, 1.4f);
}

    

static void tex_view_setup(oobj view)
{
    XViewTex *self = o_user(view);
    self->zoom = u_zoom_new(a_cam());
    tex_view_cam_home(self);
}

static void tex_view_update(oobj view, oobj tex, float dt)
{
    XViewTex *self = o_user(view);
    bool move = u_zoom_update(&self->zoom, dt);
    if(!move) {
        if(self->v_opt_pointer) {
            self->v_opt_pointer(self, a_pointer(0, 0));
        }
    }
}

static void tex_view_render(oobj view, oobj tex, float dt)
{
    XViewTex *self = o_user(view);
    RTex_clear(tex, R_BLACK);
    RTex_blit(tex, self->tex, 0, 0);
}


//
// public
//

XViewTex *XViewTex_init(oobj obj, oobj parent, oobj tex, bool move_tex, OObj__event_fn done,
                        const char *opt_title, XViewTex_pointer_fn opt_pointer)
{

    AView *super = obj;
    XViewTex *self = obj;
    o_clear(self, sizeof *self, 1);

    AView_init(obj, parent, XViewTex__v_setup, XViewTex__v_update, XViewTex__v_render);
    OObj_id_set(self, XViewTex_ID);

    self->tex = tex;
    if(move_tex) {
        o_move(tex, self);
    }
    
    self->tex_view = AView_new(self, tex_view_setup, tex_view_update, tex_view_render);
    o_user_set(self->tex_view, self);

    self->theme = WTheme_new_tiny(self);
    self->gui = WBox_new_v(self);

    oobj btn_title_box = WBox_new_h(self->gui);
    WBox_spacing_set(btn_title_box, vec2_(4));
    
    // button padding
    WObj_padding_ref(btn_title_box)->v3 = 2;

    self->exit_btn = WBtn_new(btn_title_box);
    oobj btn_icon = WIcon_new(self->exit_btn, WTheme_ICON_CROSS);
    WObj_padding_set(btn_icon, vec4_(2));
    WIcon_color_set(btn_icon, vec4_(0.8, 0.1, 0.1, 1.0));
    
    self->cam_btn = WBtn_new(btn_title_box);
    btn_icon = WIcon_new(self->cam_btn, WTheme_ICON_CAM);
    WObj_padding_set(btn_icon, vec4_(2));

    self->save_btn = WBtn_new(btn_title_box);
    btn_icon = WIcon_new(self->save_btn, WTheme_ICON_SAVE);
    WObj_padding_set(btn_icon, vec4_(2));

    self->title_text = WTextShadow_new(btn_title_box, opt_title);
    WObj_padding_set(self->title_text, vec4_(2, 4, 0));

    self->save_file = o_strf(self, "#XViewTex__%s.png", opt_title? opt_title : "NULL");
    
    // vfuncs
    self->v_done = done;
    self->v_opt_pointer = opt_pointer;

    return self;
    
}


//
// virtual implementations
//

void XViewTex__v_setup(oobj view)
{
    // noop
}

void XViewTex__v_update(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewTex);
    XViewTex *self = view;
    
    AView_update(self->tex_view, tex, AView_viewport(self));
    
    
    WTheme_update(self->theme, self->gui, vec2_(16), vec2_(0));

    if(WBtn_clicked(self->exit_btn)) {
        self->v_done(self);
    }
    
    if(WBtn_clicked(self->cam_btn)) {
        tex_view_cam_home(self);
    }

    if (WBtn_clicked(self->save_btn)) {
        RTex_write_file(self->tex, self->save_file);
    }
    
}

void XViewTex__v_render(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewTex);
    XViewTex *self = view;

    AView_render(self->tex_view, tex);
    
    WTheme_render(self->theme, tex);
}

//
// object functions:
//



oobj XViewTex_tex_set(oobj obj, oobj tex, bool del_old)
{
    OObj_assert(obj, XViewTex);
    XViewTex *self = obj;
    if(del_old) {
        o_del(self->tex);
    }
    self->tex = tex;
    return tex;
}
