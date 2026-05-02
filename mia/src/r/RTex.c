#include "r/RTex.h"
#include "o/OObj_builder.h"

#include "RTex_L.h"
#include "r/RTex_blit.h"
#include "r/RTex_channel.h"

#include "o/OPtr.h"
#include "o/img.h"
#include "o/OArray.h"
#include "m/vec/ivec2.h"
#include "m/MMat.h"
#include "m/utils/kernel.h"
#include "r/RObj.h"
#include "r/RCam.h"
#include "r/tex.h"
#include "r/program.h"
#include "r/gl.h"

#undef O_LOG_LIB
#define O_LOG_LIB "r"
#include "o/log.h"


// shared in RTex_*.c modules as extern reference
struct RTex__L_t RTex__L;



static GLenum format_channels(ou32 format)
{
    int channels = m_format_channels(format);
    switch (channels) {
        case 1:
            return GL_RED;
        case 2:
            return GL_RG;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        default:
            break;
    }
    assert(0 && "invalid format");
    // should not get here...
    return 0;
}

static GLenum format_size(ou32 format)
{
    oi32 primitive = m_format_primitive(format);
    switch (primitive) {
        case M_FORMAT__PRIMITIVE_U8:
            return GL_UNSIGNED_BYTE;
        case M_FORMAT__PRIMITIVE_F32:
            return GL_FLOAT;
        default:
            break;
    }
    assert(0 && "invalid format");
    // should not get here...
    return 0;
}


static GLint format_internal(ou32 format)
{
    switch (format) {
        case R_FORMAT_R_U8:
            return GL_R8;
        case R_FORMAT_R_F32:
            return GL_R32F;
        case R_FORMAT_RG_U8:
            return GL_RG8;
        case R_FORMAT_RG_F32:
            return GL_RG32F;
        case R_FORMAT_RGB_U8:
            return GL_RGB8;
        case R_FORMAT_RGB_F32:
            return GL_RGB32F;
        case R_FORMAT_RGBA_U8:
            return GL_RGBA8;
        case R_FORMAT_RGBA_F32:
            return GL_RGBA32F;
        default:
            break;
    }
    assert(0 && "invalid format");
    // should not get here...
    return 0;
}


static void update_fbo(RTex *self, int requested_num_draw_buffers)
{
    if (!self->fbo) {
        // create a new fbo and proj
        self->fbo = o_new0(self, struct RTex_fbo, 1);
        self->fbo->proj = r_proj_new(RTex_size_int(self), -1, vec2_(-1), true);
        glGenFramebuffers(1, &self->fbo->gl_fbo);
    }

    ou32 attachment1 = 0;
    RTex *attached_tex = OPtr_get(self->attached_tex_ptr).o;
    if (requested_num_draw_buffers == 2 && OObj_check(attached_tex, RTex)) {
        attachment1 = attached_tex->gl_tex;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, self->fbo->gl_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self->gl_tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, attachment1, 0);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    switch (status) {
        case GL_FRAMEBUFFER_COMPLETE:
            break;
        case GL_FRAMEBUFFER_UNDEFINED:
            o_log_error_s("RTex fbo", "Framebuffer undefined");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            o_log_error_s("RTex fbo", "Framebuffer incomplete attachment");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            o_log_error_s("RTex fbo", "Framebuffer incomplete missing attachment");
            break;
        case GL_FRAMEBUFFER_UNSUPPORTED:
            o_log_error_s("RTex fbo", "Framebuffer unsupported");
            break;
        case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
            o_log_error_s("RTex fbo", "Framebuffer incomplete multisample");
            break;
        case 0:
            o_log_error_s("RTex fbo", "Framebuffer failed to get status");
            r_error_check_call(__FILE__, __LINE__, "RTex fbo");
            break;
        default:
            o_log_error_s("RTex fbo", "Unknown framebuffer status");
            break;
    }

    if (OObj_check(OPtr_get(self->attached_tex_ptr).o, RTex)) {
        glDrawBuffers(2, (GLenum[]){GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1});
    } else {
        glDrawBuffers(1, (GLenum[]){GL_COLOR_ATTACHMENT0});
    }
}


