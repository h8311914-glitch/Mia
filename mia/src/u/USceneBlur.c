#include "u/USceneBlur.h"
#include "o/OObj_builder.h"
#include "m/vec/flt.h"
#include "a/app.h"
#include "a/AScene.h"
#include "a/AView.h"
#include "r/RCam.h"
#include "r/RTex.h"
#include "r/RTex_blit.h"
#include "r/RTex_imgproc.h"
#include "o/OList.h"
#include "u/USamplerVec.h"
#include "u/UAnimator.h"
#include "x/iui.h"

#undef O_LOG_LIB
#define O_LOG_LIB "u"
#include "o/log.h"


static void blur_setup(oobj view)
{
    
}

static void blur_update(oobj view, oobj tex, float dt)
{
    USceneBlur *self = o_user(view);
    OObj_assert(self, USceneBlur);

    if (self->handle_events) {
        a_app_events_handle();
    }


    //*
    if (x_iui_clicked("reset")) {
        UAnimator_reset(self->animator, 0);
        UAnimator_pause_set(self->animator, false);
    }
    //*/
}

static void blur_render(oobj view, oobj tex, float dt)
{
    USceneBlur *self = o_user(view);
    OObj_assert(self, USceneBlur);

    UAnimator_run(self->animator, dt);
        
    if (self->sigma.x < 0 && self->sigma.y < 0) {
        return;
    }

    oobj cam = AView_cam(view);
    vec2 size = RCam_size(cam); 
    vec2 size_gauss = vec2_ceil(size);
    
    oobj blit = RTex_new_blit_back(view);
    
    oobj scale = RTex_resize(blit, m_2(size_gauss), 0);
    oobj gauss = RTex_gauss(scale, ivec2_(7), self->sigma, 0);
    RTex_blit(tex, gauss, 0, 0);
    o_del(blit);
}

//
// public
//


USceneBlur *USceneBlur_init(oobj obj, oobj parent)
{
    AScene *super = obj;
    USceneBlur *self = obj;
    o_clear(self, sizeof *self, 1);
    
    oobj view = AView_new(parent, blur_setup, blur_update, blur_render);

    AScene_init_ex(obj, parent, a_stage(), view, true, AScene_UNSAFE);
    OObj_id_set(self, USceneBlur_ID);

    // to access the USceneBlur from the view
    o_user_set(view, self);

    AScene_opaque_set(self, false);
    
    self->handle_events = true;
    
    self->animator = UAnimator_new_smoothstep_vec2(self, vec2_(0), vec2_(2.5), 0.5, &self->sigma, UAnimator_AUTO_PAUSE);
    
    return self;
}

