#include "m/MMat_draw.h"
#include "m/MMat_blit.h"
#include "o/OObjRoot.h"
#include "o/OTask.h"
#include "o/OArray.h"
#include "m/int.h"
#include "m/flt.h"
#include "m/byte.h"
#include "m/discr.h"


#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"


#define MMAT_DRAW_DAB_ARRAY_INJECT "MMat_draw_dab_array"


static MMat *mmat_white_pixel(oobj parent)
{
    bvec4 white = bvec4_(255);
    return MMat_new(parent, &white, 1, 1, M_FORMAT_RGBA_U8);
}

static OArray *mmat_cached_dab_array(oobj obj)
{
    MMat_assert(obj);
    struct oobj_opt inject = OObj_find(obj, OArray, MMAT_DRAW_DAB_ARRAY_INJECT, 0);
    if(inject.o) {
        OArray_clear(inject.o);
        return inject.o;
    }

    oobj array = OArray_new_dyn(obj, NULL, sizeof(vec2), 0, 256);
    OObj_name_set(array, MMAT_DRAW_DAB_ARRAY_INJECT);
    return array;
}

//
// public
//

void MMat_clear_rect_x(oobj obj, m_ivec4 rect, const void *clear_data)
{
    MMat_assert(obj);
    MMat *self = obj;
    ou32 format = self->format;
    osize format_size = m_format_size(format);

    int left = o_clamp(rect.left, 0, self->size.x-1);
    int top = o_clamp(rect.top, 0, self->size.y-1);
    int right_end = o_clamp(rect.left + rect.width, 0, self->size.x);
    int bottom_end = o_clamp(rect.top + rect.height, 0, self->size.y);

    for (int row=top; row<bottom_end; row++) {
        for (int col=left; col<right_end; col++) {
            void *restrict data = MMat_at_raw(self, col, row);
            o_memcpy(data, clear_data, format_size, 1);
        }
    }
}


void MMat_clear_rect_vec4(oobj obj, m_ivec4 rect, m_vec4 clear_color)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(m_format_size(self->format) <= M_FORMAT_MAX_SIZE);
    obyte clear_data[M_FORMAT_MAX_SIZE] = {0};
    m_format_cast_from_vec4(clear_data, self->format, clear_color);
    MMat_clear_rect_x(obj, rect, clear_data);
}


void MMat_border_into(oobj obj, oobj into, m_ivec4 border_ltrb, m_vec4 border_color)
{
    MMat_assert(obj);
    MMat *self = obj;
    MMat_assert(into);
    MMat *res = into;
    ivec2 needed_size = ivec2_add_v(self->size, ivec2_(border_ltrb.v0 + border_ltrb.v2, border_ltrb.v1 + border_ltrb.v3));
    assert(ivec2_equals_v(res->size, needed_size));

    // top and bottom rects use full width, so left and right do >not< need to use full height
    ivec4 border_top = ivec4_(0, 0, needed_size.x, border_ltrb.v1);
    ivec4 border_bottom = ivec4_(0, needed_size.y-border_ltrb.v3, needed_size.x, border_ltrb.v3);
    ivec4 border_left = ivec4_(0, border_ltrb.v1, border_ltrb.v0,
        needed_size.y - border_ltrb.v1 - border_ltrb.v3);
    ivec4 border_right = ivec4_(needed_size.x-border_ltrb.v2, border_ltrb.v1, border_ltrb.v2,
        needed_size.y - border_ltrb.v1 - border_ltrb.v3);

    MMat_clear_rect_vec4(res, border_top, border_color);
    MMat_clear_rect_vec4(res, border_left, border_color);
    MMat_clear_rect_vec4(res, border_right, border_color);
    MMat_clear_rect_vec4(res, border_bottom, border_color);

    MMat_blit(res, obj, border_ltrb.v0, border_ltrb.v1);
}

struct oobj_opt MMat_border_try(oobj obj, m_ivec4 border_ltrb, m_vec4 border_color, ou32 format)
{
    MMat_assert(obj);
    MMat *self = obj;
    if (format == M_FORMAT_KEEP) {
        format = self->format;
    }
    border_ltrb = ivec4_max(border_ltrb, 0);
    ivec2 size = ivec2_add_v(self->size, ivec2_(border_ltrb.v0 + border_ltrb.v2, border_ltrb.v1 + border_ltrb.v3));
    MMat *res = MMat_new_ex(obj, NULL, NULL, true, format, size, ivec2_(0));
    if (!MMat_valid(res)) {
        o_del(res);
        return oobj_opt(NULL);
    }
    MMat_border_into(self, res, border_ltrb, border_color);
    return oobj_opt(res);
}


