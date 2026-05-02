#ifndef U_FRAMES_H
#define U_FRAMES_H

/**
 * @file frames.h
 *
 * A lightweight frame / sprite animator.
 * Uses a_timer to get the frame time
 * @sa "u/UAnimator.h"
 */

#include "m/vec/ivecn.h"
#include "o/timer.h"
#include "a/common.h"

#define U_FRAMES_MAX 32

struct u_frames {
    ou64 timer;
    int frames;
    int frame_time_ms[U_FRAMES_MAX];

    // offset to the timer
    // used by u_frames_frame_set to offset to the desired frame
    oi32 time_offset_ms;

    // stops at this time
    oi32 time_stop_ms;
};

/**
 * Creates a simple frame animator, timer-based
 * @param frames Number of frames [0:frames)
 * @param frame_time_ms Each frame gets this time duration
 */
static struct u_frames u_frames_new_ms(int frames, int frame_time_ms)
{
    struct u_frames self = {0};
    self.timer = a_timer();
    self.frames = frames;
    ivecn_set(self.frame_time_ms, frame_time_ms, U_FRAMES_MAX);
    self.time_stop_ms = oi32_MAX;
    return self;
}

/**
 * Creates a simple frame animator, timer-based
 * @param frames Number of frames [0:frames)
 * @param frames_per_second fps for each frame
 */
o_inline struct u_frames u_frames_new_fps(int frames, float frames_per_second)
{
    return u_frames_new_ms(frames, (int)(1000.0f / frames_per_second));
}

/**
 * Creates a simple frame animator, timer-based
 * @param frames Number of frames [0:frames)
 * @param array of ms durations to cpy from
 */
static struct u_frames u_frames_new_array(int frames, const int *frame_time_ms_array)
{
    struct u_frames self = {0};
    self.timer = a_timer();
    self.frames = frames;
    ivecn_clone(self.frame_time_ms, frame_time_ms_array, frames);
    self.time_stop_ms = oi32_MAX;
    return self;
}

//
// struct functions
//

/**
 * @return full cycle time (loop time); sum of all frame times
 */
o_inline int u_frames_cycle_time_ms(const struct u_frames *self)
{
    return ivecn_sum(self->frame_time_ms, self->frames);
}

/**
 * @param frame Start frame to set, internally sets time_offset_ms and timer
 */
static void u_frames_frame_set(struct u_frames *self, int frame)
{
    self->timer = a_timer();
    frame = o_mod(frame, self->frames);
    self->time_offset_ms = ivecn_sum(self->frame_time_ms, frame);
}

/**
 * @param frame Stop frame (including)
 * @param additional_full_cycles full cycles until animtion will stop while reaching frame
 */
static void u_frames_stop_frame_set(struct u_frames *self, int frame, int additional_full_cycles)
{
    self->time_stop_ms = additional_full_cycles * u_frames_cycle_time_ms(self);
    self->time_stop_ms += ivecn_sum(self->frame_time_ms, frame);
}

/**
 * @return Current frame according to app time
 */
static int u_frames_get(const struct u_frames *self)
{
    ou64 time =  o_timer_elapsed_millis_int(self->timer);
    time += self->time_offset_ms;
    time = o_min(time, self->time_stop_ms);
    time %= u_frames_cycle_time_ms(self);
    int frame = 0;
    while (time >= self->frame_time_ms[frame]) {
        time -= self->frame_time_ms[frame];
        frame++;
        assert(frame < self->frames);
    }
    return frame;
}


#endif //U_FRAMES_H
