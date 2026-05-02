#include "w/WSelect.h"
#include "o/OObj_builder.h"
#include "w/WTheme.h"
#include "w/WText.h"
#include "w/WPane.h"
#include "w/WBox.h"
#include "w/WBtn.h"
#include "m/vec/vec2.h"
#include "u/rect.h"

#undef O_LOG_LIB
#define O_LOG_LIB "w"
#include "o/log.h"


// protected
void WSelect__text_creator(oobj select, oobj btn, int idx)
{
    char **btn_text_list = o_user(select);
    char *btn_text = btn_text_list[idx];
    oobj text = WText_new(btn, btn_text);
    //WObj_padding_set(text, vec4_(1));
}

//
// public
//

WSelect *WSelect_init(oobj obj, oobj parent, WSelect__creator_fn creator_fn, int num, void *user_data)
{
    WObj *super = obj;
    WSelect *self = obj;
    o_clear(self, sizeof *self, 1);
    
    assert(num >= 1);

    WObj_init(obj, parent);
    OObj_id_set(self, WSelect_ID);
    
    o_user_set(self, user_data);

    self->pane = WPane_new(self);
    self->box = WBox_new_v(self->pane);
    self->btns = o_new(self, oobj, num);
    self->btns_num = num;
    
    self->header = WObj_new(self->box);
    
    for(int i=0; i<num; i++) {
        self->btns[i] = WBtn_new(self->box);
        WBtn_slideable_set(self->btns[i], true);
        creator_fn(self, self->btns[i], i);
    }
    
    self->footer = WObj_new(self->box);
    
    self->clicked = -1;
    
    // vfuncs
    super->v_update = WSelect__v_update;

    return self;
}


WSelect *WSelect_new_text(oobj parent, char **btn_text_list, int opt_list_num)
{
    int num = opt_list_num>0? opt_list_num : o_list_num(btn_text_list);
    oobj select = WSelect_new(parent, WSelect__text_creator, num, btn_text_list);
    // clear o_user used to create the text btns
    o_user_set(select, NULL);
    return select;
}


//
// vfuncs
//

vec2 WSelect__v_update(oobj obj, vec2 lt, vec2 min_size, bool enabled, oobj theme, a_pointer__fn pointer_fn)
{
    OObj_assert(obj, WSelect);
    WSelect *self = obj;

    vec2 size = WObj__v_update(self, lt, min_size, enabled, theme, pointer_fn);
    
    for(int i=0; i<self->btns_num; i++) {
        if(WBtn_clicked(self->btns[i])) {
            o_log_debug("clicked btn %i", i);
            self->clicked = i;
        }
    }

    struct a_pointer p = pointer_fn(0, 0);
    vec4 rect = u_rect_new(m_2(lt), m_2(size));
    if (u_rect_contains(rect, p.pos.xy)) {
        a_pointer_handled(0, 0);
    }
    
    return size;
}
