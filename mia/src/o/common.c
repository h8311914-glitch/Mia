// must include all, because SDL_init is located here...

#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL.h>
#  include <SDL2/SDL_image.h>
#  define SDL_INIT_GAMEPAD SDL_INIT_GAMECONTROLLER
#  define SDL_SetCurrentThreadPriority SDL_SetThreadPriority
#  define NET_Init SDLNet_Init

#  ifdef MIA_OPTION_TTF
#    include <SDL2/SDL_ttf.h>
#  endif
#  ifdef MIA_OPTION_SOCKET
#    include <SDL2/SDL_net.h>
#  endif

#else // SDL3
#  include <SDL3/SDL.h>
#  include <SDL3_image/SDL_image.h>

#  ifdef MIA_OPTION_TTF
#    include <SDL3_ttf/SDL_ttf.h>
#  endif
#  ifdef MIA_OPTION_SOCKET
#    include <SDL3_net/SDL_net.h>
#  endif

#endif

#include <stdlib.h>
#include <time.h>

#include "o/common.h"
#include "o/terminalcolor.h"
#include "o/OObjRoot.h"
#include "o/ODelcallback.h"
#include "o/OContext.h"

#undef O_LOG_LIB
#define O_LOG_LIB "o"
#include "o/log.h"




#include <time.h>


#ifdef MIA_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#endif



#define MT_N 624
#define MT_M 397
#define MT_MATRIX_A 0x9908B0DFU
#define MT_UPPER_MASK 0x80000000U
#define MT_LOWER_MASK 0x7FFFFFFFU



#ifdef MIA_OPTION_SDL2
_Static_assert(sizeof(ou64) >= sizeof(SDL_threadID), "invalid size?");
#else
_Static_assert(sizeof(ou64) >= sizeof(SDL_ThreadID), "invalid size?");
#endif

_Static_assert(sizeof(int) >= sizeof(oi32), "Using \"o\" in a u-controller or what?, "
                                            "should work fine..."
                                            "I used osize for most, but never tested it :D,"
                                            "just remove this _Static_assert...");

static struct {
    bool init;
    ou64 main_thread_id;
    bool ignore_pause;
    oobj context_root;
    oobj context;
} o_common_L;


// each thread has its own arena (if set)
static _Thread_local oobj o_common_L_arena_root;

static _Thread_local ou32 o_common_L_mt[MT_N];
static _Thread_local int o_common_L_mt_index = MT_N+1;

static _Thread_local o__compare_fn o_common_L_qsort_fn;
static _Thread_local void *o_common_L_qsort_user_data;
static _Thread_local o__compare_fn o_common_L_bsearch_fn;
static _Thread_local void *o_common_L_bsearch_user_data;

//
// protected
//

// protected
void o__sanitizer_leak_check(const char *why, bool full)
{
#ifdef MIA_OPTION_SANITIZER
    void __lsan_do_leak_check(void);
    int __lsan_do_recoverable_leak_check(void);

    // checks for memory leaks
    o_log_debug_s(why, "sanitizer leak check...");
    int leaks = __lsan_do_recoverable_leak_check();

    // this call also checks for leaks, but than it doesnt check at program end
    //      some SDL (or ports) may have some memory leaks
    //      so we call it before shutting them down to see our own mistakes
    //      may fail and exit the app
    if(full) {
        __lsan_do_leak_check();
    }

    if (leaks) {
        o_log_error_s(why, "sanitizer leak check done, got %i leaks!", leaks);
    } else {
        o_log_s(why, "sanitizer leak check done, without any leaks");
    }
#endif
}


static int o_qsort__compare(const void *a, const void *b)
{
    return o_common_L_qsort_fn(o_common_L_qsort_user_data, a, b);
}
static int o_bsearch__compare(const void *a, const void *b)
{
    return o_common_L_bsearch_fn(o_common_L_bsearch_user_data, a, b);
}


//
// public
//


void o_qsort(void *data, osize element_size, osize count, void *user_data, o__compare_fn fn)
{
    if (!data) {
        return;
    }
    assert(o_common_L_qsort_fn == NULL && "nested o_qsort calls are not possible");
    o_common_L_qsort_fn = fn;
    o_common_L_qsort_user_data = user_data;
    // note: may have used C11 qsort_s, but who cares...
    qsort(data, count, element_size, o_qsort__compare);
    o_common_L_qsort_fn = NULL;
    o_common_L_qsort_user_data = NULL;
}

