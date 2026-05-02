#include "x/XViewVirtual.h"
#include "o/OObj_builder.h"
#include "m/vec/ivec2.h"
#include "r/RTex_blit.h"
#include "a/common.h"
#include "u/pose.h"
#include "u/rect.h"
#include "w/WBox.h"
#include "w/WBtn.h"
#include "w/WFrame.h"
#include "w/WTheme.h"
#include "w/WText.h"
#include "w/WAlign.h"
#include "w/WColor.h"
#include "x/num.h"

#undef O_LOG_LIB
#define O_LOG_LIB "x"
#include "o/log.h"


//
// public
//

XViewVirtual *XViewVirtual_init(oobj obj, oobj parent, AView *view, bool move_view, ivec2 resolution)
{
    AView *super = obj;
    XViewVirtual *self = obj;
    o_clear(self, sizeof *self, 1);

    AView_init(obj, parent, XViewVirtual__v_setup, XViewVirtual__v_update, XViewVirtual__v_render);
    OObj_id_set(self, XViewVirtual_ID);

    self->view = view;
    if (move_view) {
        o_move(view, self);
    }
    self->resolution = resolution;
    self->resolution_set = true;


    const float resolutions[] = {180, 320, 360, 512, 640, 720, 1024, 1080, 1280, 1440, 1920, 2560};

    self->theme = WTheme_new_tiny(self);
    self->gui_root = WBox_new(self, WBox_V_WEIGHTS);
    self->gui = WBox_new_v(self->gui_root);
    WObj_padding_set(self->gui, vec4_(4));
    WBox_spacing_set(self->gui, vec2_(2));
    self->num_cols = x_num_ex(self->gui, 32, 4096, 1, "COLS", resolutions, o_stack_array_num(resolutions));
    self->num_rows = x_num_ex(self->gui, 32, 4096, 1, "ROWS", resolutions, o_stack_array_num(resolutions));
    self->num_zoom = x_num(self->gui, 0.0, 2.0, 0.01, "ZOOM");
    WNum_num_set(self->num_zoom, 1.0);
    self->btn_screenshot = WBtn_new(self->gui);
    WText_new(self->btn_screenshot, "SCREENSHOT");

    self->container_space = WObj_new(self->gui_root);
    WBox_child_weight_set(self->container_space, 1);

    oobj align = WAlign_new_center(self->container_space);
    oobj frame = WFrame_new(align);
    WFrame_border_color_set(frame, R_GRAY);
    self->container_view = WObj_new(frame);
    oobj col = WColor_new(self->container_view, R_BLACK);
    WObj_min_size_set(col, vec2_(8, 12));

    return self;
}


//
// virtual implementations
//

void XViewVirtual__v_setup(oobj view)
{
    // noop?
}

void XViewVirtual__v_update(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewVirtual);
    XViewVirtual *self = view;

    if (self->resolution_set) {
        self->resolution_set = false;
        WNum_num_set(self->num_cols, self->resolution.x);
        WNum_num_set(self->num_rows, self->resolution.y);
    }

    vec2 space = WObj_gen_size(self->container_space);
    // sub the frame and another padding
    space = vec2_sub(space, 8);
    vec2 view_size = vec2_(
        space.x,
        space.x * self->resolution.y / self->resolution.x
    );
    if (view_size.y > space.y) {
        view_size = vec2_(
            space.y * self->resolution.x / self->resolution.y,
            space.y
        );
    }
    WObj_min_size_set(self->container_view, view_size);

    WTheme_update_full_tex(self->theme, self->gui_root, tex);

    self->resolution.x = WNum_num(self->num_cols);
    self->resolution.y = WNum_num(self->num_rows);
    self->zoom = WNum_num(self->num_zoom);

    if (WBtn_clicked(self->btn_screenshot)) {
        o_log_s(__func__, "SCREENSHOT");
        RTex_write_file(AView_tex(self->view), "#screenshot.png");
    }

    if (!self->tex || !ivec2_equals_v(self->resolution, RTex_size_int(self->tex))) {
        // recreate self->tex
        o_del(self->tex);
        self->tex = RTex_new(self, NULL, m_2(self->resolution));
        *RTex_proj(self->tex) = r_proj_new(self->resolution, -1, vec2_(180), true);
    }


    vec4 rect = WObj_gen_rect(self->container_view);
    vec2 center = u_rect_center(rect);
    vec2 size = u_rect_size(rect);
    size = vec2_scale(size, self->zoom);
    rect = u_rect_new_center(m_2(center), m_2(size));

    vec4 display_rect = r_proj_rect_to_display(RTex_proj(tex), rect);
    mat4 display_pose = u_pose_new_rect(display_rect);

    // if in a view chain, use display_pose relative
    oobj super_view = a_view_try().o;
    if (super_view) {
        mat4 super_pose = AView_display_pose(super_view);
        display_pose = mat4_mul_mat(super_pose, display_pose);
    }

    AView_update_ex(self->view, self->tex, RTex_viewport(self->tex), display_pose);
}

void XViewVirtual__v_render(oobj view, oobj tex, float dt)
{
    OObj_assert(view, XViewVirtual);
    XViewVirtual *self = view;

    RTex_clear(tex, R_GRAY_X(0.2));
    WTheme_render(self->theme, tex);

    RTex_clear_full(self->tex, R_BLACK);
    AView_render(self->view, self->tex);

    vec4 rect = WObj_gen_rect(self->container_view);
    vec2 center = u_rect_center(rect);
    vec2 size = u_rect_size(rect);
    size = vec2_scale(size, self->zoom);
    rect = u_rect_new_center(m_2(center), m_2(size));
    vec4 uv = RTex_rect(self->tex, 0, 0);
    RTex_blit_rect(tex, self->tex, rect, uv);
}

//
// object functions:
//
