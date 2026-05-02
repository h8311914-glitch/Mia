#include "u/zoom.h"
#include "m/vec/vec2.h"
#include "r/RCam.h"
#include "a/input.h"
#include "a/pointer.h"
#include "u/pose.h"

#undef O_LOG_LIB
#define O_LOG_LIB "u"
#include "o/log.h"

struct u_zoom u_zoom_new(oobj cam)
{
    struct u_zoom self = {0};
    self.cam = cam;
    self.move_timeout = 1.5f;
    self.wheel_zoom = 1.1f;
    RCam_pixelperfect_view_set(self.cam, false);
    return self;
}


bool u_zoom_update(struct u_zoom *self, float dt)
{
    vec2 cam_pos = RCam_pos(self->cam);
    float cam_zoom = RCam_zoom(self->cam);
    vec2 zoom_pos = vec2_(0);
    
    struct a_pointer p0 = a_pointer(0, 0);
    struct a_pointer p1 = a_pointer(1, 0);
    struct a_pointer p0_p = a_pointer_p(0, 0);
    struct a_pointer p1_p = a_pointer_p(1, 0);


    if(a_pointer_touch_used()) {

        self->move_time-=dt;
        if(!p0.down && !p1.down) {
            self->move_time = self->move_timeout;
        }

        if((self->move_active || self->move_time>0) && p0.down && p1.down) {
            vec2 pos = vec2_div(vec2_add_v(p0_p.pos.xy, p1_p.pos.xy), 2.0f);
            float dist = vec2_distance(p0_p.pos.xy, p1_p.pos.xy);
            if(!self->move_active) {
                self->move_active = true;
            } else {
                vec2 pos_diff = vec2_sub_v(pos, self->move_start_pos);
                pos_diff = vec2_scale(pos_diff, cam_zoom);
                cam_pos = vec2_sub_v(cam_pos, pos_diff);

                float zoom_factor = dist / self->move_start_dist;
                zoom_factor = o_clamp(zoom_factor, 1.0f/3.0f, 3.0f);
                cam_zoom /= zoom_factor;
                
                // zooming between both pointer
                zoom_pos = vec2_mix(p0.pos.xy, p1.pos.xy, 0.5);
            }
            self->move_start_pos = pos;
            self->move_start_dist = dist;
        } else {
            self->move_active = false;
        }
    } else {

        // mouse wheel button
        if(p1.down) {
            vec2 pos = p1_p.pos.xy;
            if(!self->move_active) {
                self->move_active = true;
            } else {
                vec2 pos_diff = vec2_sub_v(pos, self->move_start_pos);
                pos_diff = vec2_scale(pos_diff, cam_zoom);
                cam_pos = vec2_sub_v(cam_pos, pos_diff);
            }
            self->move_start_pos = pos;
        } else {
            self->move_active = false;
        }

        struct a_pointer hover = a_pointer_p(0, 0);
        if(u_pose_aa_contains(RCam_proj_pose(self->cam, 0, 0), hover.pos.xy)) {
            // mouse wheel zoom
            float wheel_y = a_input_analog(A_INPUT_ANALOG_WHEEL_Y);

            // add to the integral, but if dir changed, jump back to 0
            if (wheel_y>0) {
                self->wheel_integral = m_max(self->wheel_integral, 0) + wheel_y;
            } else if (wheel_y<0) {
                self->wheel_integral = m_min(self->wheel_integral, 0) + wheel_y;
            }
            if(self->wheel_integral>1.0f) {
                self->wheel_integral-=1.0f;
                cam_zoom /= self->wheel_zoom;
            } else if(self->wheel_integral<-1.0f) {
                self->wheel_integral+=1.0f;
                cam_zoom *= self->wheel_zoom;
            }
            
            // zooming at hover pos
            zoom_pos = p0.pos.xy;
        }

    }
    
    // zooms at a given unit position and sets cam pos
    RCam_zoom_at(self->cam, zoom_pos, cam_zoom, &cam_pos);
    
    if(self->move_active) {
        self->move = true;
    } else if(!p0.down && !p1.down
            && !p0.prev_down && !p1.prev_down) {
        // false if both ptrs are up (also prev frame to avoid release=true)
        self->move = false;
    }
    
    return self->move;
}
