#include "u/UAnimator.h"
#include "o/OObj_builder.h"
#include "o/OPtr.h"
#include "u/USampler.h"
#include "u/USamplerVec.h"

//
// public
//


UAnimator *UAnimator_init(oobj obj, oobj parent, enum UAnimator_mode mode)
{
    UAnimator *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, UAnimator_ID);

    self->time = 0;
    self->time_start = -1;
    self->time_end = -1;
    self->speed = 1;
    self->mode = mode;
    self->pause = false;
    self->recursive = false;
    self->recursive_sync = false;

    self->samplers = OList_new(self, NULL, 0);
    self->targets = OArray_new_dyn(self, NULL, sizeof(struct UAnimator_target), 0, 8);

    return self;
}

UAnimator *UAnimator_new_ease(oobj parent, float a, float a_tangent, float b, float b_tangent, float duration,
                              float *target, enum UAnimator_mode mode)
{
    struct USamplerVec_float frames[2] = {0};
    frames[0].value = a;
    frames[0].tangent_out = a_tangent;
    frames[1].value = b;
    frames[1].tangent_in = b_tangent;
    frames[1].time = duration;
    oobj sampler = USamplerVec_new_frames(parent, 1, frames, 2, USamplerVec_SPLINE);
    oobj self = UAnimator_new(parent, mode);
    UAnimator_samplers_push(self, sampler, true);
    UAnimator_targets_push(self, target, 0, NULL);
    return self;
}


UAnimator *UAnimator_new_ease_vec2(oobj parent, vec2 a, vec2 a_tangent, vec2 b, vec2 b_tangent, float duration,
                                   vec2 *target, enum UAnimator_mode mode)
{
    struct USamplerVec_vec2 frames[2] = {0};
    frames[0].value = a;
    frames[0].tangent_out = a_tangent;
    frames[1].value = b;
    frames[1].tangent_in = b_tangent;
    frames[1].time = duration;
    oobj sampler = USamplerVec_new_frames(parent, 2, frames, 2, USamplerVec_SPLINE);
    oobj self = UAnimator_new(parent, mode);
    UAnimator_samplers_push(self, sampler, true);
    UAnimator_targets_push(self, target, 0, NULL);
    return self;
}


UAnimator *UAnimator_new_ease_vec3(oobj parent, vec3 a, vec3 a_tangent, vec3 b, vec3 b_tangent, float duration,
                                   vec3 *target, enum UAnimator_mode mode)
{
    struct USamplerVec_vec3 frames[2] = {0};
    frames[0].value = a;
    frames[0].tangent_out = a_tangent;
    frames[1].value = b;
    frames[1].tangent_in = b_tangent;
    frames[1].time = duration;
    oobj sampler = USamplerVec_new_frames(parent, 3, frames, 2, USamplerVec_SPLINE);
    oobj self = UAnimator_new(parent, mode);
    UAnimator_samplers_push(self, sampler, true);
    UAnimator_targets_push(self, target, 0, NULL);
    return self;
}


UAnimator *UAnimator_new_ease_vec4(oobj parent, vec4 a, vec4 a_tangent, vec4 b, vec4 b_tangent, float duration,
                                   vec4 *target, enum UAnimator_mode mode)
{
    struct USamplerVec_vec4 frames[2] = {0};
    frames[0].value = a;
    frames[0].tangent_out = a_tangent;
    frames[1].value = b;
    frames[1].tangent_in = b_tangent;
    frames[1].time = duration;
    oobj sampler = USamplerVec_new_frames(parent, 4, frames, 2, USamplerVec_SPLINE);
    oobj self = UAnimator_new(parent, mode);
    UAnimator_samplers_push(self, sampler, true);
    UAnimator_targets_push(self, target, 0, NULL);
    return self;
}

//
// object functions
//

void UAnimator_samplers_push(oobj obj, oobj sampler, bool move_sampler)
{
    OList_push(UAnimator_samplers(obj), sampler);
    if (move_sampler) {
        o_move(sampler, obj);
    }
}

