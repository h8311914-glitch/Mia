#ifndef R_GL_H
#define R_GL_H

/**
 * @file gl.h
 *
 * Include this file to include OpenGL functions
 */

#include "common.h"


#define GL_GLEXT_PROTOTYPES

#ifdef MIA_OPTION_GLEW
#  include <GL/glew.h>
#else

#ifdef MIA_OPTION_SDL2
#  include <SDL2/SDL_opengl.h>
#else
#  include <SDL3/SDL_opengl.h>
#endif

#endif

#endif //R_GL_H