void RTex__init(void)
{
    // get the default back framebuffer
    GLint current_fbo;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &current_fbo);
    RTex__L.back_fbo = (ou32) current_fbo;

    RTex__L.o = OObj_new(r_root());
    OObj_name_set(RTex__L.o, "RTex_render");

    RTex__L.b_box = RBufferBox_new(RTex__L.o);
    RTex__L.b_quad = RBufferQuad_new(RTex__L.o);

    RTex__L.s_box = RShaderBox_new(RTex__L.o, NULL, false);
    RTex__L.s_box_indexed = RShaderBoxIndexed_new(RTex__L.o, NULL, false, NULL);
    RTex__L.s_quad = RShaderQuad_new(RTex__L.o, NULL, false);
    RTex__L.s_quad_channelmerge = RShaderQuadChannelmerge_new(RTex__L.o, NULL, NULL, NULL, NULL, false);
    RTex__L.s_quad_kernel = RShaderQuadKernel_new(RTex__L.o, NULL, false, NULL, false);
    RTex__L.s_quad_merge = RShaderQuadMerge_new(RTex__L.o, NULL, false);
    RTex__L.s_quad_op = RShaderQuadOp_new(RTex__L.o, NULL, false);

    RTex__L.draw_boxes_array = OArray_new_dyn(RTex__L.o, NULL, sizeof(struct r_box), 0, 256);
    RTex__L.draw_dab_array = OArray_new_dyn(RTex__L.o, NULL, sizeof(vec2), 0, 256);

    r_error_check("init");
}

ou32 RTex__bound_gl(void)
{
    // NULL safe -> 0
    return RTex_tex(RTex__L.bound_tex);
}

//
// public
//


RTex *RTex_init_ex(oobj obj, oobj parent, const void *opt_buffer, int cols, int rows,
                   ou32 format_buffer, ou32 format_tex)
{
    if (format_tex == R_FORMAT_KEEP) {
        format_tex = format_buffer;
    }
    // will also internally assert...
    r_format_assert(format_buffer);
    r_format_assert(format_tex);

    RTex *self = obj;
    o_clear(self, sizeof *self, 1);

    OObj_init(self, parent);
    OObj_id_set(self, RTex_ID);

    self->size = ivec2_(cols, rows);
    self->viewport = ivec4_(0, 0, cols, rows);
    self->format = format_tex;

    self->attached_tex_ptr = OPtr_new(self, NULL);

    // deletor
    self->super.v_del = RTex__v_del;

    if (cols <= 0 || rows <= 0) {
        o_log_warn_s(__func__, "invalid");
        self->gl_tex = 0;
        return self;
    }

    r_error_check("tex allocation start...");

    glGenTextures(1, &self->gl_tex);
    glBindTexture(GL_TEXTURE_2D, self->gl_tex);

    // default may be that new rows are not byte aligned, so 1 for byte aligned if format_size is < 4 (R-RGB U8)
    glPixelStorei(GL_UNPACK_ALIGNMENT, m_format_size(self->format)<4 ? 1 : 4);
    r_error_check("tex allocation (before teximage)");
    // safe to pass NULL as opt_buffer
    glTexImage2D(GL_TEXTURE_2D, 0, format_internal(format_tex),
                 self->size.x, self->size.y,
                 0, format_channels(format_buffer), format_size(format_buffer), opt_buffer);
    r_error_check("tex allocation");

    glBindTexture(GL_TEXTURE_2D, 0);

    RTex_wrap_set(self, RTex_wrap_CLAMP);
    RTex_filter_set(self, RTex_filter_NEAREST);

    r_error_check("tex");

    return self;
}

RTex *RTex_new_img(oobj parent, const struct o_img *img)
{
    if (!img || !img->data) {
        o_log_warn_s(__func__, "failed to create RTex from img");
        // return invalid RTex and not NULL, because NULL is the back buffer...
        return RTex_new_ex(parent, NULL, 0, 0, R_FORMAT_RGBA_U8, R_FORMAT_RGBA_U8);
    }
    assert(img->channels>0 && img->channels<o_img_ENUM_MAX);
    ou32 format = M_FORMAT_X(img->channels, M_FORMAT__PRIMITIVE_U8);
    RTex *self = RTex_new_ex(parent, img->data, img->cols, img->rows, format, format);
    return self;
}


