#ifndef U_UANIMATOR_H
#define U_UANIMATOR_H

/**
 * @file UAnimator.h
 *
 * Object
 */


#include "o/OArray.h"
#include "o/OList.h"
#include "m/types/flt.h"

/** object id */
#define UAnimator_ID OObj_ID "UAnimator."


struct UAnimator_target {
    void *target;
    int sampler_index;
    oobj opt_ptr;
};

enum UAnimator_mode {
    UAnimator_RUN,
    UAnimator_AUTO_PAUSE,
    UAnimator_LOOP,
    UAnimator_PINGPONG,
    UAnimator_ENUM_MAX
};

typedef struct {
    OObj super;

    float time;

    float time_start;
    float time_end;
    float speed;
    enum UAnimator_mode mode;
    bool pause;
    bool recursive;
    bool recursive_sync;

    // OList of USampler
    oobj samplers;

    // OArray of UAnimator_target
    oobj targets;
} UAnimator;


/**
 * Initializes the object.
 * @param obj UAnimator object
 * @param parent to inherit from
 * @param mode timing mode
 * @return obj casted as UAnimator
 */
UAnimator *UAnimator_init(oobj obj, oobj parent, enum UAnimator_mode mode);

/**
 * Creates a new UAnimator object
 * @param parent to inherit from
 * @param mode timing mode
 * @return The new object
 */
o_inline UAnimator *UAnimator_new(oobj parent, enum UAnimator_mode mode)
{
    OObj_DECL_IMPL_NEW(UAnimator, parent, mode);
}

/**
 * Creates a new UAnimator object.
 * Internally uses a USamplerVec with 2 frames in SPLINE mode with tangents == 0
 * @param parent to inherit from
 * @param a value start at time = 0
 * @param a_tangent out_tangent (d/dt) at start
 * @param b value end at time duration
 * @param b_tangent out_tangent (d/dt) at end
 * @param duration time for b
 * @param target float to set
 * @param mode timing mode
 * @return The new object
 */
UAnimator *UAnimator_new_ease(oobj parent, float a, float a_tangent, float b, float b_tangent, float duration,
                              float *target, enum UAnimator_mode mode);


/**
 * Creates a new UAnimator object.
 * Internally uses a USamplerVec with 2 frames in SPLINE mode with tangents == 0
 * @param parent to inherit from
 * @param a value start at time = 0
 * @param a_tangent out_tangent (d/dt) at start
 * @param b value end at time duration
 * @param b_tangent out_tangent (d/dt) at end
 * @param duration time for b
 * @param target float to set
 * @param mode timing mode
 * @return The new object
 */
UAnimator *UAnimator_new_ease_vec2(oobj parent, vec2 a, vec2 a_tangent, vec2 b, vec2 b_tangent, float duration,
                                   vec2 *target, enum UAnimator_mode mode);

/**
 * Creates a new UAnimator object.
 * Internally uses a USamplerVec with 2 frames in SPLINE mode with tangents == 0
 * @param parent to inherit from
 * @param a value start at time = 0
 * @param a_tangent out_tangent (d/dt) at start
 * @param b value end at time duration
 * @param b_tangent out_tangent (d/dt) at end
 * @param duration time for b
 * @param target float to set
 * @param mode timing mode
 * @return The new object
 */
UAnimator *UAnimator_new_ease_vec3(oobj parent, vec3 a, vec3 a_tangent, vec3 b, vec3 b_tangent, float duration,
                                   vec3 *target, enum UAnimator_mode mode);

/**
 * Creates a new UAnimator object.
 * Internally uses a USamplerVec with 2 frames in SPLINE mode with tangents == 0
 * @param parent to inherit from
 * @param a value start at time = 0
 * @param a_tangent out_tangent (d/dt) at start
 * @param b value end at time duration
 * @param b_tangent out_tangent (d/dt) at end
 * @param duration time for b
 * @param target float to set
 * @param mode timing mode
 * @return The new object
 */
UAnimator *UAnimator_new_ease_vec4(oobj parent, vec4 a, vec4 a_tangent, vec4 b, vec4 b_tangent, float duration,
                                   vec4 *target, enum UAnimator_mode mode);


/**
 * Creates a new UAnimator object.
 * Internally uses a USamplerVec with 2 frames in SPLINE mode with tangents == 0
 * @param parent to inherit from
 * @param a value start at time = 0
 * @param b value end at time duration
 * @param duration time for b
 * @param target float to set
 * @param mode timing mode
 * @return The new object
 */
o_inline UAnimator *UAnimator_new_smoothstep(oobj parent, float a, float b, float duration, float *target,
                                             enum UAnimator_mode mode)
{
    return UAnimator_new_ease(parent, a, 0, b, 0, duration, target, mode);
}

/**
 * Creates a new UAnimator object.
 * Internally uses a USamplerVec with 2 frames in SPLINE mode with tangents == 0
 * @param parent to inherit from
 * @param a value start at time = 0
 * @param b value end at time duration
 * @param duration time for b
 * @param target float to set
 * @param mode timing mode
 * @return The new object
 */
