#ifndef R_RTEX_H
#define R_RTEX_H

/**
 * @file RTex.h
 *
 * Object
 *
 * Texture buffer allocated on the GPU.
 * Some functions work with a NULL passed as object to work on the back buffer.
 * So of the rendering is done onto RTex tex'tures
 *
 * @sa RTex_ex.h to import RTex and all its object extensions
 */


#include "o/OObj.h"
#include "r/common.h"
#include "r/proj.h"
#include "r/quad.h"
#include "r/box.h"

/** object id */
#define RTex_ID OObj_ID "RTex."


enum RTex_wrap_modes {
    RTex_wrap_CLAMP,
    RTex_wrap_REPEAT,
    RTex_wrap_ENUM_MAX
};

enum RTex_filter_modes {
    RTex_filter_NEAREST,
    RTex_filter_LINEAR,
    RTex_filter_LINEAR_NEAREST,
    RTex_filter_ENUM_MAX
};

struct RTex_fbo {
    ou32 gl_fbo;
    struct r_proj proj;
};

typedef struct {
    OObj super;

    // may be 0 for an invalid tex (for example cols | rows = 0)
    ou32 gl_tex;

    // const, create a new tex to change
    ivec2 size;

    // as ltwh, defaults to ivec4_(0, 0, size.x, size.y)
    ivec4 viewport;

    // const, create a new tex to change
    ou32 format;

    enum RTex_wrap_modes wrap_mode;
    enum RTex_filter_modes filter_mode;

    // OPtr of optional RTex attachment for layout=1 (automatically second out vec4...)
    oobj attached_tex_ptr;

    // lazy init
    struct RTex_fbo *fbo;
} RTex;


// forward
struct o_img;
struct r_textlayout;


/**
 * Initializes the object
 * @param obj RTex object
 * @param parent to inherit from
 * @param opt_buffer set the tex pixels, must be the size of RTex_buffer_size(format)
 * @param cols, rows columns and rows to allocate, if <=0 RTex will be invalid (RTex_valid() == false; RTex_tex()==0).
 * @param format_buffer for the opt_buffer.
 * @param format_tex used in the gpu texture. 
 *                   Use 0 to use same as format_buffer.
 * @return obj casted as RTex
 */
RTex *RTex_init_ex(oobj obj, oobj parent, const void *opt_buffer, int cols, int rows,
                   ou32 format_buffer, ou32 format_tex);

/**
 * Creates a new RTex object
 * @param parent to inherit from
 * @param opt_buffer set the tex pixels, must be the size of RTex_buffer_size(format)
 * @param cols, rows columns and rows to allocate, if <=0 RTex will be invalid (RTex_valid() == false; RTex_tex()==0).
 * @param format_buffer for the opt_buffer.
 * @param format_tex used in the gpu texture. 
 *                   Use 0 to use same as format_buffer.
 * @return The new object
 */
o_inline RTex *RTex_new_ex(oobj parent, const void *opt_buffer, int cols, int rows,
                  ou32 format_buffer, ou32 format_tex)
{
    OObj_DECL_IMPL_NEW_SPECIAL(RTex, ex, parent, opt_buffer, cols, rows, format_buffer, format_tex);
}

/**
 * Creates a new RTex object.
 * @param parent to inherit from
 * @param opt_buffer set the tex pixels, must be the size of RTex_buffer_size(R_FORMAT_RGBA_U8)
 * @param cols, rows columns and rows to allocate, if <=0 RTex will be invalid (RTex_valid() == false; RTex_tex()==0).
 * @return The new object.
 * @note R_FORMAT_RGBA_U8 is used internally.
 */
o_inline RTex *RTex_new(oobj parent, const void *opt_buffer, int cols, int rows)
{
    return RTex_new_ex(parent, opt_buffer, cols, rows, R_FORMAT_RGBA_U8, R_FORMAT_RGBA_U8);
}

/**
 * Creates a new RTex object.
 * Loads an image with o_img for the tex buffer.
 * @param parent to inherit from
 * @param img to copy the content from
 * @return The new object.
 * @note uses the format of the o_img, so M_FORMAT__PRIMITIVE_U8 + img->channels.
 *       May be invalid if img failed is empty, test with RTex_valid()
 */
RTex *RTex_new_img(oobj parent, const struct o_img *img);