void MMat_collage_get_grid_size(const oobj *sources, int cols, int rows, int *out_col_sizes, int *out_row_sizes)
{
    assert(cols>=1 && rows>=1);
    for (int c=0; c<cols; c++) {
        int col_size=0;
        for (int r=0; r<rows; r++) {
            oobj src = sources[r*cols+c];
            if (!src) {
                continue;
            }
            col_size = o_max(col_size, MMat_size_int(src).x);
        }
        out_col_sizes[c] = col_size;
    }
    for (int r=0; r<rows; r++) {
        int row_size=0;
        for (int c=0; c<cols; c++) {
            oobj src = sources[r*cols+c];
            if (!src) {
                continue;
            }
            row_size = o_max(row_size, MMat_size_int(src).y);
        }
       out_row_sizes[r] = row_size;
    }
}

void MMat_collage_into(oobj into, const oobj *sources, int cols, int rows,
    m_ivec2 spacing, m_ivec4 border_ltrb, m_vec4 bg_color, bool blend)
{
    MMat_assert(into);
    MMat *res = into;

    assert(cols>=1 && rows>=1);
    spacing = ivec2_max(spacing, 0);
    border_ltrb = ivec4_max(border_ltrb, 0);
    ivec2 needed_size = {{spacing.x * cols-1, spacing.y * rows-1}};
    needed_size = ivec2_add_v(needed_size, ivec2_(border_ltrb.v0 + border_ltrb.v2, border_ltrb.v1 + border_ltrb.v3));

    int *col_sizes = o_new(into, int, cols + rows);
    int *row_sizes = col_sizes + cols;
    MMat_collage_get_grid_size(sources, cols, rows, col_sizes, row_sizes);
    needed_size.x += ivecn_sum(col_sizes, cols);
    needed_size.y += ivecn_sum(row_sizes, rows);

    assert(ivec2_equals_v(res->size, needed_size));

    // full clear, will produce overdraw, but so we dont need to calc for every smaller img blit to fill rects here
    MMat_clear_vec4(res, bg_color);

    ivec2 pos = ivec2_(border_ltrb.v0, border_ltrb.v1);

    // blit or blend, both use the same fn parameters, etc.
    void (*blitblend_fn)(oobj, oobj, int, int) = blend ? MMat_blend : MMat_blit;

    for (int r=0; r<rows; r++) {
        ivec2 item_pos = pos;
        for (int c=0; c<cols; c++) {
            oobj src = sources[r*cols+c];
            if (src) {
                blitblend_fn(res, src, item_pos.x, item_pos.y);
            }
            item_pos.x += col_sizes[c] + spacing.x;
        }
        pos.y += row_sizes[r] + spacing.y;
    }

    // frees both col and row_sizes
    o_free(into, col_sizes);
}

struct oobj_opt MMat_collage_try(oobj parent, const oobj *sources, int cols, int rows,
    m_ivec2 spacing, m_ivec4 border_ltrb, m_vec4 bg_color, bool blend, ou32 format)
{
    if (format == M_FORMAT_KEEP) {
        for (int i=0; i<cols*rows; i++) {
            if (sources[i]) {
                format = MMat_format(sources[i]);
                break;
            }
        }
        assert(format != M_FORMAT_KEEP);
    }
    assert(cols>=1 && rows>=1);
    spacing = ivec2_max(spacing, 0);
    border_ltrb = ivec4_max(border_ltrb, 0);

    ivec2 size = {{spacing.x * cols-1, spacing.y * rows-1}};
    size = ivec2_add_v(size, ivec2_(border_ltrb.v0 + border_ltrb.v2, border_ltrb.v1 + border_ltrb.v3));

    int *col_sizes = o_new(parent, int, cols + rows);
    int *row_sizes = col_sizes + cols;
    MMat_collage_get_grid_size(sources, cols, rows, col_sizes, row_sizes);
    size.x += ivecn_sum(col_sizes, cols);
    size.y += ivecn_sum(row_sizes, rows);
    o_free(parent, col_sizes);

    MMat *res = MMat_new_ex(parent, NULL, NULL, true, format, size, ivec2_(0));
    if (!MMat_valid(res)) {
        o_del(res);
        return oobj_opt(NULL);
    }
    MMat_collage_into(res, sources, cols, rows, spacing, border_ltrb, bg_color, blend);
    return oobj_opt(res);
}