void *o_bsearch(const void *key, const void *data, osize element_size, osize count, void *user_data, o__compare_fn fn)
{
    if (!key || !data) {
        return NULL;
    }
    assert(o_common_L_bsearch_fn == NULL && "nested o_bsearch calls are not possible");
    o_common_L_bsearch_fn = fn;
    o_common_L_bsearch_user_data = user_data;
    void *res = bsearch(key, data, count, element_size, o_bsearch__compare);
    o_common_L_bsearch_fn = NULL;
    o_common_L_bsearch_user_data = NULL;
    return res;
}

oi64 o_time(void)
{
    return (oi64) time(NULL);
}

void o_init(void)
{
    if(o_common_L.init) {
        o_log_error_s(__func__, "o_init called already!");
        return;
    }
    o_common_L.init = true;

#ifdef MIA_OPTION_SDL2
    Uint32 flags = SDL_INIT_EVERYTHING;
#else
    Uint32 flags = SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_CAMERA;
#endif

#ifdef MIA_OPTION_GAMEPAD
    flags |= SDL_INIT_GAMEPAD;
#endif

    if (SDL_Init(flags) != O_SDL_OK) {
        o_log_error_s(__func__, "SDL_Init failed: %s", SDL_GetError());
        o_exit("SDL_Init failed");
    }

#ifdef MIA_OPTION_SDL2
    // IMG only needs to be init in SDL2

    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if(!(IMG_Init(img_flags) & img_flags)) {
        o_exit("IMG_Init failed");
    }
#endif

    // SDL_Mixer should be init after first pointer event
    //           not part of the framework itself yet, up to the user

#ifdef MIA_OPTION_TTF
    if (TTF_Init() != O_SDL_OK) {
        o_exit("TTF_Init failed");
    }
#endif

#ifdef MIA_OPTION_SOCKET
    if (NET_Init() != O_SDL_OK) {
        o_exit("SDLNet_Init failed");
    }
#endif


    o_common_L.main_thread_id = o_thread_id();

    // seed random
    o_rand_seed(time(NULL));

    // will allow color codes for logs and terminal stuff on windows
    o_terminalcolor_start();

    // context
    o_common_L.context_root = OObjRoot_new_heap("[mia]o_context_root");
    o_common_L.context = OContext_new(o_common_L.context_root, NULL);
    ODelcallback_new_assert(o_common_L.context, "[mia]o_context", "deleted!");

#ifdef MIA_OPTION_FETCH
    // protected  init OFetch
    void o__fetch_init(void);
    o__fetch_init();
#endif

    // protected init file system
    // warning, MIA_PLATFORM_EMSCRIPTEN is a special case in which the file system mounting is async.
    //          so we need to wait for it to finish before continuing.
    //          this is done in a/app.c cause on that platform its (for now?) not possible to not use all of mia
    void o_file__init(void);
    o_file__init();


    o_log_s(__func__, "start");
}

bool o_init_called(void)
{
    return o_common_L.init;
}


#define MSG_SIZE 4096

void o__exit_impl(const char *file, int line, const char *reason_format, ...)
{
    va_list args;
    va_start(args, reason_format);
    char *msg = SDL_malloc(MSG_SIZE);
    vsnprintf(msg, MSG_SIZE, reason_format, args);
    va_end(args);


#ifdef NDEBUG
    fprintf(stderr, "-> Program exit, reason: %s\n", msg);
#else
    fprintf(stderr, "-> Program exit at %s:%d %s\n", file, line, msg);
#endif

#ifdef MIA_PLATFORM_EMSCRIPTEN
    {
        // exit emscriptens main loop and call js error handler
        int script_size = 2*MSG_SIZE;
        char *script = SDL_malloc(script_size);
        snprintf(script, script_size, "o__exit(\'%s\');", msg);
        emscripten_cancel_main_loop();
        emscripten_run_script(script);
    }
#endif

#ifdef MIA_PLATFORM_ANDROID
    {
        // show a small toast dialog in the app and then exit
        int toast_size = 2 * MSG_SIZE;
        char *toast = SDL_malloc(toast_size);
        snprintf(toast, toast_size, "Program exit, reason: %s", msg);
        SDL_ShowAndroidToast(toast, 1, -1, 0, 0);
        SDL_Delay(8000);
    }
#endif

    o__sanitizer_leak_check("o_exit", true);

    exit(EXIT_FAILURE);

    // should not get here...
    for (;;) {
        SDL_Delay(1);
    }
}