/**
 * Creates a new RTex object.
 * Loads the texture data from an MMat object.
 * @param parent to inherit from
 * @param mat MMat object to copy the content from
 * @param format if 0, uses the MMat_format (if possible), else casts into it.
 * @return The new object.
 * @note if the wanted format is invalid for RTex, its casted to the nearest valid and logged as warning
 *       With the exception of M_FORMAT_1_UR8, which is silently changed to M_FORMAT_1_U8 (indexed images)
 */
RTex *RTex_new_mat(oobj parent, oobj mat, ou32 format);


/**
 * Creates a new RTex object.
 * Loads an image with o_img for the tex buffer.
 * Supports at least .jpg and .png
 * @param parent to inherit from
 * @param file to load the tex from, either png or jpg, loaded with SDL_image
 * @return The new object.
 * @note R_FORMAT_RGBA_U8 is used internally.
 *       May be invalid if file failed to read, test with RTex_valid()
 */
RTex *RTex_new_file(oobj parent, const char *file);

/**
 * Creates a new RTex object.
 * Blits the current back buffer into the new RTex.
 * @param parent to inherit from
 * @return The new object
 * @note R_FORMAT_RGBA_U8 is used internally.
 *       calls RTex_blit_back internally
 */
RTex *RTex_new_blit_back(oobj parent);

/**
 * Creates a kernel tex.
 * @param parent to inherit from
 * @param cols, rows to allocate
 * @param kernel float data row major of the kernel
 * @param format typically R_FORMAT_R_F32.
 *               if R channel only, its just the kernel data.
 *               if RGBA, the kernel is in the alpha channel, else is full white
 *               RG or RGB not supported
 * @return The new object
 */
RTex *RTex_new_kernel_buffer(oobj parent, int cols, int rows, const float *kernel, ou32 format);

/**
 * Creates a kernel tex.
 * @param parent to inherit from
 * @param cols, rows to allocate
 * @param set for all values
 * @param plus if true, edges cleared to 0.0
 * @param normalize if >0.0: normalize to given
 * @param format typically R_FORMAT_R_F32.
 *               if R channel only, its just the kernel data.
 *               if RGBA, the kernel is in the alpha channel, else is full white
 *               RG or RGB not supported
 * @return The new object
 */
RTex *RTex_new_kernel(oobj parent, int cols, int rows, float set, bool plus, float normalize, ou32 format);

/**
 * Creates a kernel tex with a gauss 2d bell kurve.
 * @param parent to inherit from
 * @param cols, rows to allocate
 * @param sigma gauss factor for x and y axis, pass <=0 to auto scale (vec2_(-1))
 * @param normalize, if <=0: center is 1.0
 *                     else: normalized to given value (... 1.0f)
 * @param format typically R_FORMAT_R_F32.
 *               if R channel only, its just the kernel data.
 *               if RGBA, the kernel is in the alpha channel, else is full white
 *               RG or RGB not supported
 * @return The new object
 */
RTex *RTex_new_kernel_gauss(oobj parent, int cols, int rows, vec2 sigma, float normalize, ou32 format);

/**
 * Creates a kernel tex with a gauss 2d bell kurve.
 * @param parent to inherit from
 * @param cols, rows to allocate
 * @param norm_radius in (0:1], if <0 its gonne be 1.05 (which fits well to touch the border up to at least 64x64)
 * @param normalize, if <=0: center is 1.0
 *                     else: normalized to given value (... 1.0f)
 * @param format typically R_FORMAT_R_F32.
 *               if R channel only, its just the kernel data.
 *               if RGBA, the kernel is in the alpha channel, else is full white
 *               RG or RGB not supported
 * @return The new object
 */
RTex *RTex_new_kernel_ellipse(oobj parent, int cols, int rows, float norm_radius, float normalize, ou32 format);


#ifdef MIA_OPTION_TTF
/**
 * Creates a text in white drawn onto the texture
 * @param parent to inherit from
 * @param file .ttf file to load from
 * @param size in pixels for the height of the basic text
 *             Resulting tex will be larger
 * @param text to be drawn (utf-8) (in white, use a color shader for colouring)
 * @return The new object
 * @note R_FORMAT_RGBA_U8 is used internally.
 *       May be invalid if file failed to read, test with RTex_valid()
 *       Uses OTtf internally. To cache for dynamic or more texts, use that instead to create o_img's
 */
RTex *RTex_new_ttf(oobj parent, const char *file, float size, const char *text);
#endif


