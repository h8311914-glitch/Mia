#ifdef MIA_OPTION_THREAD

#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_mutex.h>
#  define SDL_CreateCondition SDL_CreateCond
#  define SDL_DestroyCondition SDL_DestroyCond
#  define SDL_BroadcastCondition SDL_CondBroadcast
#  define SDL_SignalCondition SDL_CondSignal
#  define SDL_WaitCondition SDL_CondWait
#else
#  include <SDL3/SDL_mutex.h>
#endif

#include "o/OCondition.h"
#include "o/OObj_builder.h"



OCondition *OCondition_init(oobj obj, oobj parent)
{
    OCondition *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, OCondition_ID);

    self->sdl_cond = SDL_CreateCondition();
    o_assume(self->sdl_cond, "SDL_CreateCond failed");

    // mutex deletor that calls closed SDL_DestroyMutex
    self->super.v_del = OCondition__v_del;

    return self;
}


void OCondition__v_del(oobj obj)
{
    OObj_assert(obj, OCondition);
    OCondition *self = obj;
    SDL_DestroyCondition(self->sdl_cond);

    // call super del
    OObj__v_del(obj);
}


void OCondition_broadcast(oobj obj)
{
    OObj_assert(obj, OCondition);
    OCondition *self = obj;
    SDL_BroadcastCondition(self->sdl_cond);
}

void OCondition_signal(oobj obj)
{
    OObj_assert(obj, OCondition);
    OCondition *self = obj;
    SDL_SignalCondition(self->sdl_cond);
}

void OCondition_wait(oobj obj, oobj mutex)
{
    OObj_assert(obj, OCondition);
    OObj_assert(mutex, OObj);
    OCondition *self = obj;
    OObj *self_mutex = mutex;
    SDL_WaitCondition(self->sdl_cond, self_mutex->thread_mutex);
}

bool OCondition_wait_timeout(oobj obj, oobj mutex, int timeout_ms)
{
    OObj_assert(obj, OCondition);
    OObj_assert(mutex, OObj);
    OCondition *self = obj;
    OObj *self_mutex = mutex;
#ifdef MIA_OPTION_SDL2
    int ret = SDL_CondWaitTimeout(self->sdl_cond, self_mutex->thread_mutex, timeout_ms);
    o_assume(ret != 0 && ret != SDL_MUTEX_TIMEDOUT, "SDL_CondWait failed");
    return ret == 0;
#else
    return SDL_WaitConditionTimeout(self->sdl_cond, self_mutex->thread_mutex, timeout_ms);
#endif
}


#endif // MIA_OPTION_THREAD
typedef int avoid_empty_translation_unit;