RTex *RTex_new_mat(oobj parent, oobj mat, ou32 format)
{
    MMat_assert(mat);
    ou32 mat_format = MMat_format(mat);
    oobj container = NULL;

    if (mat_format == M_FORMAT_1_UR8) {
        // indexed mats silently changed to normalized
        mat_format = M_FORMAT_1_U8;
    }

    // cast to a valid format or make packed (due to cast to the same format)
    if (!r_format_valid(mat_format) || !MMat_packed(mat)) {
        // needs a cast
        ou32 new_format = r_format_valid_make(mat_format);
        if (new_format != mat_format) {
            o_log_warn_s(__func__, "need a cast due to an invalid format: %x, replacing with %x", mat_format, new_format);
        } else {
            o_log_warn_s(__func__, "need a cast case it was not packed");
        }
        mat = MMat_cast_try(mat, new_format).o;
        container = mat;
        if (!mat) {
            o_log_warn_s(__func__, "cast failed");
            return RTex_new_ex(parent, NULL, 0, 0, R_FORMAT_RGBA_U8, R_FORMAT_RGBA_U8);
        }
        mat_format = new_format;
    }

    if (format == R_FORMAT_KEEP) {
        format = mat_format;
    }
    if (!r_format_valid(format)) {
        ou32 new_format = r_format_valid_make(format);
        o_log_warn_s(__func__, "got an invalid format: %x, replacing with %x", format, new_format);
    }

    RTex *res = RTex_new_ex(parent, MMat_data(mat), m_2(MMat_size_int(mat)), mat_format, format);

    o_del(container);
    return res;
}


RTex *RTex_new_file(oobj parent, const char *file)
{
    struct o_img image = o_img_new_file(parent, file, o_img_RGBA);
    if (!image.data) {
        o_log_warn_s(__func__, "failed to create RTex from file: %s", file);
        // return invalid RTex and not NULL, because NULL is the back buffer...
        return RTex_new_ex(parent, NULL, 0, 0, R_FORMAT_RGBA_U8, R_FORMAT_RGBA_U8);
    }
    RTex *self = RTex_new_img(parent, &image);
    o_img_free(&image);
    return self;
}

RTex *RTex_new_blit_back(oobj parent)
{
    RTex *self = RTex_new(parent, NULL, m_2(r_back_size_int()));
    RTex_blit_back(self, RTex_filter_NEAREST);
    return self;
}


RTex *RTex_new_kernel_buffer(oobj parent, int cols, int rows, const float *kernel, ou32 format)
{
    RTex *self = NULL;
    if (format == R_FORMAT_R_U8 || format == R_FORMAT_R_F32) {
        self = RTex_new_ex(parent, kernel, cols, rows, format, R_FORMAT_R_F32);
    } else if (format == R_FORMAT_RGBA_U8 || format == R_FORMAT_RGBA_F32) {
        RTex *rgb = RTex_new_ex(parent, NULL, cols, rows, R_FORMAT_R_F32, R_FORMAT_R_F32);
        RTex_clear_full(rgb, R_WHITE);
        RTex *alpha = RTex_new_ex(parent, kernel, cols, rows,
                                  format == R_FORMAT_RGBA_U8 ? R_FORMAT_R_U8 : R_FORMAT_R_F32, R_FORMAT_R_F32);
        self = RTex_channels_merge(rgb, rgb, rgb, alpha, format);
        o_move(self, parent);
        o_del(rgb);
        o_del(alpha);
    }
    return self;
}

RTex *RTex_new_kernel(oobj parent, int cols, int rows, float set, bool plus, float normalize, ou32 format)
{
    cols = o_max(cols, 1);
    rows = o_max(rows, 1);

    float *buffer = o_new(parent, float, cols*rows);
    m_kernel_2d(buffer, set, plus, normalize, cols, rows);
    RTex *self = RTex_new_kernel_buffer(parent, cols, rows, buffer, format);
    o_free(parent, buffer);
    return self;
}

