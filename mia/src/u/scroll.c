#include "u/scroll.h"
#include "m/vec/vec2.h"
#include "r/RCam.h"
#include "a/input.h"

#undef O_LOG_LIB
#define O_LOG_LIB "u"
#include "o/log.h"

struct u_scroll u_scroll_new(enum u_scroll_mode mode, oobj opt_cam)
{
    struct u_scroll self = {0};
    self.mode = mode;
    self.align_x = self.align_y = u_scroll_CENTER;
    self.cam = opt_cam;
    self.on_units = true;
    self.view_rect = self.cam_limits_rect = u_rect_new(0, 0, m_MAX, m_MAX);
    self.view_rect_auto = true;
    self.deacc = vec2_(1000.0f);
    self.handle_distance = 16;
    return self;
}


vec2 u_scroll_update(struct u_scroll *self, float dt)
{
    if(self->cam && self->view_rect_auto) {
        self->view_rect = RCam_proj_rect(self->cam, 0, 0);
    }
    
    struct a_pointer pointer = a_pointer_p(0, 0);
    vec2 pp = pointer.pos.xy;

    if (pointer.down && u_rect_contains(self->view_rect, pp)) {
        if (a_pointer_pressed(pointer)) {
            self->start_pointer = pp;
            self->start_pos = self->prev_pos = self->raw_pos;
            self->scrolling = true;
        }
    } else {
        self->scrolling = false;
    }

    if (self->scrolling) {
        vec2 diff = vec2_sub_v(pp, self->start_pointer);

        vec2 dist = vec2_abs(diff);

        if (((self->mode == u_scroll_X || self->mode == u_scroll_XY) && dist.x > self->handle_distance)
            || ((self->mode == u_scroll_Y || self->mode == u_scroll_XY) && dist.y > self->handle_distance)) {
            a_pointer_handled(0, 0);
        }

        self->raw_pos = vec2_sub_v(self->start_pos, diff);

        diff = vec2_sub_v(self->raw_pos, self->prev_pos);
        self->speed = vec2_mix(self->speed, vec2_div(diff, dt), 0.25);

        self->prev_pos = self->raw_pos;
    } else {
        // deacc
        vec2 dir = vec2_sign(self->speed);
        vec2 speed = vec2_add_v(vec2_abs(self->speed), vec2_scale(self->deacc, -dt));
        speed = vec2_max(speed, 0);
        self->speed = vec2_scale_v(speed, dir);

        // apply speed
        self->raw_pos = vec2_add_scaled(self->raw_pos, self->speed, dt);
    }

    // wheel scrolling
    vec2 wheel = a_input_analog_vec2(A_INPUT_ANALOG_WHEEL_X);
    self->raw_pos = vec2_add_scaled(self->raw_pos, wheel, -16);


    vec4 cam_rect = self->cam_limits_rect;
    cam_rect.zw = vec2_sub_v(cam_rect.zw, self->view_rect.zw);
    cam_rect.zw = vec2_max(cam_rect.zw, 0);

    // limits
    if (self->raw_pos.x < u_rect_left(cam_rect)) {
        self->raw_pos.x = u_rect_left(cam_rect);
        self->speed.x = 0;
    }
    if (self->raw_pos.x > u_rect_right(cam_rect)) {
        self->raw_pos.x = u_rect_right(cam_rect);
        self->speed.x = 0;
    }
    if (self->raw_pos.y > u_rect_bottom(cam_rect)) {
        self->raw_pos.y = u_rect_bottom(cam_rect);
        self->speed.y = 0;
    }
    if (self->raw_pos.y < u_rect_top(cam_rect)) {
        self->raw_pos.y = u_rect_top(cam_rect);
        self->speed.y = 0;
    }

    // finish...
    if (self->mode != u_scroll_OFF) {

        // apply alignments from raw_pos -> pos
        vec2 offset = {0};
        if (self->align_x == u_scroll_BEGIN) {
            offset.x = +o_max(0, (self->view_rect.v2 - self->cam_limits_rect.v2) / 2.0f);
        } else if (self->align_x == u_scroll_END) {
            offset.x = -o_max(0, (self->view_rect.v2 - self->cam_limits_rect.v2) / 2.0f);
        }
        if (self->align_y == u_scroll_BEGIN) {
            offset.y = +o_max(0, (self->view_rect.v3 - self->cam_limits_rect.v3) / 2.0f);
        } else if (self->align_y == u_scroll_END) {
            offset.y = -o_max(0, (self->view_rect.v3 - self->cam_limits_rect.v3) / 2.0f);
        }

        if(self->mode == u_scroll_X || self->mode == u_scroll_XY) {
            self->pos.x = self->raw_pos.x + offset.x;
        }
        if(self->mode == u_scroll_Y || self->mode == u_scroll_XY) {
            self->pos.y = self->raw_pos.y + offset.y;
        }

        if (self->on_units) {
            self->pos = vec2_round(self->pos);
        }
        if (self->cam) {
            RCam_pos_set(self->cam, self->pos, false);
        }
    }
    return self->pos;
}