void MMat_replace_x(oobj obj, const void *restrict color_from, const void *restrict color_to)
{
    MMat_assert(obj);
    MMat *self = obj;
    int format_size = m_format_size(self->format);
    osize num = MMat_num(self);
    for (osize i=0; i<num; i++) {
        void *restrict data = MMat_at_idx_raw(self, i);
        if (o_equals(data, color_from, format_size, 1)) {
            o_memcpy(data, color_to, format_size, 1);
        }
    }
}

void MMat_replace(oobj obj, m_vec4 color_from, m_vec4 color_to)
{

    MMat_assert(obj);
    MMat *self = obj;
    int format_size = m_format_size(self->format);
    assert(format_size <= M_FORMAT_MAX_SIZE);
    obyte from[M_FORMAT_MAX_SIZE] = {0};
    obyte to[M_FORMAT_MAX_SIZE] = {0};
    m_format_cast_from_vec4(from, self->format, color_from);
    m_format_cast_from_vec4(to, self->format, color_to);
    MMat_replace_x(obj, from, to);
}


struct fill_context {
    MMat *img;
    // ivec3 array
    OArray *pos_stack;
    int format_size;
    obyte check_color[M_FORMAT_MAX_SIZE];
    obyte fill_color[M_FORMAT_MAX_SIZE];
    bool mode8;
    int step_runs;
};

static float fill_task(oobj task, int run)
{
    struct fill_context *C = o_user(task);
    const int step_runs = C->step_runs;
    
    ivec2 size = C->img->size;
    
    for(int i=0; i<step_runs && OArray_num(C->pos_stack)>0; i++) {
        ivec3 pop;
        OArray_pop(C->pos_stack, &pop);
        
        // bounds check
        if(pop.x<0 || pop.x>=size.x || pop.y<0 || pop.y>=size.y) {
            continue;
        }
        
        obyte *restrict pop_at = MMat_at_raw(C->img, pop.x, pop.y);
        
        // color check
        if(!o_equals(pop_at, C->check_color, C->format_size, 1)) {
            continue;
        }
        
        // draw color
        o_memcpy(pop_at, C->fill_color, C->format_size, 1);
       
        // pushing into the fill pos stack
        // the last pushed is the first evaluated
        // so in the best case its row major
        
        // .z == direction
        // 0 init
        // 1234 l  t  r  b
        // 5678 lt rt rb lb
        if(C->mode8) {
            if(pop.z!=6) {
                OArray_push(C->pos_stack, &ivec3_(pop.x-1, pop.y+1, 8));
            }
            if(pop.z!=5) {
                OArray_push(C->pos_stack, &ivec3_(pop.x+1, pop.y+1, 7));
            }
            if(pop.z!=8) {
                OArray_push(C->pos_stack, &ivec3_(pop.x+1, pop.y-1, 6));
            }
            if(pop.z!=7) {
                OArray_push(C->pos_stack, &ivec3_(pop.x-1, pop.y-1, 5));
            }
        }
        
        // push mode 4
        if(pop.z!=2) {
            OArray_push(C->pos_stack, &ivec3_(pop.x, pop.y+1, 4));
        }
        if(pop.z!=1) {
            OArray_push(C->pos_stack, &ivec3_(pop.x+1, pop.y, 3));
        }
        if(pop.z!=4) {
            OArray_push(C->pos_stack, &ivec3_(pop.x, pop.y-1, 2));
        }
        if(pop.z!=3) {
            OArray_push(C->pos_stack, &ivec3_(pop.x-1, pop.y, 1));
        }
       
    }
    
    return OArray_num(C->pos_stack)>0? -1 : OTask_FINISH;
}