RTex *RTex_new_kernel_gauss(oobj parent, int cols, int rows, vec2 sigma, float normalize, ou32 format)
{
    cols = o_max(cols, 1);
    rows = o_max(rows, 1);

    float *buffer = o_new(parent, float, cols*rows);
    m_kernel_2d_gauss(buffer, sigma, normalize, cols, rows);
    RTex *self = RTex_new_kernel_buffer(parent, cols, rows, buffer, format);
    o_free(parent, buffer);
    return self;
}

RTex *RTex_new_kernel_ellipse(oobj parent, int cols, int rows, float norm_radius, float normalize, ou32 format)
{
    cols = o_max(cols, 1);
    rows = o_max(rows, 1);

    float *buffer = o_new(parent, float, cols*rows);
    m_kernel_2d_ellipse(buffer, norm_radius, normalize, cols, rows);
    RTex *self = RTex_new_kernel_buffer(parent, cols, rows, buffer, format);
    o_free(parent, buffer);
    return self;
}


#ifdef MIA_OPTION_TTF
RTex *RTex_new_ttf(oobj parent, const char *file, float size, const char *text)
{
    oobj ttf = OTtf_new(parent, file, size);
    struct o_img img = OTtf_draw(ttf, text);
    RTex *self = RTex_new_img(parent, &img);
    o_del(ttf);
    return self;
}
#endif


//
// virtual implementations:
//

void RTex__v_del(oobj obj)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;

    // deletes fbo, if available
    RTex_use_done(self);
    // safe for 0
    glDeleteTextures(1, &self->gl_tex);

    OObj__v_del(obj);
}


//
// object functions:
//

ivec4 RTex_viewport(oobj obj)
{
    if (!obj) {
        ivec4 vp = RTex__L.back_gl_viewport;
        float height = r_back_size_int().y;
        vp.y = height - (vp.y + vp.v3);
        return vp;
    }
    OObj_assert(obj, RTex);
    RTex *self = obj;
    // rendering to texture is mirrored in y anyway, so we use our ltwh instead of the gl_viewport (lbwh)
    return self->viewport;
}

ivec4 RTex_viewport_set(oobj obj, ivec4 set)
{
    RTex__L.bound_valid = false;
    if (!obj) {
        RTex__L.back_gl_viewport = set;
        float height = r_back_size_int().y;
        RTex__L.back_gl_viewport.y = height - (set.y + set.v3);
        return set;
    }
    OObj_assert(obj, RTex);
    RTex *self = obj;
    // rendering to texture is mirrored in y anyway, so we use our ltwh instead of the gl_viewport (lbwh)
    self->viewport = set;
    return set;
}


ivec4 RTex_viewport_set_rect(oobj obj, vec4 rect)
{
    return RTex_viewport_set(obj, r_proj_rect_to_viewport(RTex_proj(obj), rect));
}

ivec4 RTex_viewport_set_pose_aa(oobj obj, mat4 pose_aa)
{
    // according to u/pose.h
    vec4 rect = vec4_(pose_aa.m30, pose_aa.m31, pose_aa.m00, pose_aa.m11);
    return RTex_viewport_set_rect(obj, rect);
}

void RTex_wrap_set(oobj obj, enum RTex_wrap_modes mode)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    self->wrap_mode = mode;
    glBindTexture(GL_TEXTURE_2D, self->gl_tex);
    if (self->wrap_mode == RTex_wrap_CLAMP) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RTex_filter_set(oobj obj, enum RTex_filter_modes mode)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    self->filter_mode = mode;
    glBindTexture(GL_TEXTURE_2D, self->gl_tex);
    if (self->filter_mode == RTex_filter_LINEAR) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    } else if (self->filter_mode == RTex_filter_NEAREST) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}


oobj RTex_cam(oobj obj)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    oobj cam = RCam_new(self, true, vec2_(-1));
    RCam_update_ex(cam, self->size.x, self->size.y);
    return cam;
}

//
// fbo stuff
//


