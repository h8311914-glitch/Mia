#ifndef U_BTN_H
#define U_BTN_H

/**
 * @file btn.h
 *
 * Button logics.
 * Uses multiple ways to update either a box or a quad.
 * Either by a sprite position.
 * Or by fxyz
 * Or by setting an atlas
 */

#include "o/OObj.h"
#include "r/quad.h"
#include "r/box.h"
#include "a/pointer.h"
#include "atlas.h"
#include "pose.h"
#include "rect.h"

/**
 * state for a button like shader colors and sprite atlas index
 */
struct u_btn_state {
    vec2 sprite;
    vec4 fx, fy, fz;
    int atlas;
};

/**
 * @return default state (with .s as rgba color R_WHITE)
 */
o_inline struct u_btn_state u_btn_state_new(void)
{
    struct u_btn_state self = {0};
    self.fx = vec4_(1);
    return self;
}

/**
 * btn struct containing some weak referenced to the sprite
 */
struct u_btn {
    struct r_quad *opt_quad_ref;
    struct r_box *opt_box_ref;
    struct u_atlas *opt_atlas_ref;

    // 0 for up, 1 for down, >1 for custom
    int mode;

    union {
        struct u_btn_state states[2];
        struct {
            struct u_btn_state state_up, state_down;
        };
    };
};

/**
 * Creates a new btn struct
 * @param quad_ref weak reference to an r_quad (RObjQuad...)
 * @param sprite_up sprite position for mode up
 * @param sprite_down sprite position for mode down
 */
static struct u_btn u_btn_new_sprite(struct r_quad *quad_ref, vec2 sprite_up, vec2 sprite_down)
{
    struct u_btn self = {0};
    self.opt_quad_ref = quad_ref;
    self.state_up = self.state_down = u_btn_state_new();
    self.state_up.sprite = sprite_up;
    self.state_down.sprite = sprite_down;
    return self;
}

/**
 * Creates a new btn struct
 * @param quad_ref weak reference to an r_quad (RObjQuad...)
 * @param opt_atlas_ref weak reference to a pre created atlas
 * @param atlas_up atlas index for mode up
 * @param atlas_down atlas index for mode down
 */
static struct u_btn u_btn_new_atlas(struct r_quad *quad_ref, struct u_atlas *opt_atlas_ref, int atlas_up, int atlas_down)
{
    struct u_btn self = {0};
    self.opt_quad_ref = quad_ref;
    self.opt_atlas_ref = opt_atlas_ref;
    self.state_up = self.state_down = u_btn_state_new();
    self.state_up.atlas = atlas_up;
    self.state_down.atlas = atlas_down;
    if(self.opt_quad_ref && self.opt_atlas_ref) {
        self.opt_quad_ref->uv_pose = u_atlas_pose(*self.opt_atlas_ref, self.state_up.atlas);
        u_pose_size_set(&self.opt_quad_ref->pose, m_2(u_atlas_size(*self.opt_atlas_ref, 0)));
    }
    return self;
}

/**
 * Shortcut to create a button quad which is simply colored with up and down
 */
static struct u_btn u_btn_new_color(oobj parent, struct r_quad *quad_ref, vec4 unpressed, vec4 pressed) {
    struct u_btn self = u_btn_new_sprite(quad_ref, vec2_(0), vec2_(0));
    self.state_up.fx = unpressed;
    self.state_down.fx = pressed;
    return self;
}

/**
 * Creates a new btn struct
 * @param box_ref weak reference to an r_box (RObjQuad...)
 * @param sprite_up sprite position for mode up
 * @param sprite_down sprite position for mode down
 */
static struct u_btn u_btn_new_box_sprite(struct r_box *box_ref, vec2 sprite_up, vec2 sprite_down)
{
    struct u_btn self = {0};
    self.opt_box_ref = box_ref;
    self.state_up = self.state_down = u_btn_state_new();
    self.state_up.sprite = sprite_up;
    self.state_down.sprite = sprite_down;
    return self;
}

/**
 * Creates a new btn struct from a box reference
 * @param box_ref weak reference to an r_box (RObjQuad...)
 * @param opt_atlas_ref weak reference to a pre created atlas
 * @param atlas_up atlas index for mode up
 * @param atlas_down atlas index for mode down
 */