oobj MMat_fill_task_x(oobj obj, int x, int y, bool mode8, const void *fill_color, int opt_step_runs)
{
    MMat_assert(obj);
    MMat *self = obj;
    
    int format_size = m_format_size(self->format);
    assert(format_size <= M_FORMAT_MAX_SIZE);
    
    if(x<0 || x>=self->size.x || y<0 || y>=self->size.y) {
        o_log_warn_s(__func__, "out of bounds");
        return OTask_new_finished(self);
    }
    
    // bounds already checked
    const obyte *picked_color = MMat_at_raw(self, x, y);
    
    if(o_equals(picked_color, fill_color, format_size, 1)) {
        // done, nothing to do
        return OTask_new_finished(self);
    }
    
    oobj task = OTask_new(self, fill_task, true);
    struct fill_context *C = o_user_new0(task, *C, 1);
    C->img = self;
    C->pos_stack = OArray_new_dyn(task, NULL, sizeof(ivec3), 0, 1024);
    OArray_push(C->pos_stack, &ivec3_(x, y, 0));
    C->format_size = format_size;
    o_memcpy(C->check_color, picked_color, format_size, 1);
    o_memcpy(C->fill_color, fill_color, format_size, 1);
    C->mode8 = mode8;
    C->step_runs = opt_step_runs>0? opt_step_runs : 1024;
    return task;
}

void MMat_fill_x(oobj obj, int x, int y, bool mode8, const void *fill_color)
{
    oobj task = MMat_fill_task_x(obj, x, y, mode8, fill_color, 0);
    OTask_run_blocking(task);
    o_del(task);
}

void MMat_fill_vec4(oobj obj, int x, int y, bool mode8, m_vec4 fill_color)
{
    MMat_assert(obj);
    MMat *self = obj;
    assert(m_format_size(self->format) <= M_FORMAT_MAX_SIZE);
    obyte fill_data[M_FORMAT_MAX_SIZE] = {0};
    m_format_cast_from_vec4(fill_data, self->format, fill_color);
    MMat_fill_x(obj, x, y, mode8, fill_data);
}


//
// draw functions
//


void MMat_draw_dabs(oobj obj, m_vec4 color, const m_vec2 *dabs, int num, oobj opt_brush)
{
    MMat_assert(obj);
    MMat *self = obj;

    oobj white_pixel = NULL;
    if (!opt_brush) {
        white_pixel = opt_brush = mmat_white_pixel(self);
    }

    vec2 brush_size = MMat_size(opt_brush);
    vec2 offset = vec2_sub(brush_size, 0.1);
    offset = vec2_floor(vec2_div(offset, 2));
    for (int i = 0; i < num; i++) {
        vec2 pos = vec2_sub_v(dabs[i], offset);
        MMat_blend_dab(self, opt_brush, pos.x, pos.y, color);
    }

    o_del(white_pixel);
}

void MMat_draw_line(oobj obj, m_vec4 color, m_vec2 pos_a, m_vec2 pos_b, oobj opt_brush)
{
    oobj dab_array = mmat_cached_dab_array(obj);
    m_discr_grid_point(dab_array, pos_a);
    m_discr_grid_line(dab_array, pos_a, pos_b);
    MMat_draw_dabs(obj, color, OArray_data(dab_array, vec2), OArray_num(dab_array), opt_brush);
}

void MMat_draw_box(oobj obj, m_vec4 color, m_vec2 pos_a, m_vec2 pos_b, oobj opt_brush)
{
    oobj dab_array = mmat_cached_dab_array(obj);
    m_discr_grid_box(dab_array, pos_a, pos_b);
    MMat_draw_dabs(obj, color, OArray_data(dab_array, vec2), OArray_num(dab_array), opt_brush);
}

void MMat_draw_circle(oobj obj, m_vec4 color, m_vec2 center, float radius, oobj opt_brush)
{
    oobj dab_array = mmat_cached_dab_array(obj);
    m_discr_grid_circle(dab_array, center, radius);
    MMat_draw_dabs(obj, color, OArray_data(dab_array, vec2), OArray_num(dab_array), opt_brush);
}

void MMat_draw_circle_bow(oobj obj, m_vec4 color, m_vec2 center, float radius, float a_rad, float b_rad, oobj opt_brush)
{
    oobj dab_array = mmat_cached_dab_array(obj);
    m_discr_grid_circle_bow(dab_array, center, radius, a_rad, a_rad);
    m_discr_grid_circle_bow(dab_array, center, radius, a_rad, b_rad);
    MMat_draw_dabs(obj, color, OArray_data(dab_array, vec2), OArray_num(dab_array), opt_brush);
}