void RTex_use(oobj obj, int requested_draw_buffers)
{
    if (!obj) {
        if (requested_draw_buffers != 1) {
            o_log_warn_s(__func__, "using back buffer, but requested_draw_buffers!=1: %d", requested_draw_buffers);
        }
        if (!RTex__L.bound_valid || RTex__L.bound_tex != NULL) {
            RTex__L.bound_valid = true;
            RTex__L.bound_tex = NULL;
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, RTex__L.back_fbo);

            // gl deprecated using plain GL_BACK, but gles does not support GL_BACK_LEFT
#ifdef MIA_OPTION_GLES
            GLenum back_buffer = GL_BACK;
#else
            GLenum back_buffer = GL_BACK_LEFT;
#endif
            glDrawBuffers(1, (GLenum[]){back_buffer});
            glViewport(m_4(RTex__L.back_gl_viewport));
            r_error_check("use back");
        }
        return;
    }

    OObj_assert(obj, RTex);
    RTex *self = obj;

    if (!RTex__L.bound_valid || RTex__L.bound_tex != obj) {
        // only valid for multiple runs if used without attachments
        // once had a bug where drawing with multiple draw calls in a frame 
        // resulted in only the first beeing rendered
        // to the attachment.
        // In between updates fixed it.
        RTex__L.bound_valid = requested_draw_buffers == 1;

        RTex__L.bound_tex = obj;
        update_fbo(self, requested_draw_buffers); // Ensure the framebuffer is created
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, self->fbo->gl_fbo);

        // rendering to texture is mirrored in y anyways, so we use our ltwh instead of the gl_viewport (lbwh)
        glViewport(m_4(self->viewport));
        r_error_check("use tex");
    }
}


void RTex_use_done(oobj obj)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    if (!self->fbo) {
        return;
    }
    glDeleteFramebuffers(1, &self->fbo->gl_fbo);
    o_free(self, self->fbo);
    self->fbo = NULL;
}

void RTex_get_ex(oobj obj, void *out_buffer, ou32 format)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    if (format == R_FORMAT_KEEP) {
        format = self->format;
    }
    update_fbo(self, 1); // Ensure the framebuffer is created
    glBindTexture(GL_TEXTURE_2D, self->gl_tex);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, self->fbo->gl_fbo);
    // default may be that new rows are not byte aligned, so 1 for byte aligned if format_size is < 4 (R-RGB U8)
    glPixelStorei(GL_PACK_ALIGNMENT, m_format_size(self->format)<4 ? 1 : 4);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, self->size.x, self->size.y,
                 format_channels(format), format_size(format),
                 out_buffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void RTex_set_ex(oobj obj, const void *buffer, ou32 format)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    if (format == R_FORMAT_KEEP) {
        format = self->format;
    }
    glBindTexture(GL_TEXTURE_2D, self->gl_tex);
    // default may be that new rows are not byte aligned, so 1 for byte aligned if format_size is < 4 (R-RGB U8)
    glPixelStorei(GL_UNPACK_ALIGNMENT, m_format_size(self->format)<4 ? 1 : 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0,
                    0, 0, self->size.x, self->size.y,
                    format_channels(format), format_size(format),
                    buffer);
    glBindTexture(GL_TEXTURE_2D, 0);
}


oobj RTex_mat(oobj obj, ou32 format)
{
    assert(RTex_valid(obj));
    RTex *self = obj;
    oobj mat = MMat_new(self, NULL, m_2(self->size), self->format);
    RTex_get_ex(self, MMat_data(mat), self->format);
    return mat;
}

void RTex_get_into_mat(oobj obj, oobj mat)
{
    assert(RTex_valid(obj));
    RTex *self = obj;
    MMat_assert(mat);
    assert(ivec2_equals_v(self->size, MMat_size_int(mat)));

    bool need_cast = !MMat_packed(mat) || !r_format_valid(MMat_format(mat));

    if (!need_cast) {
        RTex_get_ex(self, MMat_data(mat), MMat_format(mat));
        return;
    }

    o_log_warn_s(__func__, "needs an internal inbetween cast!");

    ou32 format = r_format_valid_make(MMat_format(mat));
    oobj cast = MMat_new(mat, NULL, m_2(self->size), format);
    RTex_get_ex(self, MMat_data(cast), MMat_format(cast));
    MMat_cast_into(cast, mat);
    o_del(cast);
}

