#include "u/pose.h"
#include "o/OTask.h"
#include "o/OObjRoot.h"
#include "m/vec/flt.h"
#include "m/utils/random.h"

static vec4 poseapply(mat4 m, vec4 v)
{
    vec4 r = mat4_mul_vec(m, v);
    r = vec4_div(r, r.w);
    return r;
}

static double pose_iter4_step_v(mat4 *pose, const vec4 *lt_rt_lb_rb, int v_idx)
{
    vec4 b_lt = vec4_(0, 0, 0, 1);
    vec4 b_rt = vec4_(1, 0, 0, 1);
    vec4 b_lb = vec4_(0, 1, 0, 1);
    vec4 b_rb = vec4_(1, 1, 0, 1);
    
    
    vec4 box[] = {b_lt, b_rt, b_lb, b_rb};
    
    double error = 0;
    for(int i=0; i<4; i++) {
        vec4 res = poseapply(*pose, box[i]);
        error += vec3_sqr_distance(res.xyz, lt_rt_lb_rb[i].xyz);
    }
    
    // random change
    mat4 pose_new = *pose;
    double noise = error / 1024;    
    //o_rand_seed(o_timer());
    pose_new.v[v_idx] += m_random_noise(0, noise);
    
    double new_error = 0;
    for(int i=0; i<4; i++) {
        vec4 res = poseapply(pose_new, box[i]);
        new_error += vec3_sqr_distance(res.xyz, lt_rt_lb_rb[i].xyz);
    }
    
    if(new_error < error) {
        //o_log("success: %f < %f", new_error, error);
        *pose = pose_new;
        return new_error;
    } else {
        //o_log("failed: %f >= %f!", new_error, error);
        return error;
    }
}

o_inline double pose_iter4_step(mat4 *pose, const vec4 *lt_rt_lb_rb)
{
    for(int i=0; i<14; i++) {
        pose_iter4_step_v(pose, lt_rt_lb_rb, i);
    }
    return pose_iter4_step_v(pose, lt_rt_lb_rb, 14);
}


double u_pose_new_4_fit_step(mat4 *p, vec2 lt, vec2 rt, vec2 lb, vec2 rb, int steps)
{
    if(p->m33 != 1) {
        *p = u_pose_new_3(lt, rt, lb);
        vec2 center3 = u_pose_xy(*p);
        center3 = vec2_add_v(center3, vec2_scale(u_pose_size(*p), 0.5));
        
        vec2 center4 = vec2_add_v(lt, rt);
        center4 = vec2_add_v(center4, lb);
        center4 = vec2_add_v(center4, rb);
        center4 = vec2_scale(center4, 0.25);
        
        vec2 diff = vec2_sub_v(center4, center3);
        
        u_pose_shift_xy(p, diff.x, diff.y);
    }
    
    vec4 lt_rt_lb_rb[4] = {
        vec4_(m_2(lt), 0, 1),
        vec4_(m_2(rt), 0, 1),
        vec4_(m_2(lb), 0, 1),
        vec4_(m_2(rb), 0, 1),
    };
    
    for(int i=0; i<steps-1; i++) {
        pose_iter4_step(p, lt_rt_lb_rb);
    }
    return pose_iter4_step(p, lt_rt_lb_rb);
}


struct pose_4_fit_context {
    mat4 *p;
    vec2 lt, rt, lb, rb;
    int steps;
    double prev_error;
    int max_runs;
    double min_error_rate;
};

float pose_4_fit_task(oobj task, int run)
{
    struct pose_4_fit_context *C = o_user(task);
    
    double new_error = u_pose_new_4_fit_step(C->p, C->lt, C->rt, C->lb, C->rb, C->steps);
    
    double rate = C->prev_error - new_error;
    C->prev_error = new_error;
    
    float progress = (run + 1) / C->max_runs;
    if(rate <= C->min_error_rate) {
        progress = OTask_FINISH;
    }
    return progress;
}


oobj u_pose_new_4_fit_task(oobj parent, mat4 *p, vec2 lt, vec2 rt, vec2 lb, vec2 rb,
                           int opt_steps, int opt_max_runs, double opt_min_error_rate)
{
    *p = (mat4) {0};
    
    oobj task = OTask_new(parent, pose_4_fit_task, true);
    struct pose_4_fit_context *C = o_user_new0(task, *C, 1);
    C->p = p;
    C->lt = lt;
    C->rt = rt;
    C->lb = lb;
    C->rb = rb;
    C->steps = opt_steps>0? opt_steps : 1024;
    C->prev_error = m_MAX;
    C->max_runs = opt_max_runs>0? opt_max_runs : 128;
    C->min_error_rate = opt_min_error_rate>0? opt_min_error_rate : 0.01;

    return task;
}


mat4 u_pose_new_4_fit(vec2 lt, vec2 rt, vec2 lb, vec2 rb,
                           int opt_steps, int opt_max_runs, double opt_min_error_rate)
{
    mat4 res;
    // new unregistered root
    oobj root = OObjRoot_new(NULL, o_allocator_heap_new(), false);
    oobj task = u_pose_new_4_fit_task(root, &res, lt, rt, lb, rb, opt_steps, opt_max_runs, opt_min_error_rate);
    OTask_run_blocking(task);
    o_del(root);
    return res;
}


bool u_pose_aa_intersects_line(mat4 p, vec2 a, vec2 b)
{
    vec2 wh = u_pose_size(p);
    float size = wh.v0 < wh.v1 ? wh.v0 : wh.v1;

    vec2 diff = {{b.x - a.x, b.y - a.y}};
    float dist = m_sqrt(diff.x * diff.x + diff.y * diff.y);

    int checks = m_ceil(dist / size);
    checks = checks <= 0 ? 1 : checks; // min 1

    for (int i = 0; i < checks; i++) {
        vec2 pos = {{a.x + diff.x / checks, a.y + diff.y / checks}};

        if (u_pose_aa_contains(p, pos))
            return true;
    }
    return false;
}
