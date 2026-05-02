#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_timer.h>
#else
#  include <SDL3/SDL_timer.h>
#endif

#include "o/timer.h"





ou64 o_timer(void)
{
    return SDL_GetPerformanceCounter();
}

ou64 o_timer_freq_s(void)
{
    return SDL_GetPerformanceFrequency();
}