void RTex_set_from_mat(oobj obj, oobj mat)
{
    assert(RTex_valid(obj));
    RTex *self = obj;
    MMat_assert(mat);
    assert(ivec2_equals_v(self->size, MMat_size_int(mat)));

    bool need_cast = !MMat_packed(mat) || !r_format_valid(MMat_format(mat));

    if (!need_cast) {
        RTex_set_ex(self, MMat_data(mat), MMat_format(mat));
        return;
    }

    o_log_warn_s(__func__, "needs an internal inbetween cast!");

    ou32 format = r_format_valid_make(MMat_format(mat));
    oobj cast = MMat_cast(mat, format);
    RTex_set_ex(self, MMat_data(cast), MMat_format(cast));
    o_del(cast);
}

bool RTex_write_file(oobj obj, const char *file)
{
    ivec2 size = RTex_size_int(obj);
    struct o_img img = o_img_new(obj, m_2(size), o_img_RGBA);
    RTex_get(obj, img.data);
    bool saved = o_img_write_file(img, file);
    o_img_free(&img);
    return saved;
}

struct r_proj *RTex_proj(oobj obj)
{
    if (!obj) {
        return r_back_proj();
    }
    OObj_assert(obj, RTex);
    RTex *self = obj;
    update_fbo(self, 1); // Ensure the framebuffer is created
    return &self->fbo->proj;
}

struct oobj_opt RTex_attach(oobj obj)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    return OPtr_get(self->attached_tex_ptr);
}

oobj RTex_attach_set(oobj obj, oobj opt_tex_attachment)
{
    OObj_assert(obj, RTex);
    RTex *self = obj;
    OPtr_set(self->attached_tex_ptr, opt_tex_attachment);
    RTex__L.bound_valid = false;
    return opt_tex_attachment;
}

//
// render stuff
//

void RTex_clear_full(oobj obj, vec4 clear_color)
{
    RTex_use(obj, 1);

    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);
    glClear(GL_COLOR_BUFFER_BIT);

    r_error_check("clear");
}

void RTex_clear_ex(oobj obj, vec4 clear_color, bool blend)
{
    ivec2 size = RTex_size_int(obj);
    // tmp 1:1 projection for the texture, reversed at function end
    struct r_proj prev = *RTex_proj(obj);
    *RTex_proj(obj) = r_proj_new(size, -1, vec2_(-1), true);

    // quad to override the full texture
    struct r_quad q = r_quad_new(0, 0);
    // pose is 3x as big as the RTex, so it's definitely large enough to fit
    q.pose = (mat4){
        {
            size.x * 3, 0, 0, 0,
            0, size.y * 3, 0, 0,
            0, 0, 1, 0,
            -size.x, -size.y, 0, 1
        }
    };
    q.fx = clear_color;

    RShaderQuad_tex_set(RTex__L.s_quad, r_tex_white(), false);
    RShader_blend_set(RTex__L.s_quad, blend);
    RShader_program_set(RTex__L.s_quad, r_program_Quad_rgba());
    RTex_quads(obj, RTex__L.s_quad, &q, 1);
    *RTex_proj(obj) = prev;
}

void RTex_shader_ex(oobj obj, oobj shader, oobj buffer, int opt_num_rendered, const struct r_proj *opt_proj)
{
    RShader_render_ex(shader, buffer, obj, opt_num_rendered, opt_proj);
}

void RTex_ro_ex(oobj obj, oobj ro, const struct r_proj *opt_proj, bool update)
{
    RObj_render_ex(ro, obj, opt_proj, update);
}

void RTex_quads(oobj obj, oobj shader, const struct r_quad *quads, int n)
{
    RBuffer_update(RTex__L.b_quad, quads, n);
    RTex_shader(obj, shader, RTex__L.b_quad);
}

void RTex_boxes(oobj obj, oobj shader, const struct r_box *boxes, int n)
{
    RBuffer_update(RTex__L.b_box, boxes, n);
    RTex_shader(obj, shader, RTex__L.b_box);
}
