#include "mp/history.h"

#include "o/log.h"
#include "o/str.h"
#include "o/timer.h"
#include "o/OJson.h"
#include "o/OArray.h"
#include "o/OList.h"
#include "a/common.h"


#define NUM_STEPS 64


struct mp_history_module {
    const char *name;
    void *user_data;
    mp_history_fn create_fn;
    mp_history_fn apply_fn;
    mp_history_fn cmp_fn;
    mp_history_fn opt_valid_fn;
    mp_history_fn opt_load_fn;
};

struct mp_history_step_context {
    ou64 time;
    osize cached_bytes;
};

static void mp_history_apply(struct mp_history *self)
{
    osize steps_max = OList_num(self->steps);

    if (self->num <= 0) {
        return;
    }
    osize modules_num = OArray_num(self->modules);
    int ring_pos = (self->start + self->pos) %steps_max;
    oobj step = OList_at(self->steps, ring_pos);
    struct mp_history_step_context *step_C = o_user(step);
    bool valid = true;
    for (osize m=0; m<modules_num; m++) {
        struct mp_history_module *module = OArray_at(self->modules, m, struct mp_history_module);
        oobj step_module = OJson_path(step, module->name).o;
        if (!step_module) {
            continue;
        }
        osize ret = module->apply_fn(self, step_module, self->pos, module->user_data);
        if(!ret) {
            valid = false;
        }
    }
    if (!valid) {
        o_log_warn("failed to apply step, invalid");
    }
}

//
// public
//

struct mp_history *mp_history_new(oobj parent, osize max_cached_bytes, const char *opt_auto_save_json_file)
{
    oobj container = OObj_new(parent);
    struct mp_history *self = o_new0(container, *self, 1);
    self->container = container;

    self->max_cached_bytes = max_cached_bytes > 0? max_cached_bytes : MP_HISTORY_MAX_CACHED_BYTES;
    self->opt_auto_save_json_file = o_str_clone(self->container, opt_auto_save_json_file);

    self->modules = OArray_new_dyn(self->container, NULL, sizeof(struct mp_history_module), 0, 8);

    
    // will create the steps list
    mp_history_steps_clear(self);
    return self;
}

void mp_history_module_add(struct mp_history *self, const char *name, void *user_data,
        mp_history_fn create_fn,
        mp_history_fn apply_fn,
        mp_history_fn cmp_fn,
        mp_history_fn opt_valid_fn,
        mp_history_fn opt_load_fn)
{
    //assert(OJson_num(self->json) == 0 && "add is invalid after first commit");
    name = o_str_clone(self->container, name);
    struct mp_history_module module = {name, user_data, create_fn, apply_fn, cmp_fn, opt_valid_fn, opt_load_fn};
    OArray_push(self->modules, &module);
}

void mp_history_steps_clear(struct mp_history *self)
{
    // also deletes all steps, cause the list is the parent of em.
    o_del(self->steps);
    self->steps = OList_new(self->container, NULL, NUM_STEPS);
    self->num = 0;
    self->start = 0;
    self->pos = -1;
}

osize mp_history_cached_bytes(struct mp_history *self)
{
    osize steps_max = OList_num(self->steps);
    osize bytes = 0;
    for (int i=0; i<self->num; i++) {
        int ring_pos = (self->start + i) % steps_max;
        oobj step = OList_at(self->steps, ring_pos);
        struct mp_history_step_context *step_C = o_user(step);
        bytes += step_C->cached_bytes;
    }
    return bytes;
}

void mp_history_steps_cleanup(struct mp_history *self)
{
    osize modules_num = OArray_num(self->modules);

    // remove until cache does not exceeds limit
    osize bytes = mp_history_cached_bytes(self);
    while (bytes > self->max_cached_bytes) {
        assert(self->num > 0);
        oobj *step_ptr = OList_at_ref(self->steps, self->start);
        struct mp_history_step_context *step_C = o_user(*step_ptr);
        bytes -= step_C->cached_bytes;

        // remove this step
        o_del(*step_ptr);
        self->start++;
        self->pos--;
        self->num--;
    }

    // remove invalid until a step is valid again
    while (self->num > 0) {
        oobj *step_ptr = OList_at_ref(self->steps, self->start);
        bool valid = true;
        for (osize m=0; m<modules_num; m++) {
            struct mp_history_module *module = OArray_at(self->modules, m, struct mp_history_module);
            if (!module->opt_valid_fn) {
                continue;
            }
            oobj step_module = OJson_path(*step_ptr, module->name).o;
            if (!step_module) {
                continue;
            }
            osize ret = module->opt_valid_fn(self, step_module, 0, module->user_data);
            if (!ret) {
                valid = false;
                break;
            }
        }
        if (valid) {
            // found a valid step, done with removal
            break;
        }
        // remove this step
        o_del(*step_ptr);
        self->start++;
        self->pos--;
        self->num--;
    }
}