o_inline UAnimator *UAnimator_new_smoothstep_vec2(oobj parent, vec2 a, vec2 b, float duration, vec2 *target,
                                                  enum UAnimator_mode mode)
{
    return UAnimator_new_ease_vec2(parent, a, vec2_(0), b, vec2_(0), duration, target, mode);
}

/**
 * Creates a new UAnimator object.
 * Internally uses a USamplerVec with 2 frames in SPLINE mode with tangents == 0
 * @param parent to inherit from
 * @param a value start at time = 0
 * @param b value end at time duration
 * @param duration time for b
 * @param target float to set
 * @param mode timing mode
 * @return The new object
 */
o_inline UAnimator *UAnimator_new_smoothstep_vec3(oobj parent, vec3 a, vec3 b, float duration, vec3 *target,
                                                  enum UAnimator_mode mode)
{
    return UAnimator_new_ease_vec3(parent, a, vec3_(0), b, vec3_(0), duration, target, mode);
}


/**
 * Creates a new UAnimator object.
 * Internally uses a  USamplerVec with 2 frames in SPLINE mode with tangents == 0
 * @param parent to inherit from
 * @param a value start at time = 0
 * @param b value end at time duration
 * @param duration time for b
 * @param target float to set
 * @param mode timing mode
 * @return The new object
 */
o_inline UAnimator *UAnimator_new_smoothstep_vec4(oobj parent, vec4 a, vec4 b, float duration, vec4 *target,
                                                  enum UAnimator_mode mode)
{
    return UAnimator_new_ease_vec4(parent, a, vec4_(0), b, vec4_(0), duration, target, mode);
}

//
// object functions:
//

/**
 * @param obj UAnimator object
 * @return start time for looping or ping pong
 * @note special case if <0 (init) which uses the start time of the sampler
 */
OObj_DECL_GETSET(UAnimator, float, time_start)

/**
 * @param obj UAnimator object
 * @return end time for looping or ping pong
 * @note special case if <0 (init) which uses the end time of the sampler
 */
OObj_DECL_GETSET(UAnimator, float, time_end)

/**
 * @param obj UAnimator object
 * @return speed that is applied to the dt on UAnimator_run, defaults to 1.0 (100%) (may be negative for backwards)
 */
OObj_DECL_GETSET(UAnimator, float, speed)

/**
 * @param obj UAnimator object
 * @return timing mode. 
 *         AUTO_PAUSE sets pause true after passing edges
 *         PINGPONG sets the speed to (-speed) at the edges (default is SINGLE)
 */
OObj_DECL_GETSET(UAnimator, enum UAnimator_mode, mode)


/**
 * @param obj UAnimator object
 * @return if true, run is a noop (default is false)
 */
OObj_DECL_GETSET(UAnimator, bool, pause)

/**
 * @param obj UAnimator object
 * @return if true, it calls UAnimator_run in recursion using UAnimator_list (default is false)
 */
OObj_DECL_GETSET(UAnimator, bool, recursive)

/**
 * @param obj UAnimator object
 * @return if recursive is active, this copies the time of this UAnimator to the children
 *         and passes dt=0 on run to em (default is false)
 */
OObj_DECL_GETSET(UAnimator, bool, recursive_sync)


/**
 * @param obj UAnimator object
 * @return OList of USampler object
 */
OObj_DECL_GET(UAnimator, oobj, samplers)

/**
 * @param obj UAnimator object
 * @return OArray of UAnimator_target
 */
OObj_DECL_GET(UAnimator, oobj, targets)

/**
 * Get the children of a UAnimator object
 * @param obj UAnimator object
 * @param opt_out_num if not NULL, returns the list size/num
 * @return A null terminated list of the UAnimator children, call o_free(obj, list) to free it manually
 * @note the list is allocated on obj.
 *       Create an UAnimator **it=list to traverse through the list
 */
o_inline UAnimator **UAnimator_list(oobj obj, osize *opt_out_num)
{
    return (UAnimator **) OObj_list(obj, opt_out_num, UAnimator);
}

/**
 *
 * @param obj UAnimator object
 * @param sampler USampler object
 * @param move_sampler true to o_move sampler into obj
 */
void UAnimator_samplers_push(oobj obj, oobj sampler, bool move_sampler);

/**
 *
 * @param obj UAnimator object
 * @param target to be set by the sampler
 * @param opt_parent OObj object that is checked for availability if not NULL (targets parent) (creates an OPtr from it)
 */
void UAnimator_targets_push(oobj obj, void *target, int sampler_index, oobj opt_parent);

/**
 * Runs the sampler and applies it to the targets.
 * If targets are empty, sampler is ok to be NULL.
 * If UAnimator_recursive == true, UAnimator_list is used to call run in recursion (applied speed) (default is false)
 * @param obj UAnimator object
 * @param dt delta time
 */
void UAnimator_run(oobj obj, float dt);


/**
 * Calls UAnimator_time_set.
 * If UAnimator_recursive == true, UAnimator_list is used to call reset in recursion (default is false)
 * @param obj UAnimator object
 * @param time current time
 */
void UAnimator_reset(oobj obj, float time);


#endif //U_UANIMATOR_H
