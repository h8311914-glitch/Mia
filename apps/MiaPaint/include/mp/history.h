#ifndef MP_HISTORY_H
#define MP_HISTORY_H

/**
 * @file history.h
 *
 * Shared context for generic history steps.
 * Uses an OJson tree for serialization.
 */

#include "o/common.h"


#define MP_HISTORY_MAX_CACHED_BYTES (256 * 1024 * 1024)
#define MP_HISTORY_AUTO_SAVE_PATH "&mp_history.json"

// forward
struct mp_history;

/**
 * Virtual function for the history api.
 * Used for:
 * - creating new history entries, return ~ byte size of additional cached memory
 * - apply a history step, return false if failed
 * - compare this step with the current data, return true on match (!=0)
 * - (optional) valid, return false if this step is not valid anymore
 * - (optional) load, return ~ byte size of additional cached memory
 * @param step_json OJson for this module and this step to allocate on (use OJson children for serialization).
 * @param step_pos current step position relative of start (so step_idx + history->start == ring_idx)
 * @param user_data passed in mp_history_module_add
 */
typedef osize (*mp_history_fn)(struct mp_history *history, oobj step_json, int step_pos, void *user_data);



struct mp_history {
    oobj container;

    osize max_cached_bytes;
    char *opt_auto_save_json_file;

    // OArray of internal structs
    oobj modules;

    // OList of json's step children as a ring buffer
    oobj steps;

    int num;
    int start;
    int pos; // relative from start
};


/**
 * Creates a new history context
 * @param parent to inherit from
 * @param max_cached_bytes if additional ~ cached bytes exceed that, it removes steps until its in limits again.
 *                         if <= 0: MP_HISTORY_MAX_CACHED_BYTES
 * @param opt_auto_save_json_file if not NULL, used auto save the history.
 *                                May be set to MP_HISTORY_AUTO_SAVE_PATH.
 *                                Also results in mp_history_auto_save() returning true.
 */
struct mp_history *mp_history_new(oobj parent, osize max_cached_bytes, const char *opt_auto_save_json_file);


/**
 * Adds a module to the history system.
 * @param name of the module to add (used as json object name)
 * @param user_data passed to the vfuncs
 * @param create_fn: creating new history entries, return ~ byte size of additional cached memory
 * @param apply_fn: apply a history step, return false if failed
 * @param cmp_fn: compare this step with the current data, return true on match (!=0)
 * @param opt_valid_fn: (optional) valid, return false if this step is not valid anymore
 * @param opt_load_fn: (optional) load, return ~ byte size of additional cached memory
 */
void mp_history_module_add(struct mp_history *self, const char *name, void *user_data,
        mp_history_fn create_fn,
        mp_history_fn apply_fn,
        mp_history_fn cmp_fn,
        mp_history_fn opt_valid_fn,
        mp_history_fn opt_load_fn);

/**
 * Removes all steps.
 */
void mp_history_steps_clear(struct mp_history *self);

/**
 * @return Rough amount of additional cached bytes
 */
osize mp_history_cached_bytes(struct mp_history *self);

/**
 * If mp_history_cached_bytes exceeds self->max_cached_bytes, it removes steps until in limit again.
 */
void mp_history_steps_cleanup(struct mp_history *self);

/**
 * Writes the history to a json file.
 * Auto called on commit if self->opt_auto_save_json_file is set
 */
void mp_history_save(struct mp_history *self, const char *json_file);

/**
 * Loads the history from a json file.
 * Should be called after adding external modules with mp_history_module_add.
 * Checks steps for being valid.
 * Applies the current history step.
 * @param opt_json_file if NULL is passed, self->opt_auto_save_json_file is used instead
 */
void mp_history_load(struct mp_history *self, const char *opt_json_file);


/**
 * @param pos relative from start
 * @return ring_pos in the steps ring OList buffer
 */
int mp_history_ring_pos(struct mp_history *self, int step_pos);

/**
 * @param ring_pos in the steps ring OList buffer
 * @return step_pos relative from start
 */
int mp_history_step_pos(struct mp_history *self, int ring_pos);


/**
 * @param ring_pos ring index asserts bounds
 * @param path module name or OJson path to receive, or NULL for all
 * @return OJson if that step, for the given module (or all), NULL if path failed
 */
struct oobj_opt mp_history_at_ring(struct mp_history *self, int ring_pos, const char *path);


/**
 * @param pos step index, relative from start, asserts bounds
 * @param path module name or OJson path to receive, or NULL for all
 * @return OJson if that step, for the given module (or all), NULL if path failed
 */
o_inline struct oobj_opt mp_history_at(struct mp_history *self, int pos, const char *path)
{
    assert(pos>=0 && pos<self->num);
    return mp_history_at_ring(self, mp_history_ring_pos(self, pos), path);
}


/**
 * @return True if auto saving is enabled.
 *         Results in modules maybe loading external cached stuff like images.
 */
o_inline bool mp_history_auto_save(struct mp_history *self)
{
    return self->opt_auto_save_json_file != NULL;
}

/**
 * @return Current ring index
 */
o_inline int mp_history_current_idx(struct mp_history *self)
{
    return self->start + self->pos;
}

//
// commit or abort changes
//

/**
 * Commits changes as the next history step.
 * Aborts if each module is equal to the current step.
 */
void mp_history_commit(struct mp_history *self);

/**
 * Aborts changes and reverts to the last history commit step
 */
void mp_history_abort(struct mp_history *self);


/**
 * Commits only if the history is empty
 */
o_inline void mp_history_initial_commit(struct mp_history *self)
{
    if(self->num <= 0) {
        mp_history_commit(self);
    } else {
        mp_history_abort(self);
    }
}


//
// history stuff
//

/**
 * @return true if undo is available
 */
o_inline bool mp_history_undo_available(const struct mp_history *self)
{
    return self->pos>0;
}

/**
 * @return true if redo is available
 */
o_inline bool mp_history_redo_available(const struct mp_history *self) 
{
    return self->pos<self->num-1;
}

/**
 * undo to the previous history step.
 * @param opt_elapsed_time if >0: undo to this elapsed time (or less)
 *                         <=0: ignore timestep
 *                         Only works for current app run, not for loaded history
 */
void mp_history_undo(struct mp_history *self, double opt_elapsed_time);

/**
 * redo to the next history step.
 */
void mp_history_redo(struct mp_history *self);




#endif //MP_HISTORY_H
