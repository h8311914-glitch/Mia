#ifndef R_COMMON_H
#define R_COMMON_H

/**
 * @file common.h
 *
 * Common used | Basic includes, macros, types and functions
 */


#include "o/common.h"
#include "m/types/byte.h"
#include "m/types/int.h"
#include "m/types/flt.h"
#include "m/format.h"

// GLES and WebGL use GLES3.0
#ifdef MIA_OPTION_GLES
#  define R_GL_VERSION_MAJOR 3
#  define R_GL_VERSION_MINOR 0

// GL uses GL3.3
#else
#  define R_GL_VERSION_MAJOR 3
#  define R_GL_VERSION_MINOR 3
#endif

//
// some useful default colors
//

static const vec4 R_TRANSPARENT = {{0, 0, 0, 0}};
static const vec4 R_BLACK = {{0, 0, 0, 1}};
static const vec4 R_WHITE = {{1, 1, 1, 1}};
static const vec4 R_RED = {{1, 0, 0, 1}};
static const vec4 R_GREEN = {{0, 1, 0, 1}};
static const vec4 R_BLUE = {{0, 0, 1, 1}};
static const vec4 R_YELLOW = {{1, 1, 0, 1}};
static const vec4 R_CYAN = {{0, 1, 1, 1}};
static const vec4 R_MAGENTA = {{1, 0, 1, 1}};
static const vec4 R_GRAY = {{0.5, 0.5, 0.5, 1}};

#define R_GRAY_X(val) vec4_((val),(val),(val),1)

#define R_HUE_RED 0.0
#define R_HUE_GREEN (1.0/3.0)
#define R_HUE_BLUE (1.0/3.0)
#define R_HUE_YELLOW (0.5/3.0)
#define R_HUE_CYAN (1.5/3.0)
#define R_HUE_MAGENTA (2.5/3.0)

//
// pixel formats for RTex and co.
// Based on m/format.h, so can be used interchangeable.
// But not only those listed here are valid in RTex and co.
// Passed as ou32 cause its a bitflag, see m/format.h
//


#define R_FORMAT_KEEP M_FORMAT_KEEP
#define R_FORMAT_R_U8 M_FORMAT_R_U8
#define R_FORMAT_R_F32 M_FORMAT_R_F32
#define R_FORMAT_RG_U8 M_FORMAT_RG_U8
#define R_FORMAT_RG_F32 M_FORMAT_RG_F32
#define R_FORMAT_RGB_U8 M_FORMAT_RGB_U8
#define R_FORMAT_RGB_F32 M_FORMAT_RGB_F32
#define R_FORMAT_RGBA_U8 M_FORMAT_RGBA_U8
#define R_FORMAT_RGBA_F32 M_FORMAT_RGBA_F32


/**
 * @return true if the format is a valid RTex format
 */
#define r_format_valid(format) (\
   (format)==R_FORMAT_R_U8\
|| (format)==R_FORMAT_R_F32\
|| (format)==R_FORMAT_RG_U8\
|| (format)==R_FORMAT_RG_F32\
|| (format)==R_FORMAT_RGB_U8\
|| (format)==R_FORMAT_RGB_F32\
|| (format)==R_FORMAT_RGBA_U8\
|| (format)==R_FORMAT_RGBA_F32)

/**
 * @param format r or m style format
 * @return a format that is valid as r format (RTex format), even if it was an m format thats not supported.
 *         Uses the nearest type on missmatch
 */
ou32 r_format_valid_make(ou32 format);

/**
 * Asserts the given format is valid for being used in an RTex
 */
#define r_format_assert(format) assert(r_format_valid(format))


/**
 * Should be called with an active OpenGL context
 * @param opt_root a root that will NOT be deleted or NULL to create a heap root internally
 */
void r_init(oobj opt_root);


/**
 * @return A default allocator for the "r" module.
 * Allocates stuff like default RShader's in RObj's.
 */
oobj r_root(void);


/**
 * @return The size of the current back output buffer
 */
ivec2 r_back_size_int(void);

/**
 * @return The size of the current back output buffer
 * @note integer values
 */
o_inline vec2 r_back_size(void)
{
    ivec2 size = r_back_size_int();
    return vec2_(size.x, size.y);
}

/**
 * @return the default back projection to edit
 */
struct r_proj *r_back_proj(void);

/**
 * @return maximal possible size for a texture (RTex) to use (device specific...)
 * @note cols==rows
 */
ivec2 r_max_tex_size(void);

/**
 * @return number of draw calls emitted last frame
 */
int r_draw_calls(void);

/**
 * @return number of draw calls emitted since the last frame
 */
int r_draw_calls_current_frame(void);


//
// less needed stuff
//


/**
 * Starts a frame and updates the back size
 * @note The viewport to draw into the output frame buffer will be set to the next even size.
 */
void r_frame_begin(int back_cols, int window_rows);

/**
 * Ends a frame and finishes pending draw calls
 */
void r_frame_end(void);


/**
 * Checks for OpenGL errors
 */
bool r_error_check_call(const char *file, int line, const char *tag);

/**
 * Checks for OpenGL errors, if debug mode and MIA_OPTION_GL_CHECK
 */
#if !defined(NDEBUG) && defined(MIA_OPTION_GL_CHECK)
#  define r_error_check(tag) r_error_check_call(__FILE__, __LINE__, (tag))
#else
#  define r_error_check(tag) ((void)0)
#endif

#endif //R_COMMON_H