//
// virtual implementations:
//

/**
 * Default deletor that deletes the internal tex
 * @param obj RTex object
 */
void RTex__v_del(oobj obj);


//
// object functions:
//

/**
 * @param obj RTex object
 * @return the opengl tex handle, if NULL: 0
 */
o_inline ou32 RTex_tex(oobj obj)
{
    if (!obj) {
        return 0;
    }
    OObj_assert(obj, RTex);
    RTex *self = obj;
    return self->gl_tex;
}

/**
 * @param obj RTex object
 * @return true if valid
 */
o_inline bool RTex_valid(oobj obj)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    return self->gl_tex != 0;
}


/**
 * @param obj RTex object
 * @return current internal pixel format
 */
OObj_DECL_GET(RTex, ou32, format)

/**
 * @param obj RTex object, if NULL: r_back_size is returned
 * @return size of the allocated framebuffer
 */
o_inline ivec2 RTex_size_int(oobj obj)
{
    if (!obj) {
        return r_back_size_int();
    }
    OObj_assert(obj, RTex);
    RTex *self = obj;
    return self->size;
}

/**
 * @param obj RTex object, if NULL: back buffer
 * @return viewport ltwh of the allocated framebuffer
 */
ivec4 RTex_viewport(oobj obj);

/**
 * @param obj RTex object, if NULL: back buffer
 * @return viewport ltwh of the allocated framebuffer
 */
ivec4 RTex_viewport_set(oobj obj, ivec4 set);

/**
 * @param obj RTex object, if NULL: back buffer
 * @return viewport for the full tex: vec4_(0, 0, m_2(RTex_size_int(obj))
 */
o_inline ivec4 RTex_viewport_set_full(oobj obj)
{
    ivec4 viewport = ivec4_(0);
    viewport.zw = RTex_size_int(obj);
    return RTex_viewport_set(obj, viewport);
}

/**
 * @param obj RTex object, if NULL: back buffer
 * @param rect as l, t, w, h (in projected space)
 * @return viewport ltwh of the allocated framebuffer
 */
ivec4 RTex_viewport_set_rect(oobj obj, vec4 rect);

/**
 * @param obj RTex object, if NULL: back buffer
 * @param pose_aa axis aligned pose in projection space
 * @return viewport ltwh of the allocated framebuffer
 */
ivec4 RTex_viewport_set_pose_aa(oobj obj, mat4 pose_aa);

/**
 * @param obj RTex object, if NULL: r_back_size is returned
 * @return size of the allocated framebuffer
 * @note integer values
 */
o_inline vec2 RTex_size(oobj obj)
{
    ivec2 size = RTex_size_int(obj);
    return vec2_(size.x, size.y);
}

/**
 * @param obj RTex object, if NULL: r_back_size is returned
 * @return number of pixels (size.x * size.y)
 */
o_inline osize RTex_num(oobj obj)
{
    ivec2 size = RTex_size_int(obj);
    return (osize) size.x * (osize) size.y;
}

/**
 * @param obj RTex object
 * @return bytes needed to get or set the buffer (RTex_num * r_format_size)
 */
o_inline osize RTex_buffer_size(oobj obj, ou32 format)
{
    OObj_assert(obj, RTex); // to assert obj not NULL
    format = format == R_FORMAT_KEEP ? RTex_format(obj) : format;
    return RTex_num(obj) * m_format_size(format);
}

/**
 * @param obj RTex object
 * @return the current wrap mode
 */
o_inline enum RTex_wrap_modes RTex_wrap(oobj obj)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    return self->wrap_mode;
}

/**
 * Updates the wrap mode, if texture is sampled outside its borders through uv mapping
 * @param obj RTex object
 * @param mode CLAMP to border; REPEAT texture
 */
void RTex_wrap_set(oobj obj, enum RTex_wrap_modes mode);

/**
 * @param obj RTex object
 * @return the current filter mode
 */
o_inline enum RTex_filter_modes RTex_filter(oobj obj)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    return self->filter_mode;
}

/**
 * Updates the filter mode
 * @param obj RTex object
 * @param mode: LINEAR:          high res textures
 *              NEAREST:         pixel art (default)
 *              LINEAR_NEAREST: pixel art, but linear of sprite size > render size
 */
void RTex_filter_set(oobj obj, enum RTex_filter_modes mode);