void mp_history_save(struct mp_history *self, const char *json_file)
{
    o_log_debug("save");
    osize steps_max = OList_num(self->steps);
    
    OObj_assert(self->container, OObj);

    oobj json = OJson_new_object(self->container, "mp_history");

    OJson_new_number(json, "start", self->start);
    OJson_new_number(json, "pos", self->pos);
    oobj json_steps = OJson_new_array(json, "steps");
    // set steps as external list

    oobj *steps = OList_list(self->steps);
            
    oobj list = OList_new(json, NULL, self->num);
    oobj *list_data = OList_list(list);
    
    int start_a = self->start;
    int num_a = o_min(self->num, steps_max - start_a);
    int start_b = 0;
    int num_b = self->num - num_a;
    
    if(num_a>0) {
        o_memcpy(list_data, steps+start_a, sizeof(oobj), num_a);
    }
    if(num_b>0) {
        o_memcpy(list_data+num_a, steps+start_b, sizeof(oobj), num_b);
    }
    
    OJson_list_external_set(json_steps, list);

    // each step should be packed, but the array itsself not
    OJson_packed_set_tree(json_steps, true, oi32_MAX);
    OJson_packed_set(json_steps, false);

    // write to file
    
    bool success = OJson_write_file(json, json_file);
    if(!success) {
        o_log_warn("failed to write json history file");
    }
    o_del(json);
}

void mp_history_load(struct mp_history *self, const char *opt_json_file)
{
    o_log("load");
    
    mp_history_steps_clear(self);
    osize modules_num = OArray_num(self->modules);
    osize steps_max = OList_num(self->steps);

    if (!opt_json_file) {
        assert(self->opt_auto_save_json_file);
        opt_json_file = self->opt_auto_save_json_file;
    }
    oobj json = OJson_new_read_file(self->container, "mp_history", opt_json_file).o;
    if (!json) {
        o_log_warn("failed to load json history file");
        return;
    }
    
    double *start_ptr = OJson_path_number(json, "start");
    double *pos_ptr = OJson_path_number(json, "pos");
    
    oobj json_steps = OJson_path(json, "steps").o;

    if (!start_ptr || !pos_ptr || !json_steps) {
        o_log_warn("failed to parse json history file, missing fields");
        goto CLEAN_UP;
    }

    int start = (int) (*start_ptr);
    int pos = (int) (*pos_ptr);
    
    osize steps_num;
    OJson **json_steps_it = OJson_list(json_steps, &steps_num);
    if (steps_num<=0 || steps_num>steps_max) {
        o_log_warn("failed to parse json history file, invalid steps num");
        goto CLEAN_UP;
    }
    if (start < 0 || pos >= steps_max) {
        o_log_warn("failed to parse json history file, start invalid");
        goto CLEAN_UP;
    }
    if (pos < 0 || pos >= steps_num) {
        o_log_warn("failed to parse json history file, pos invalid");
        goto CLEAN_UP;
    }


    self->num = steps_num;
    self->start = start;
    self->pos = pos;

    // setup each step and call optional module loader
    for (int i=0; i<steps_num; i++) {
        oobj step = json_steps_it[i];
        
        int ring_pos = (start + i) % steps_max;
        oobj *step_ptr = OList_at_ref(self->steps, ring_pos);
        *step_ptr = step;
        o_move(step, self->steps);
        
        struct mp_history_step_context *step_C = o_user_new0(step, *step_C, 1);
        
        for (osize m=0; m<modules_num; m++) {
            struct mp_history_module *module = OArray_at(self->modules, m, struct mp_history_module);
            if(!module->opt_load_fn) {
                continue;
            }
            oobj step_module = OJson_child(step, module->name).o;
            if(!step_module) {
                continue;
            }
            step_C->cached_bytes += module->opt_load_fn(self, step_module, self->pos, module->user_data);
        }
    }

    CLEAN_UP:
    o_del(json);
}

