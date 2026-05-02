#include "ex/EXViewOverlay.h"
#include "o/OObj_builder.h"
#include "o/str.h"
#include "o/utils.h"
#include "r/RTex.h"
#include "w/WIcon.h"
#include "w/WBtn.h"
#include "w/WTheme.h"
#include "w/WAlign.h"
#include "w/WTooltip.h"
#include "o/log.h"


//
// public
//

EXViewOverlay *EXViewOverlay_init(oobj obj, oobj parent, OObj__event_fn event, const char *opt_url)
{

    AView *super = obj;
    EXViewOverlay *self = obj;
    o_clear(self, sizeof *self, 1);

    AView_init(obj, parent, EXViewOverlay__v_setup, EXViewOverlay__v_update, EXViewOverlay__v_render);
    OObj_id_set(self, EXViewOverlay_ID);

    self->event = event;

    // OObj_new(self);
    // o_log_s(__func__, "creating WTheme");
    self->theme = WTheme_new_tiny(self);
    // o_log_s(__func__, "creating other W stuff");
    
    self->gui = WTooltip_new(self);
    
    oobj align = WAlign_new(self->gui);
    WAlign_align_set(align, WAlign_START, WAlign_START);
    self->cancel_btn = WBtn_new(align);
    WObj_tooltip_set(self->cancel_btn, "Back to example selection");
    WObj_padding_set(self->cancel_btn, vec4_(2));
    WBtn_style_set(self->cancel_btn, WBtn_BIG_ROUND);
    oobj icon = WIcon_new(self->cancel_btn, WTheme_ICON_CROSS);
    WIcon_color_set(icon, vec4_(0.8, 0.1, 0.1, 1.0));
    WObj_padding_set(icon, vec4_(4, 0, 4, 0));
    
    if(opt_url) {
        align = WAlign_new(self->gui);
        WAlign_align_set(align, WAlign_END, WAlign_START);
        self->source_btn = WBtn_new(align);
        WObj_tooltip_set(self->source_btn, "Open source code URL");
        WObj_padding_set(self->source_btn, vec4_(2));
        WBtn_style_set(self->source_btn, WBtn_BIG_ROUND);
        icon = WIcon_new(self->source_btn, WTheme_ICON_FILE_TXT);
    
        WObj_padding_set(icon, vec4_(4, 0, 4, 0));
        
        self->opt_url = o_str_clone(self, opt_url);
    }
    
    // o_log_s(__func__, "done");

    return self;

}


//
// virtual implementations
//

void EXViewOverlay__v_setup(oobj view)
{
    // noop
}

void EXViewOverlay__v_update(oobj view, oobj tex, float dt)
{
    OObj_assert(view, EXViewOverlay);
    EXViewOverlay *self = view;

    struct r_proj *proj = RTex_proj(tex);
    vec2 lt = vec2_(0);
    WTheme_update_full_tex(self->theme, self->gui, tex);
    if(WBtn_clicked(self->cancel_btn)) {
        o_log_s(__func__, "cancel button clicked");
        if(self->event) {
            self->event(self);
        }
    }
    
    if(self->source_btn && WBtn_clicked(self->source_btn)) {
        o_log_s(__func__, "source button clicked");
        o_utils_open_url(self->opt_url);
    }
}

void EXViewOverlay__v_render(oobj view, oobj tex, float dt)
{
    OObj_assert(view, EXViewOverlay);
    EXViewOverlay *self = view;
    WTheme_render(self->theme, tex);
}

//
// object functions:
//