/**
 * Creates a rect containing the full tex for rendering with quads, etc. (also for uvs...)
 * @param obj RTex object, if NULL r_back_size is used internally
 * @param x position (left)
 * @param y position (top)
 * @return rect (ltwh) containing the full tex on position xy (lt), also usable for uv's
 * @note mostly valid for render with, not render on. see r_proj_left, etc.
 */
o_inline vec4 RTex_rect(oobj obj, float x, float y)
{
    vec2 size = RTex_size(obj);
    return (vec4){{x, y, size.x, size.y}};
}

/**
 * Creates a pose containing the full tex for rendering with quads, etc. (also for uvs...)
 * @param obj RTex object, if NULL r_back_size is used internally
 * @param x position (left)
 * @param y position (top)
 * @return pose containing the full tex on position xy (left top), also usable for uv's
 * @note mostly valid for render with, not render on. see r_proj_left, etc.
 */
o_inline mat4 RTex_pose(oobj obj, float x, float y)
{
    vec2 size = RTex_size(obj);
    return (mat4){
        {
            size.x, 0, 0, 0,
            0, size.y, 0, 0,
            0, 0, 1, 0,
            x, y, 0, 1
        }
    };
}


/**
 * Create a new RCam (pixelperfect; min_unit_radius = vec2_(-1)) (so without scaling = 1)
 * @param obj RTex object
 */
oobj RTex_cam(oobj obj);


/**
 * Returns the tex scaling used by render objects
 * @param obj RTex object
 * @return 1 / tex_size.
 */
o_inline vec2 RTex_get_tex_scale(oobj obj)
{
    vec2 size = RTex_size(obj);
    return vec2_(1.0f / size.x, 1.0f / size.y);
}


//
// fbo stuff
//

/**
 * Use this RTex as active framebuffer (called by shaders)
 * @param obj RTex object
 * @param requested_draw_buffers from RProgram_num_draw_buffers.
 *                               the number of "out vec4 f_rgba" in fragment shader (typically 1)
 */
void RTex_use(oobj obj, int requested_draw_buffers);


/**
 * Deletes the internal fbo, proj, etc. if available.
 * Could be called after editing this texture on setup.
 * @param obj RTex object
 */
void RTex_use_done(oobj obj);

/**
 * Retrieves the current buffer.
 * @param obj RTex object
 * @param out_buffer a buffer of (at least) RTex_buffer_size
 * @param format for the out_buffer pixels
 * @note internal and given format may be different.
 *       row major, starting with top left.
 */
void RTex_get_ex(oobj obj, void *out_buffer, ou32 format);

/**
 * Retrieves the current buffer as R_FORMAT_RGBA_U8 (default).
 * @param obj RTex object
 * @param out_buffer a buffer of (at least) RTex_buffer_size
* @note internal and given format may be different.
 *       row major, starting with top left.
 */
o_inline void RTex_get(oobj obj, void *out_buffer)
{
    RTex_get_ex(obj, out_buffer, R_FORMAT_RGBA_U8);
}

/**
 * Sets the buffer
 * @param obj RTex object
 * @param buffer a buffer of (at least) RTex_buffer_size size
 * @param format for the buffer pixels
 * @note internal and given format may be different.
 *       row major, starting with top left.
 */
void RTex_set_ex(oobj obj, const void *buffer, ou32 format);

/**
 * Sets the buffer as R_FORMAT_RGBA_U8 (default)
 * @param obj RTex object
 * @param buffer a buffer of (at least) RTex_buffer_size
 * @note internal and given format may be different.
 *       row major, starting with top left.
 */
o_inline void RTex_set(oobj obj, const void *buffer)
{
    RTex_set_ex(obj, buffer, R_FORMAT_RGBA_U8);
}


/**
 * Allocates a new MMat with the current RTex buffer, see RTex_get
 * @param obj RTex object
 * @param format for the MMat (valid RTex format)
 *               or 0 to keep / use the same RTex format
 * @return MMat allocated on obj
 * @note asserts for RTex_valid      
 */
oobj RTex_mat(oobj obj, ou32 format);


/**
 * @param obj RTex object to get values from
 * @param mat MMat object to set values into
 * @note asserts sizes matches and logs a warns if mat is in an invalid format (needs an internal cast)
 */
void RTex_get_into_mat(oobj obj, oobj mat);