void UAnimator_targets_push(oobj obj, void *target, int sampler_index, oobj opt_parent)
{
    oobj ptr = NULL;
    if (opt_parent) {
        ptr = OPtr_new(obj, opt_parent);
    }
    struct UAnimator_target t = {target, sampler_index, ptr};
    OArray_push(UAnimator_targets(obj), &t);
}

void UAnimator_run(oobj obj, float dt)
{
    OObj_assert(obj, UAnimator);
    UAnimator *self = obj;

    if (self->pause) {
        return;
    }
    dt *= self->speed;

    self->time += dt;

    osize num_samplers = o_num(self->samplers);
    osize num_targets = o_num(self->targets);

    if (num_samplers>0 && (self->time_start < 0 || self->time_end < 0)) {
        float start_time, end_time;
        oobj sampler = o_at(self->samplers, 0);
        USampler_times_start_end(sampler, &start_time, &end_time);
        if (self->time_start < 0) {
            self->time_start = start_time;
        }
        if (self->time_end < 0) {
            self->time_end = end_time;
        }
    }

    switch (self->mode) {
        default:
        case UAnimator_RUN:
            break;
        case UAnimator_AUTO_PAUSE:
            if (self->speed>0 && self->time >= self->time_end) {
                self->pause = true;
            } else if (self->speed<0 && self->time <= self->time_start) {
                self->pause = true;
            }
            break;
        case UAnimator_LOOP:
            if (self->speed>0 && self->time > self->time_end) {
                float duration = self->time_end - self->time_start;
                self->time -= duration;
            } else if (self->speed<0 && self->time < self->time_start) {
                float duration = self->time_end - self->time_start;
                self->time += duration;
            }
            break;
        case UAnimator_PINGPONG:
            if (self->speed>0 && self->time > self->time_end) {
                float exceed = self->time - self->time_end;
                self->time = self->time_end - exceed;
                self->speed = -self->speed;
            } else if (self->speed<0 && self->time < self->time_start) {
                float exceed = self->time_start - self->time;
                self->time = self->time_start + exceed;
                self->speed = -self->speed;
            }
    }

    if (num_targets > 0 ) {
        assert(num_samplers > 0);

        oobj container = OObj_new(self);
        void **data = o_new0(container, void*, num_samplers);

        oobj *samplers = OList_list(self->samplers);
        struct UAnimator_target *targets = OArray_data_void(self->targets);
        osize data_size = USampler_element_size(samplers[0]);
        for (osize i=0; i<num_targets; i++) {
            struct UAnimator_target *t = &targets[i];

            // check parent available if present
            if (t->opt_ptr) {
                if (!OPtr_available(t->opt_ptr)) {
                    continue;
                }
            }

            osize sampler_index = o_clamp(t->sampler_index, 0, num_samplers-1);
            if (!data[sampler_index]) {
                assert(data_size == USampler_element_size(samplers[sampler_index]));
                data[sampler_index] = o_alloc(container, data_size, 1);
                USampler_interpolate(samplers[sampler_index], data[sampler_index], self->time);
            }

            o_memcpy(t->target, data[sampler_index], data_size, 1);
        }

        // deletes data and its elements
        o_del(container);
    }

    if (self->recursive) {
        UAnimator **list = UAnimator_list(self, NULL);
        if (self->recursive_sync) {
            for (UAnimator **it = list; *it; it++) {
                UAnimator *child = *it;
                child->time = self->time;
                UAnimator_run(child, 0);
            }
        } else {
            for (UAnimator **it = list; *it; it++) {
                UAnimator_run(*it, dt);
            }
        }
        o_free(self, list);
    }
}


void UAnimator_reset(oobj obj, float time)
{
    OObj_assert(obj, UAnimator);
    UAnimator *self = obj;

    self->time = time;
    if (self->recursive) {
        UAnimator **list = UAnimator_list(self, NULL);
        for (UAnimator **it = list; *it; it++) {
            UAnimator_reset(*it, time);
        }
        o_free(self, list);
    }
}