static struct u_btn u_btn_new_box_atlas(struct r_box *box_ref, struct u_atlas *opt_atlas_ref, int atlas_up, int atlas_down)
{
    struct u_btn self = {0};
    self.opt_box_ref = box_ref;
    self.opt_atlas_ref = opt_atlas_ref;
    self.state_up = self.state_down = u_btn_state_new();
    self.state_up.atlas = atlas_up;
    self.state_down.atlas = atlas_down;
    if(self.opt_box_ref && self.opt_atlas_ref) {
        self.opt_box_ref->uv_rect = u_atlas_rect(*self.opt_atlas_ref, self.state_up.atlas);
        self.opt_box_ref->rect.zw = u_atlas_size(*self.opt_atlas_ref, 0);
    }
    return self;
}


/**
 * Shortcut to create a button box which is simply colored with up and down
 */
static struct u_btn u_btn_new_box_color(oobj parent, struct r_box *box_ref, vec4 unpressed, vec4 pressed) {
    struct u_btn self = u_btn_new_box_sprite(box_ref, vec2_(0), vec2_(0));
    self.state_up.fx = unpressed;
    self.state_down.fx = pressed;
    return self;
}


//
// struct functions
//

/**
 * @param self struct u_btn reference
 * @return true if down (pressed down); else up or custom
 */
o_inline bool u_btn_down(const struct u_btn *self) {
    return self->mode == 1;
}

/**
 * @param self struct u_btn reference
 * @param down sets mode to 1 (pressed down) if true, else 0
 */
o_inline void u_btn_down_set(struct u_btn *self, bool down) {
    self->mode = down? 1 : 0;
    if(self->opt_quad_ref) {
        self->opt_quad_ref->sprite.xy = self->states[self->mode].sprite;
        self->opt_quad_ref->fx = self->states[self->mode].fx;
        self->opt_quad_ref->fy = self->states[self->mode].fy;
        self->opt_quad_ref->fz = self->states[self->mode].fz;
        if(self->opt_atlas_ref) {
            self->opt_quad_ref->uv_pose = u_atlas_pose(*self->opt_atlas_ref, self->states[self->mode].atlas);
        }
    }
    if(self->opt_box_ref) {
        self->opt_box_ref->sprite.xy = self->states[self->mode].sprite;
        self->opt_box_ref->fx = self->states[self->mode].fx;
        if(self->opt_atlas_ref) {
            self->opt_box_ref->uv_rect = u_atlas_rect(*self->opt_atlas_ref, self->states[self->mode].atlas);
        }
    }
}

/**
 * @param self struct u_btn reference
 * @param pos (x, y, 0, 1)
 * @return true if the button contains the pos
 */
o_inline bool u_btn_contains(struct u_btn *self, vec4 pos)
{
    bool contains = false;
    if(self->opt_quad_ref) {
        contains = u_pose_contains(self->opt_quad_ref->pose, pos);
    }
    if(self->opt_box_ref) {
        contains = u_rect_contains(self->opt_box_ref->rect, pos.xy);
    }
    return contains;
}

/**
 * Checks if the button got clicked (down and released up again) and will update the sprites
 * @param self struct u_btn reference
 * @param pointer, typically a_pointer()
 * @param returns true if button got pressed down
 */
o_inline bool u_btn_clicked(struct u_btn *self, struct a_pointer pointer)
{
    bool contains = pointer.down && u_btn_contains(self, pointer.pos);
    bool press = a_pointer_pressed(pointer) && contains;
    bool clicked = a_pointer_released(pointer) && u_btn_down(self); // was pressed before

    if(press) {
        u_btn_down_set(self, true);
    }
    if(!contains || clicked) {
        u_btn_down_set(self, false);
    }
    return clicked;
}

/**
 * Checks if the button got pressed down and will update the sprites
 * @param self struct u_btn reference
 * @param pointer, typically a_pointer()
 * @param returns true if button got pressed down
 */
o_inline bool u_btn_pressed(struct u_btn *self, struct a_pointer pointer)
{
    bool contains = u_btn_contains(self, pointer.pos);
    bool press = a_pointer_pressed(pointer) && contains;
    if(press) {
        u_btn_down_set(self, true);
    }
    return press;
}

/**
 * Checks if the button got toggled and will update the sprites
 * @param self struct u_btn reference
 * @param pointer, typically a_pointer()
 * @param returns true if button got toggled(switched pressed down; released up mode)
 */
o_inline bool u_btn_toggled(struct u_btn *self, struct a_pointer pointer)
{
    bool contains = u_btn_contains(self, pointer.pos);
    bool toggle = a_pointer_pressed(pointer) && contains;
    if(toggle) {
        u_btn_down_set(self, !u_btn_down(self));
    }
    return toggle;
}


#endif //U_BTN_H
