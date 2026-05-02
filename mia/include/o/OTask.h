#ifndef O_OTASK_H
#define O_OTASK_H

/**
 * @file OTask.h
 *
 * Object
 *
 * A Task tries to simplify and abstract long running tasks.
 * Instead of a calling a function working on data once, it calls it with multiple runs.
 * In that way, the main thread does not block and other stuff can animate, like a progress bar, etc.
 * Examples for a Task are image manipulation functions.
 * 
 * Why not use threads?
 * Threads seem to be tricky when using emscripten. Its possible, but the server needs to be setup right.
 * So with tasks we can bypass that problem.
 * 
 * Each frame the task can be run and loops over the task function, until a specific computation time limit is reached.
 *
 * You need to hold some data?
 * Have a look at o_user_ref to set and get for example an allocated (on OTask) struct of data
 * 
 * Beware: Created from the mia library often use o_user internally
 *         (in contrast to other library oobjs)
 */


#include "OObj.h"

/**
 * Task execution function.
 * Init the operation on run==0.
 *    Allocate resources via the task object, see OTask_at
 * Create / set the result to OTask_result_set.
 * A task can be aborted! 
 *      deletor of the task should kill all resources allocated in the function (default when using the oobj library)
 * @param task OTask object which operate with this function
 * @param run a counter how many times this function was called before
 * @return The progress in % [0.0-1.0), -1.0 to indicate progress not computable, >=1.0 to indicate a finish
 */
typedef float (*OTask__fn)(oobj task, int run);


/** object id */
#define OTask_ID OObj_ID "OTask."

/** when a task is finished */
#define OTask_FINISH 1.0f

/** default timeout for OTask_run millis */
#define OTask_TIMEOUT_MS_DEFAULT (5.0f)


typedef struct {
    OObj super;

    OTask__fn fn;
    bool threadable;

    int run_counter;
    float progress;
    
    // timeout in millis, fn is called in a loop until the computation_time >= timeout
    double timeout_ms;

    // set by fn, if not, it's set to this OTask instance (self)
    oobj result;

    oobj opt_threadpool;
} OTask;


/**
 * Initializes the object
 * @param obj OTask object
 * @param parent to inherit from
 * @param fn the task execution function
 * @param threadable true if this task is allowed to run in a different thread
 * @return obj casted as OTask
 */
OTask *OTask_init(oobj obj, oobj parent, OTask__fn fn, bool threadable);

/**
 * Creates a new OTask object
 * @param parent to inherit from
 * @param fn the task execution function
 * @param threadable true if this task is allowed to run in a different thread
 * @return The new object
 */
o_inline OTask *OTask_new(oobj parent, OTask__fn fn, bool threadable)
{
    OObj_DECL_IMPL_NEW(OTask, parent, fn, threadable);
}

/**
 * Creates a new OTask object that has already finished.
 * Useful for functions returning an already completed (but valid) OTask
 * @param parent to inherit from
 * @retuen The new object
 */
OTask *OTask_new_finished(oobj parent);

//
// object functions:
//



/**
 * @param obj OTask object
 * @return true if this task is allowed to run in a different thread.
 * @note The OTask is never forced to run in threads
 */
OObj_DECL_GET(OTask, bool, threadable)

/**
 * @param obj OTask object
 * @return the current task progress
 *         in % [0.0-1.0), -1.0 to indicate progress not computable, >=1.0 to indicate a finish
 * @threadsafe
 */
float OTask_progress(oobj obj);

/**
 * @param obj OTask object
 * @return the current run counter (counts how many times the execution function was called)
 * @threadsafe
 */
int OTask_run_counter(oobj obj);

/**
 * @param obj OTask object
 * @return the result set by the given execution with OTask_result_set, or NULL if not set or finished yet
 * @note To check if the task has been completed, have a look at OTask_finished()
 * @threadsafe
 */
struct oobj_opt OTask_result(oobj obj);

/**
 * @param obj OTask object
 * @return the result set by the given execution with OTask_result_set, or NULL if not finished yet
 * @note if the OTask does not set the result and indicates finished, it's set to the OTask instance obj
 *       So always valid to check if(OTask_finished(task)) ...
 *       Is allocated using the task object, so may be moved.
 * @threadsafe
 */
oobj OTask_finished(oobj obj);


/**
 * @param obj OTask object
 * @param timeout in millis for OTask_run
 * @note not used when running in the OThreadpool
 */
o_inline void OTask_timeout(oobj obj, double timeout_ms)
{
    OObj_assert(obj, OTask);
    OTask *self = obj;
    self->timeout_ms = timeout_ms;
}

/**
 * Calls the internal fn a single run
 * @param obj OTask object
 * @return The progress in % [0.0-1.0), -1.0 to indicate progress not computable, >=1.0 to indicate a finish
 * @threadsafe
 */
float OTask_run_once(oobj obj);

/**
 * Calls the internal fn multiple times, measuring the time until timeout is reached
 * @param obj OTask object
 * @return The progress in % [0.0-1.0), -1.0 to indicate progress not computable, >=1.0 to indicate a finish
 * @threadsafe
 */
float OTask_run(oobj obj);

/**
 * Calls the internal fn until OTask_FINISH is returned (blocking)
 * @param obj OTask object
 * @return >= OTask_FINISH
 */
o_inline float OTask_run_blocking(oobj obj)
{
    float progress;
    do {
        progress = OTask_run_once(obj);
    } while(progress < OTask_FINISH);
    return progress;
}


/**
 * If called, run and run_once return just the progress
 * @param obj OTask object
 * @param threadpool OThreadpool to run the task in parallel
 * @return true if the task started with the threadpool (also true if already started)
 *         false if not possible to run in the thread (!OTask_threadable || ! MIA_OPTION_THREAD)
 */
bool OTask_run_in_threadpool(oobj obj, oobj threadpool);


//
// for execution functions
//

/**
 * Result to set from within the execution function
 * @param obj OTask object
 * @param result the result to set from the execution function
 * @note must have be allocated using the task object
 * @threadsafe
 */
void OTask_result_set(oobj obj, oobj result);


#endif //O_OTASK_H