/**
 * @param obj RTex object to set values to
 * @param mat MMat object to read values from
 * @note asserts sizes matches and logs a warns if mat is in an invalid format (needs an internal cast)
 */
void RTex_set_from_mat(oobj obj, oobj mat);

/**
 * Save the tex image as .png file (rgba 8bit)
 * @param obj RTex object
 * @param file png file to write into
 * @return false if failed
 * @note uses R_FORMAT_RGBA_U8 to save
 */
bool RTex_write_file(oobj obj, const char *file);

/**
 * @param obj RTex object
 * @return a reference the default render projection, defaults to a minradius -> min units cam
 */
struct r_proj *RTex_proj(oobj obj);


/**
 * @param obj RTex object
 * @return the attached RTex object or NULL
 */
struct oobj_opt RTex_attach(oobj obj);


/**
 * Sets opt_tex_attachment as second color attachment for rendering (or disable it, if NULL is passed)
 * @param obj RTex object
 * @param opt_tex_attachment RTex object to attach onto obj, or NULL to disable
 */
oobj RTex_attach_set(oobj obj, oobj opt_tex_attachment);


//
// render stuff
//


/**
 * @param obj RTex object, if NULL, window buffer is used instead
 * @param clear_color to set as draw color clearing the full texture
 * @note ignores the viewport
 */
void RTex_clear_full(oobj obj, vec4 clear_color);


/**
 * @param obj RTex object, if NULL, window buffer is used instead
 * @param clear_color to set as draw color, draws within the viewport
 * @param blend if true, else blits
 * @note draws a full size'd quad
 */
void RTex_clear_ex(oobj obj, vec4 clear_color, bool blend);


/**
 * @param obj RTex object, if NULL, window buffer is used instead
 * @param clear_color to set as draw color, draws within the viewport
 * @note uses blitting to draw a fullsize'd quad
 * @note draws a full size'd quad
 */
o_inline void RTex_clear(oobj obj, vec4 clear_color)
{
    RTex_clear_ex(obj, clear_color, false);
}

/**
 * Calls the virtual render function of the given RShader.
 * @param obj RTex object
 * @param shader RShader object
 * @param buffer RBuffer object
 * @param opt_num_rendered if <=0: RBuffer_num is used, else o_min(num_rendered, 1, RBuffer_num(buffer))
 * @param opt_proj the camera projection to use (vp), if NULL RTex_proj(tex) is used
 * @note calls RShader_render_ex internally
 */
void RTex_shader_ex(oobj obj, oobj shader, oobj buffer, int opt_num_rendered, const struct r_proj *opt_proj);

/**
 * Calls the virtual render function of the given RShader.
 * Simpler default version.
 * @param obj RTex object
 * @param shader RShader object
 * @param buffer RBuffer object
 * @note just calls RTex_shader_ex(obj, buffer, tex, 0, NULL)
 */
o_inline void RTex_shader(oobj obj, oobj shader, oobj buffer)
{
    RTex_shader_ex(obj, shader, buffer, 0, NULL);
}

/**
 * Calls the virtual render function of the given RObj.
 * Extended version.
 * Renders the object to the bound frame buffer.
 * Uses all shaders in the shader_pipeline one by one.
 * @param obj RTex object
 * @param ro RObj object
 * @param opt_proj the camera projection to use (vp), if NULL: RTex_proj(tex) is used instead
 * @param update if true: calls RObj_update first
 * @note calls RObj_render_ex internally
 */
void RTex_ro_ex(oobj obj, oobj ro, const struct r_proj *opt_proj, bool update);

/**
 * Calls the virtual render function of the given RObj.
 * Simpler default version.
 * Renders the object to the bound frame buffer
 * @param obj RTex object
 * @param ro RObj object
 * @note just calls RTex_ro_ex(obj, ro, NULL, true)
 */
o_inline void RTex_ro(oobj obj, oobj ro)
{
    RTex_ro_ex(obj, ro, NULL, true);
}

/**
 * Renders a bunch of quads on this RTex using the given RShader.
 * @param obj RTex object
 * @param shader RShader object
 */
void RTex_quads(oobj obj, oobj shader, const struct r_quad *quads, int n);

/**
 * Renders a bunch of boxes on this RTex using the given RShader.
 * @param obj RTex object
 * @param shader RShader object
 */
void RTex_boxes(oobj obj, oobj shader, const struct r_box *boxs, int n);





#endif //R_RTEX_H