int mp_history_ring_pos(struct mp_history *self, int step_pos)
{
    return (self->start + step_pos) % OList_num(self->steps);
}

int mp_history_step_pos(struct mp_history *self, int ring_pos)
{
    return o_mod(ring_pos-self->start, OList_num(self->steps));
}

struct oobj_opt mp_history_at_ring(struct mp_history *self, int ring_pos, const char *path)
{
    osize steps_max = OList_num(self->steps);
    assert(ring_pos >= 0 && ring_pos < steps_max);
    oobj step = OList_at(self->steps, ring_pos);
    OObj_assert(step, OJson);
    if (!path) {
        return oobj_opt(step);
    }
    return OJson_path(step, path);
}

void mp_history_commit(struct mp_history *self)
{
    osize modules_num = OArray_num(self->modules);
    osize steps_max = OList_num(self->steps);

    // check if equal to current step
    if (self->num > 0) {
        int ring_pos = (self->start + self->pos) % steps_max;
        oobj step = OList_at(self->steps, ring_pos);
        bool equal = true;
        for (osize m=0; m<modules_num; m++) {
            struct mp_history_module *module = OArray_at(self->modules, m, struct mp_history_module);
            oobj step_module = OJson_path(step, module->name).o;
            if (!step_module) {
                continue;
            }
            if (!module->cmp_fn(self, step_module, self->pos, module->user_data)) {
                equal = false;
                break;
            }
        }
        if (equal) {
            o_log_debug("ignored");
            return;
        }
    }

    // create the new step
    {
        self->pos++;
        if (self->pos>=steps_max) {
            self->pos=steps_max-1;
            self->start++;
        }
        self->num = self->pos+1;

        int ring_pos = (self->start + self->pos) % steps_max;
        oobj *step_ptr = OList_at_ref(self->steps, ring_pos);
        o_del(*step_ptr);
        *step_ptr = OJson_new_object(self->steps, NULL);
        oobj step = *step_ptr;

        struct mp_history_step_context *step_C = o_user_new0(step, *step_C, 1);
        step_C->time = a_timer();

        for (osize m=0; m<modules_num; m++) {
            struct mp_history_module *module = OArray_at(self->modules, m, struct mp_history_module);
            oobj step_module = OJson_new_object(step, module->name);
            step_C->cached_bytes += module->create_fn(self, step_module, self->pos, module->user_data);
        }
    }
    

    // check and remove invalid steps
    mp_history_steps_cleanup(self);

    // save
    if(self->opt_auto_save_json_file) {
        mp_history_save(self, self->opt_auto_save_json_file);
    }

    o_log("commit");
}

void mp_history_abort(struct mp_history *self)
{
    o_log_trace("abort");
    mp_history_apply(self);
}

void mp_history_undo(struct mp_history *self, double opt_elapsed_time)
{
    if(!mp_history_undo_available(self)) {
        return;
    }
    osize steps_max = OList_num(self->steps);

    int target_pos;
    if (opt_elapsed_time <= 0) {
        target_pos = self->pos-1;
    } else {
        ou64 time = a_timer();
        target_pos = -1;
        for (int i = self->pos-1; i >= 0; i--) {
            int ring_pos = (self->start + i) % steps_max;
            oobj step = OList_at(self->steps, ring_pos);
            struct mp_history_step_context *step_C = o_user(step);
            if (!step_C) {
                // context not available, maybe cause of history loaded from json
                break;
            }
            double diff = o_timer_diff_s(step_C->time, time);
            if (diff < opt_elapsed_time) {
                target_pos = i;
            } else {
                break;
            }
        }
        if (target_pos < 0) {
            return;
        }
    }
    o_log("undo: %f", opt_elapsed_time);
    self->pos = target_pos;
    mp_history_apply(self);
    
    // save
    if(self->opt_auto_save_json_file) {
        mp_history_save(self, self->opt_auto_save_json_file);
    }
}


void mp_history_redo(struct mp_history *self)
{
    if(!mp_history_redo_available(self)) {
        return;
    }
    o_log("redo");
    self->pos++;
    mp_history_apply(self);
    
    // save
    if(self->opt_auto_save_json_file) {
        mp_history_save(self, self->opt_auto_save_json_file);
    }
}