void o__assume_impl(const char *expression, const char *file, int line, const char *reason_format, ...)
{
    va_list args;
    va_start(args, reason_format);
    char *msg = SDL_malloc(MSG_SIZE);
    vsnprintf(msg, MSG_SIZE, reason_format, args);
    va_end(args);

    char *full_msg = SDL_malloc(MSG_SIZE);
#ifdef NDEBUG
    snprintf(full_msg, MSG_SIZE, "Assumption failed: %s", msg);
#else
    snprintf(full_msg, MSG_SIZE, "Assumption failed: (%s) %s", expression, msg);
#endif

    o__exit_impl(file, line, full_msg);
}


ou64 o_thread_id(void)
{
#ifdef MIA_OPTION_THREAD
#  ifdef MIA_OPTION_SDL2
    return SDL_ThreadID();
#  else
    return SDL_GetCurrentThreadID();
#  endif
#else
    return 0;
#endif
}

ou64 o_thread_main_id(void)
{
    return o_common_L.main_thread_id;
}

bool o_thread_set_priority(enum OThread_priority priority)
{
    // enums should match, just in case
    SDL_ThreadPriority sdl_prio = (SDL_ThreadPriority[]) {SDL_THREAD_PRIORITY_LOW,
                            SDL_THREAD_PRIORITY_NORMAL,
                            SDL_THREAD_PRIORITY_HIGH,
                            SDL_THREAD_PRIORITY_TIME_CRITICAL}[priority];

    return SDL_SetCurrentThreadPriority(sdl_prio) == O_SDL_OK;
}

void o_sleep(osize millis)
{
    millis = o_max(0, millis);
    SDL_Delay((Uint32) millis);
}


ou32 o_rand(void)
{
    ou32 y;
    if (o_common_L_mt_index >= MT_N) {
        o_common_L_mt_index = 0;
        // twist
        for (int i = 0; i < MT_N; i++) {
            ou32 z = (o_common_L_mt[i] & MT_UPPER_MASK) | (o_common_L_mt[(i + 1) % MT_N] & MT_LOWER_MASK);
            o_common_L_mt[i] = o_common_L_mt[(i + MT_M) % MT_N] ^ (z >> 1);
            if (z & 1) {
                o_common_L_mt[i] ^= MT_MATRIX_A;
            }
        }
    }

    y = o_common_L_mt[o_common_L_mt_index++];

    // Tempering
    y ^= (y >> 11);
    y ^= (y << 7) & 0x9D2C5680U;
    y ^= (y << 15) & 0xEFC60000U;
    y ^= (y >> 18);

    return y;
}

void o_rand_seed(ou32 seed)
{
    o_common_L_mt[0] = seed;
    for (o_common_L_mt_index = 1; o_common_L_mt_index < MT_N; o_common_L_mt_index++) {
        o_common_L_mt[o_common_L_mt_index] = (1812433253U * (o_common_L_mt[o_common_L_mt_index - 1] ^ (o_common_L_mt[o_common_L_mt_index - 1] >> 30)) + o_common_L_mt_index);
    }
}

oobj o_context(void)
{
    return o_common_L.context;
}

oobj o_arena(void)
{
    assert(o_common_L_arena_root && "call o_arena_set (thread local)");
    return o_common_L_arena_root;
}

oobj o_arena_set(oobj set)
{
    o_common_L_arena_root = set;
    return o_common_L_arena_root;
}

// protected
bool o__ignore_pause(void)
{
    return o_common_L.ignore_pause;
}

// protected
bool o__ignore_pause_set(bool set)
{
    o_common_L.ignore_pause = set;
    return o_common_L.ignore_pause;
}

void *o_noop(void *inout)
{
    return inout;
}
