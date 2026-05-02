#include "m/MMat_blit.h"
#include "o/OTask.h"
#include "m/flt.h"
#include "m/int.h"
#include "m/byte.h"


#undef O_LOG_LIB
#define O_LOG_LIB "m"

#include "o/log.h"



static ivec4 blitblend_rect(MMat *self, MMat *blitblend, int x, int y, m_ivec4 img_rect)
{
    // auto set and clamp img_rect to be valid
    if (img_rect.width <= 0) {
        img_rect.left = 0;
        img_rect.width = blitblend->size.x;
    }
    if (img_rect.height <= 0) {
        img_rect.top = 0;
        img_rect.height = blitblend->size.y;
    }
    img_rect.left = o_clamp(img_rect.left, 0, blitblend->size.x-1);
    img_rect.top = o_clamp(img_rect.top, 0, blitblend->size.y-1);
    img_rect.width = o_clamp(img_rect.width, 1, blitblend->size.x-img_rect.left);
    img_rect.height = o_clamp(img_rect.height, 1, blitblend->size.y-img_rect.top);

    // calc window of really cloned values

    // combined end point in dst
    int x2 = o_min(x + img_rect.width, self->size.x);
    int y2 = o_min(y + img_rect.height, self->size.y);

    // "boolean and" of the aabb rects
    int res_c = o_max(img_rect.left, -x);
    int res_r = o_max(img_rect.top, -y);
    int res_c2 = o_max(img_rect.left, x2 - x);
    int res_r2 = o_max(img_rect.top, y2 - y);
    return ivec4_(res_c, res_r, res_c2, res_r2);
}


void MMat_blit_rect(oobj obj, oobj img, int x, int y, m_ivec4 img_rect)
{
    MMat_assert(obj);
    MMat *self = obj;
    OObj_assert(img, MMat);
    MMat *read = img;

    assert(self->data_root != read->data_root);

    ivec4 res_rect = blitblend_rect(self, read, x, y, img_rect);
    int res_c = res_rect.v0;
    int res_r = res_rect.v1;
    int res_c2 = res_rect.v2;
    int res_r2 = res_rect.v3;


    if (res_c>=res_c2 || res_r>=res_r2) {
        // nothing to blit
        return;
    }

    // fast path: format and col stride matches (o_memcpy the whole row)
    if (self->format == read->format && self->stride.x == read->stride.x) {
        int c = 0;
        int dst_c = x;
        int num = res_c2 - res_c;
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            void *restrict src = MMat_at_raw(read, c, r);
            void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
            o_memcpy(dst, src, m_format_size(self->format), num);
        }
        return;
    }
    // fast path: format matches (but stride not, so o_memcpy for each pixel)
    if (self->format == read->format) {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                void *restrict src = MMat_at_raw(read, c, r);
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                o_memcpy(dst, src, m_format_size(self->format), 1);
            }
        }
        return;
    }

    // generic format missmatch
    {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                void *restrict src = MMat_at_raw(read, c, r);
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                vec4 tmp = m_format_cast_to_vec4(src, read->format);
                m_format_cast_from_vec4(dst, self->format, tmp);
            }
        }
        return;
    }
}

void MMat_blend_rect(oobj obj, oobj img, int x, int y, m_ivec4 img_rect)
{
    MMat_assert(obj);
    MMat *self = obj;
    OObj_assert(img, MMat);
    MMat *read = img;

    assert(self->data_root != read->data_root);

    ivec4 res_rect = blitblend_rect(self, read, x, y, img_rect);
    int res_c = res_rect.v0;
    int res_r = res_rect.v1;
    int res_c2 = res_rect.v2;
    int res_r2 = res_rect.v3;


    if (res_c>=res_c2 || res_r>=res_r2) {
        // nothing to blit
        return;
    }

    // fast path: bvec4
    if (self->format == M_FORMAT_RGBA_U8 && read->format == M_FORMAT_RGBA_U8) {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                bvec4 *restrict src = MMat_at_raw(read, c, r);
                bvec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                dst->xyz = bvec3_mix(dst->xyz, src->xyz, src->a/255.0f);
                dst->a = o_min(255, dst->a + src->a);
            }
        }
        return;
    }
    // fast path: vec4
    if (self->format == M_FORMAT_RGBA_F32 && read->format == M_FORMAT_RGBA_F32) {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                vec4 *restrict src = MMat_at_raw(read, c, r);
                vec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                dst->xyz = vec3_mix(dst->xyz, src->xyz, src->a);
                dst->a = o_min(1.0, dst->a + src->a);
            }
        }
        return;
    }

    // generic format missmatch
    {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                void *restrict src = MMat_at_raw(read, c, r);
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                vec4 src_vec = m_format_cast_to_vec4(src, read->format);
                vec4 dst_vec = m_format_cast_to_vec4(dst, self->format);
                dst_vec.xyz = vec3_mix(dst_vec.xyz, src_vec.xyz, src_vec.a);
                dst_vec.a = o_min(1.0, dst_vec.a + src_vec.a);
                m_format_cast_from_vec4(dst, self->format, dst_vec);
            }
        }
        return;
    }
}

void MMat_resample_blit_rect(oobj obj, oobj img, m_ivec4 rect, m_vec4 img_rect, bool linear)
{
    MMat_assert(obj);
    MMat *self = obj;
    OObj_assert(img, MMat);
    MMat *read = img;

    assert(self->data_root != read->data_root);

    if (rect.width<=0 || rect.height<=0) {
        rect = MMat_rect_int(self, 0, 0);
    }
    if (img_rect.width<=0 || img_rect.height <=0) {
        img_rect = MMat_rect(img, 0, 0);
    }

    // clamp rect
    rect.xy = ivec2_max(rect.xy, 0);
    rect.zw = ivec2_min_v(rect.zw, ivec2_sub_v(self->size, rect.xy));
    

    if (rect.width<=0 || rect.height<=0) {
        // nothing to blit
        return;
    }

    // fast path: neareast and format matches (so o_memcpy for each pixel)
    if (!linear && self->format == read->format) {
        for (int r_i = 0; r_i<rect.height; r_i++) {
            int dst_r = r_i + rect.y;
            float r_t = (float) r_i / (float) (rect.height-1);
            float sample_r = img_rect.y + r_t * img_rect.height;
            int img_r = m_round(sample_r);
            img_r = o_clamp(img_r, 0, (read->size.y-1));
            
            for (int c_i=0; c_i < rect.width; c_i++) {
                int dst_c = c_i + rect.x;
                float c_t = (float) c_i / (float) (rect.width-1);
                float sample_c = img_rect.x + c_t * img_rect.width;
                int img_c = m_round(sample_c);
                img_c = o_clamp(img_c, 0, (read->size.x-1));
            
                
                void *restrict src = MMat_at_raw(read, img_c, img_r);
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                o_memcpy(dst, src, m_format_size(self->format), 1);
            }
        }
        return;
    }


    // nearest generic format missmatch
    if(!linear) {
        for (int r_i = 0; r_i<rect.height; r_i++) {
            int dst_r = r_i + rect.y;
            float r_t = (float) r_i / (float) (rect.height-1);
            float sample_r = img_rect.y + r_t * img_rect.height;
            int img_r = m_round(sample_r);
            img_r = o_clamp(img_r, 0, (read->size.y-1));
            
            for (int c_i=0; c_i < rect.width; c_i++) {
                int dst_c = c_i + rect.x;
                float c_t = (float) c_i / (float) (rect.width-1);
                float sample_c = img_rect.x + c_t * img_rect.width;
                int img_c = m_round(sample_c);
                img_c = o_clamp(img_c, 0, (read->size.x-1));
                
                void *restrict src = MMat_at_raw(read, img_c, img_r);
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                vec4 tmp = m_format_cast_to_vec4(src, read->format);
                m_format_cast_from_vec4(dst, self->format, tmp);
            }
        }
        return;
    }
    
    // fast path linear bvec4
    if(self->format == M_FORMAT_RGBA_U8 && read->format == M_FORMAT_RGBA_U8) {
        for (int r_i = 0; r_i<rect.height; r_i++) {
            int dst_r = r_i + rect.y;
            float r_t = (float) r_i / (float) (rect.height-1);
            float sample_r = img_rect.y + r_t * img_rect.height;
            int img_r_a = m_floor(sample_r);
            int img_r_b = m_ceil(sample_r);
            float img_r_t = m_fract(sample_r);
            img_r_a = o_clamp(img_r_a, 0, (read->size.y-1));
            img_r_b = o_clamp(img_r_b, 0, (read->size.y-1));
            
            for (int c_i=0; c_i < rect.width; c_i++) {
                int dst_c = c_i + rect.x;
                float c_t = (float) c_i / (float) (rect.width-1);
                float sample_c = img_rect.x + c_t * img_rect.width;
                int img_c_a = m_floor(sample_c);
                int img_c_b = m_ceil(sample_c);
                float img_c_t = m_fract(sample_c);
                img_c_a = o_clamp(img_c_a, 0, (read->size.x-1));
                img_c_b = o_clamp(img_c_b, 0, (read->size.x-1));
            
                
                bvec4 *src_aa = MMat_at_raw(read, img_c_a, img_r_a);
                bvec4 *src_ba = MMat_at_raw(read, img_c_b, img_r_a);
                bvec4 *src_ab = MMat_at_raw(read, img_c_a, img_r_b);
                bvec4 *src_bb = MMat_at_raw(read, img_c_b, img_r_b);
                
                bvec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                
                vec4 tmp_aa = vec4_cast_byte(src_aa->v);
                vec4 tmp_ba = vec4_cast_byte(src_ba->v);
                vec4 tmp_ab = vec4_cast_byte(src_ab->v);
                vec4 tmp_bb = vec4_cast_byte(src_bb->v);
                
                vec4 tmp_a = vec4_mix(tmp_aa, tmp_ba, img_c_t);
                vec4 tmp_b = vec4_mix(tmp_ab, tmp_bb, img_c_t);
                vec4 tmp = vec4_mix(tmp_a, tmp_b, img_r_t);       
                
                *dst = bvec4_cast_float(tmp.v);
            }
        }
        return;
    }

    // fast path linear vec4 -> bvec4
    if(self->format == M_FORMAT_RGBA_U8 && read->format == M_FORMAT_RGBA_F32) {
        for (int r_i = 0; r_i<rect.height; r_i++) {
            int dst_r = r_i + rect.y;
            float r_t = (float) r_i / (float) (rect.height-1);
            float sample_r = img_rect.y + r_t * img_rect.height;
            int img_r_a = m_floor(sample_r);
            int img_r_b = m_ceil(sample_r);
            float img_r_t = m_fract(sample_r);
            img_r_a = o_clamp(img_r_a, 0, (read->size.y-1));
            img_r_b = o_clamp(img_r_b, 0, (read->size.y-1));

            for (int c_i=0; c_i < rect.width; c_i++) {
                int dst_c = c_i + rect.x;
                float c_t = (float) c_i / (float) (rect.width-1);
                float sample_c = img_rect.x + c_t * img_rect.width;
                int img_c_a = m_floor(sample_c);
                int img_c_b = m_ceil(sample_c);
                float img_c_t = m_fract(sample_c);
                img_c_a = o_clamp(img_c_a, 0, (read->size.x-1));
                img_c_b = o_clamp(img_c_b, 0, (read->size.x-1));


                vec4 *src_aa = MMat_at_raw(read, img_c_a, img_r_a);
                vec4 *src_ba = MMat_at_raw(read, img_c_b, img_r_a);
                vec4 *src_ab = MMat_at_raw(read, img_c_a, img_r_b);
                vec4 *src_bb = MMat_at_raw(read, img_c_b, img_r_b);

                bvec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);

                vec4 tmp_a = vec4_mix(*src_aa, *src_ba, img_c_t);
                vec4 tmp_b = vec4_mix(*src_ab, *src_bb, img_c_t);
                vec4 tmp = vec4_mix(tmp_a, tmp_b, img_r_t);

                *dst = bvec4_cast_float_1(tmp.v);
            }
        }
        return;
    }

    // fast path linear vec4
    if(self->format == M_FORMAT_RGBA_F32 && read->format == M_FORMAT_RGBA_F32) {
        for (int r_i = 0; r_i<rect.height; r_i++) {
            int dst_r = r_i + rect.y;
            float r_t = (float) r_i / (float) (rect.height-1);
            float sample_r = img_rect.y + r_t * img_rect.height;
            int img_r_a = m_floor(sample_r);
            int img_r_b = m_ceil(sample_r);
            float img_r_t = m_fract(sample_r);
            img_r_a = o_clamp(img_r_a, 0, (read->size.y-1));
            img_r_b = o_clamp(img_r_b, 0, (read->size.y-1));

            for (int c_i=0; c_i < rect.width; c_i++) {
                int dst_c = c_i + rect.x;
                float c_t = (float) c_i / (float) (rect.width-1);
                float sample_c = img_rect.x + c_t * img_rect.width;
                int img_c_a = m_floor(sample_c);
                int img_c_b = m_ceil(sample_c);
                float img_c_t = m_fract(sample_c);
                img_c_a = o_clamp(img_c_a, 0, (read->size.x-1));
                img_c_b = o_clamp(img_c_b, 0, (read->size.x-1));


                vec4 *src_aa = MMat_at_raw(read, img_c_a, img_r_a);
                vec4 *src_ba = MMat_at_raw(read, img_c_b, img_r_a);
                vec4 *src_ab = MMat_at_raw(read, img_c_a, img_r_b);
                vec4 *src_bb = MMat_at_raw(read, img_c_b, img_r_b);

                vec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);

                vec4 tmp_a = vec4_mix(*src_aa, *src_ba, img_c_t);
                vec4 tmp_b = vec4_mix(*src_ab, *src_bb, img_c_t);
                vec4 tmp = vec4_mix(tmp_a, tmp_b, img_r_t);

                *dst = tmp;
            }
        }
        return;
    }
    
    // linear generic
    {
        for (int r_i = 0; r_i<rect.height; r_i++) {
            int dst_r = r_i + rect.y;
            float r_t = (float) r_i / (float) (rect.height-1);
            float sample_r = img_rect.y + r_t * img_rect.height;
            int img_r_a = m_floor(sample_r);
            int img_r_b = m_ceil(sample_r);
            float img_r_t = m_fract(sample_r);
            img_r_a = o_clamp(img_r_a, 0, (read->size.y-1));
            img_r_b = o_clamp(img_r_b, 0, (read->size.y-1));
            
            for (int c_i=0; c_i < rect.width; c_i++) {
                int dst_c = c_i + rect.x;
                float c_t = (float) c_i / (float) (rect.width-1);
                float sample_c = img_rect.x + c_t * img_rect.width;
                int img_c_a = m_floor(sample_c);
                int img_c_b = m_ceil(sample_c);
                float img_c_t = m_fract(sample_c);
                img_c_a = o_clamp(img_c_a, 0, (read->size.x-1));
                img_c_b = o_clamp(img_c_b, 0, (read->size.x-1));
            
                
                void *src_aa = MMat_at_raw(read, img_c_a, img_r_a);
                void *src_ba = MMat_at_raw(read, img_c_b, img_r_a);
                void *src_ab = MMat_at_raw(read, img_c_a, img_r_b);
                void *src_bb = MMat_at_raw(read, img_c_b, img_r_b);
                
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                
                
                vec4 tmp_aa = m_format_cast_to_vec4(src_aa, read->format);
                vec4 tmp_ba = m_format_cast_to_vec4(src_ba, read->format);
                vec4 tmp_ab = m_format_cast_to_vec4(src_ab, read->format);
                vec4 tmp_bb = m_format_cast_to_vec4(src_bb, read->format);
                
                vec4 tmp_a = vec4_mix(tmp_aa, tmp_ba, img_c_t);
                vec4 tmp_b = vec4_mix(tmp_ab, tmp_bb, img_c_t);
                vec4 tmp = vec4_mix(tmp_a, tmp_b, img_r_t);       
                
                m_format_cast_from_vec4(dst, self->format, tmp);
            }
        }
        return;
    }
    
}

void MMat_resample_blend_rect(oobj obj, oobj img, m_ivec4 rect, m_vec4 img_rect, bool linear)
{
    MMat_assert(obj);
    MMat *self = obj;
    OObj_assert(img, MMat);
    MMat *read = img;

    assert(self->data_root != read->data_root);

    if (rect.width<=0 || rect.height<=0) {
        rect = MMat_rect_int(self, 0, 0);
    }
    if (img_rect.width<=0 || img_rect.height <=0) {
        img_rect = MMat_rect(img, 0, 0);
    }

    // clamp rect
    rect.xy = ivec2_max(rect.xy, 0);
    rect.zw = ivec2_min_v(rect.zw, ivec2_sub_v(self->size, rect.xy));


    if (rect.width<=0 || rect.height<=0) {
        // nothing to blit
        return;
    }

    // nearest generic format missmatch
    if(!linear) {
        for (int r_i = 0; r_i<rect.height; r_i++) {
            int dst_r = r_i + rect.y;
            float r_t = (float) r_i / (float) (rect.height-1);
            float sample_r = img_rect.y + r_t * img_rect.height;
            int img_r = m_round(sample_r);
            img_r = o_clamp(img_r, 0, (read->size.y-1));

            for (int c_i=0; c_i < rect.width; c_i++) {
                int dst_c = c_i + rect.x;
                float c_t = (float) c_i / (float) (rect.width-1);
                float sample_c = img_rect.x + c_t * img_rect.width;
                int img_c = m_round(sample_c);
                img_c = o_clamp(img_c, 0, (read->size.x-1));

                void *restrict src = MMat_at_raw(read, img_c, img_r);
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                vec4 src_vec = m_format_cast_to_vec4(src, read->format);
                vec4 dst_vec = m_format_cast_to_vec4(dst, self->format);
                dst_vec.xyz = vec3_mix(dst_vec.xyz, src_vec.xyz, src_vec.a);
                dst_vec.a = o_min(1.0, dst_vec.a + src_vec.a);
                m_format_cast_from_vec4(dst, self->format, dst_vec);
            }
        }
        return;
    }

    // linear generic
    {
        for (int r_i = 0; r_i<rect.height; r_i++) {
            int dst_r = r_i + rect.y;
            float r_t = (float) r_i / (float) (rect.height-1);
            float sample_r = img_rect.y + r_t * img_rect.height;
            int img_r_a = m_floor(sample_r);
            int img_r_b = m_ceil(sample_r);
            float img_r_t = m_fract(sample_r);
            img_r_a = o_clamp(img_r_a, 0, (read->size.y-1));
            img_r_b = o_clamp(img_r_b, 0, (read->size.y-1));

            for (int c_i=0; c_i < rect.width; c_i++) {
                int dst_c = c_i + rect.x;
                float c_t = (float) c_i / (float) (rect.width-1);
                float sample_c = img_rect.x + c_t * img_rect.width;
                int img_c_a = m_floor(sample_c);
                int img_c_b = m_ceil(sample_c);
                float img_c_t = m_fract(sample_c);
                img_c_a = o_clamp(img_c_a, 0, (read->size.x-1));
                img_c_b = o_clamp(img_c_b, 0, (read->size.x-1));


                void *restrict src_aa = MMat_at_raw(read, img_c_a, img_r_a);
                void *restrict src_ba = MMat_at_raw(read, img_c_b, img_r_a);
                void *restrict src_ab = MMat_at_raw(read, img_c_a, img_r_b);
                void *restrict src_bb = MMat_at_raw(read, img_c_b, img_r_b);

                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);


                vec4 tmp_aa = m_format_cast_to_vec4(src_aa, read->format);
                vec4 tmp_ba = m_format_cast_to_vec4(src_ba, read->format);
                vec4 tmp_ab = m_format_cast_to_vec4(src_ab, read->format);
                vec4 tmp_bb = m_format_cast_to_vec4(src_bb, read->format);

                vec4 tmp_a = vec4_mix(tmp_aa, tmp_ba, img_c_t);
                vec4 tmp_b = vec4_mix(tmp_ab, tmp_bb, img_c_t);
                vec4 tmp = vec4_mix(tmp_a, tmp_b, img_r_t);

                vec4 dst_vec = m_format_cast_to_vec4(dst, self->format);
                dst_vec.xyz = vec3_mix(dst_vec.xyz, tmp.xyz, tmp.a);
                dst_vec.a = o_min(1.0, dst_vec.a + tmp.a);
                m_format_cast_from_vec4(dst, self->format, dst_vec);
            }
        }
        return;
    }
}


void MMat_resample_down_into(oobj obj, oobj into)
{
    MMat_assert(obj);
    MMat *self = obj;
    OObj_assert(into, MMat);
    MMat *down = into;

    assert(self->data_root != down->data_root);

    // fast path bvec4 packed
    if (self->format == M_FORMAT_RGBA_U8 && down->format == M_FORMAT_RGBA_U8
        && MMat_packed(self)) {
        for (int r=0; r<down->size.y; r++) {
            int src_r = o_min(r*2, self->size.y-2);
            for (int c=0; c<down->size.x; c++) {
                int src_c = o_min(c*2, self->size.x-2);
                
                const bvec4 *src_aa = MMat_at_raw(self, src_c, src_r);
                const bvec4 *src_ba = src_aa+1;
                const bvec4 *src_ab = MMat_at_raw(self, src_c, src_r+1);
                const bvec4 *src_bb = src_ab+1;
                bvec4 *dst = MMat_at_raw(down, c, r);
                bvec4 tmp = {
                    {
                        (obyte) ( ((float) src_aa->v0 + (float) src_ba->v0 + (float) src_ab->v0 + (float) src_bb->v0) / 4.0f),
                        (obyte) ( ((float) src_aa->v1 + (float) src_ba->v1 + (float) src_ab->v1 + (float) src_bb->v1) / 4.0f),
                        (obyte) ( ((float) src_aa->v2 + (float) src_ba->v2 + (float) src_ab->v2 + (float) src_bb->v2) / 4.0f),
                        (obyte) ( ((float) src_aa->v3 + (float) src_ba->v3 + (float) src_ab->v3 + (float) src_bb->v3) / 4.0f)
                    }};
                *dst = tmp;
            }
        }
        return;
    }


    // fast path bvec4
    if (self->format == M_FORMAT_RGBA_U8 && down->format == M_FORMAT_RGBA_U8) {
        for (int r=0; r<down->size.y; r++) {
            int src_r = o_min(r*2, self->size.y-2);
            for (int c=0; c<down->size.x; c++) {
                int src_c = o_min(c*2, self->size.x-2);

                const bvec4 *src_aa = MMat_at_raw(self, src_c, src_r);
                const bvec4 *src_ba = MMat_at_raw(self, src_c+1, src_r);
                const bvec4 *src_ab = MMat_at_raw(self, src_c, src_r+1);
                const bvec4 *src_bb = MMat_at_raw(self, src_c+1, src_r+1);
                bvec4 *dst = MMat_at_raw(down, c, r);
                bvec4 tmp = {
                    {
                        (obyte) ( ((float) src_aa->v0 + (float) src_ba->v0 + (float) src_ab->v0 + (float) src_bb->v0) / 4.0f),
                        (obyte) ( ((float) src_aa->v1 + (float) src_ba->v1 + (float) src_ab->v1 + (float) src_bb->v1) / 4.0f),
                        (obyte) ( ((float) src_aa->v2 + (float) src_ba->v2 + (float) src_ab->v2 + (float) src_bb->v2) / 4.0f),
                        (obyte) ( ((float) src_aa->v3 + (float) src_ba->v3 + (float) src_ab->v3 + (float) src_bb->v3) / 4.0f)
                    }};
                *dst = tmp;
            }
        }
        return;
    }
    
    // fast path vec4 packed
    if (self->format == M_FORMAT_RGBA_F32 && down->format == M_FORMAT_RGBA_F32
        && MMat_packed(self)) {
        for (int r=0; r<down->size.y; r++) {
            int src_r = o_min(r*2, self->size.y-2);
            for (int c=0; c<down->size.x; c++) {
                int src_c = o_min(c*2, self->size.x-2);
                
                const vec4 *src_aa = MMat_at_raw(self, src_c, src_r);
                const vec4 *src_ba = src_aa+1;
                const vec4 *src_ab = MMat_at_raw(self, src_c, src_r+1);
                const vec4 *src_bb =src_ab+1;
                vec4 *dst = MMat_at_raw(down, c, r);
                vec4 tmp = {
                    {
                        (src_aa->v0 + src_ba->v0 + src_ab->v0 + src_bb->v0) / 4.0f,
                        (src_aa->v1 + src_ba->v1 + src_ab->v1 + src_bb->v1) / 4.0f,
                        (src_aa->v2 + src_ba->v2 + src_ab->v2 + src_bb->v2) / 4.0f,
                        (src_aa->v3 + src_ba->v3 + src_ab->v3 + src_bb->v3) / 4.0f,
                    }};
                *dst = tmp;
            }
        }
        return;
    }
    
    // fast path vec4
    if (self->format == M_FORMAT_RGBA_F32 && down->format == M_FORMAT_RGBA_F32) {
        for (int r=0; r<down->size.y; r++) {
            int src_r = o_min(r*2, self->size.y-2);
            for (int c=0; c<down->size.x; c++) {
                int src_c = o_min(c*2, self->size.x-2);
                
                const vec4 *src_aa = MMat_at_raw(self, src_c, src_r);
                const vec4 *src_ba = MMat_at_raw(self, src_c+1, src_r);
                const vec4 *src_ab = MMat_at_raw(self, src_c, src_r+1);
                const vec4 *src_bb = MMat_at_raw(self, src_c+1, src_r+1);
                vec4 *dst = MMat_at_raw(down, c, r);
                vec4 tmp = {
                    {
                        (src_aa->v0 + src_ba->v0 + src_ab->v0 + src_bb->v0) / 4.0f,
                        (src_aa->v1 + src_ba->v1 + src_ab->v1 + src_bb->v1) / 4.0f,
                        (src_aa->v2 + src_ba->v2 + src_ab->v2 + src_bb->v2) / 4.0f,
                        (src_aa->v3 + src_ba->v3 + src_ab->v3 + src_bb->v3) / 4.0f,
                    }};
                *dst = tmp;
            }
        }
        return;
    }

    // generic
    {
        for (int r=0; r<down->size.y; r++) {
            int src_r = o_min(r*2, self->size.y-2);
            for (int c=0; c<down->size.x; c++) {
                int src_c = o_min(c*2, self->size.x-2);
                
                const void *src_aa = MMat_at_raw(self, src_c, src_r);
                const void *src_ba = MMat_at_raw(self, src_c+1, src_r);
                const void *src_ab = MMat_at_raw(self, src_c, src_r+1);
                const void *src_bb = MMat_at_raw(self, src_c+1, src_r+1);
                
                void *dst = MMat_at_raw(down, c, r);
                
                vec4 tmp_aa = m_format_cast_to_vec4(src_aa, self->format);
                vec4 tmp_ba = m_format_cast_to_vec4(src_ba, self->format);
                vec4 tmp_ab = m_format_cast_to_vec4(src_ab, self->format);
                vec4 tmp_bb = m_format_cast_to_vec4(src_bb, self->format);
                
                vec4 tmp = {
                    {
                        (tmp_aa.v0 + tmp_ba.v0 + tmp_ab.v0 + tmp_bb.v0) / 4.0f,
                        (tmp_aa.v1 + tmp_ba.v1 + tmp_ab.v1 + tmp_bb.v1) / 4.0f,
                        (tmp_aa.v2 + tmp_ba.v2 + tmp_ab.v2 + tmp_bb.v2) / 4.0f,
                        (tmp_aa.v3 + tmp_ba.v3 + tmp_ab.v3 + tmp_bb.v3) / 4.0f,
                    }};
                
                m_format_cast_from_vec4(dst, down->format, tmp);
            }
        }
        return;
    }
}
struct oobj_opt MMat_resample_down_try(oobj obj, ou32 format)
{
    MMat_assert(obj);
    MMat *self = obj;
    if (format == M_FORMAT_KEEP) {
        format = self->format;
    }
    ivec2 size = MMat_size_int(obj);
    size.x = (int) m_ceil((float) size.x / 2.0f);
    size.y = (int) m_ceil((float) size.y / 2.0f);
    MMat *res = MMat_new_try(obj, NULL, size.x, size.y, format).o;
    if (!res) {
        return oobj_opt(NULL);
    }
    MMat_resample_down_into(self, res);
    return oobj_opt(res);
}

void MMat_resample_resize_rect_into(oobj obj, oobj into, m_vec4 rect, bool linear)
{
    if (rect.width <= 0 || rect.height <= 0) {
        rect = MMat_rect(obj, 0, 0);
    }
    ivec2 into_size = MMat_size_int(into);
    ou32 obj_format = MMat_format(into);
    ou32 into_format = MMat_format(into);

    // to steed things up for a large into_size, cast source if format is not perfect for linear blitting
    bool internal_cast = false;
    if (linear && (into_size.x * into_size.y > 256*256) && obj_format != M_FORMAT_RGBA_F32) {
        internal_cast = into_format == M_FORMAT_RGBA_U8 || into_format == M_FORMAT_RGBA_F32;
    }

    oobj container = NULL;
    oobj source = obj;
    if (internal_cast) {
        o_log_debug_s(__func__, "internal cast to speed up");
        container = MMat_cast_try(obj, M_FORMAT_RGBA_F32).o;
        if (container) {
            source = container;
        }
    }

    MMat_resample_blit_rect(into, source, ivec4_(0), rect, linear);
    o_del(container);
}
struct oobj_opt MMat_resample_resize_rect_try(oobj obj, m_vec4 rect, m_ivec2 size, bool linear, ou32 format)
{
    MMat_assert(obj);
    MMat *self = obj;
    if (format == M_FORMAT_KEEP) {
        format = self->format;
    }
    MMat *res = MMat_new_try(obj, NULL, m_2(size), format).o;
    if (!res) {
        return oobj_opt(NULL);
    }
    MMat_resample_resize_rect_into(self, res, rect, linear);
    return oobj_opt(res);
}

struct resize_smooth_context {
    MMat *obj;
    MMat *down;
    MMat *into;
    m_vec4 rect;
    int needed_cnts;
};

static float resize_smooth_task(oobj task, int run)
{
    struct resize_smooth_context *C = o_user(task);

    vec2 scale = vec2_div_v(C->rect.zw, MMat_size(C->into));
    if (scale.x <= 2.0f && scale.y <= 2.0f) {
        MMat_resample_blit_rect(C->into, C->down, ivec4_(0), C->rect, true);
        OTask_result_set(task, C->into);
        return OTask_FINISH;
    }

    // downsample
    oobj old_down = C->down;
    C->down = MMat_resample_down_try(C->down, 0).o;
    if (!C->down) {
        o_log_error_s(__func__, "Allocation error");
        return OTask_FINISH;
    }

    // apply scaling
    vec4 rect_full = MMat_rect(old_down, 0, 0);
    vec4 rect_down = MMat_rect(C->down, 0, 0);
    vec2 scale_down = vec2_div_v(rect_down.zw, rect_full.zw);
    C->rect.xy = vec2_scale_v(C->rect.xy, scale_down);
    C->rect.zw = vec2_scale_v(C->rect.zw, scale_down);

    // free some memory if down was large (before OTask is deleted...)
    // needs o_move, else down is a child of down...
    o_move(C->down, task);
    if (old_down != C->obj) {
        o_del(old_down);
    }

    float progress = (float) (run+1) / C->needed_cnts;
    // must not be >= 1.0f (just to be sure...)
    progress = o_min(progress, 0.95f);
    return progress;
}

oobj MMat_resample_resize_smooth_rect_into_task(oobj obj, oobj into, m_vec4 rect)
{
    if (rect.width <= 0 || rect.height <= 0) {
        rect = MMat_rect(obj, 0, 0);
    }
    oobj task = OTask_new(obj, resize_smooth_task, true);
    struct resize_smooth_context *C = o_user_new0(task, *C, 1);
    C->obj = obj;
    C->into = into;
    C->down = obj;
    C->rect = rect;

    // calc needed cnts
    C->needed_cnts = 1;
    vec2 rect_test_size = rect.zw;
    vec2 into_size = MMat_size(into);
    vec2 scale = vec2_div_v(rect_test_size, into_size);
    while (scale.x > 2.0f || scale.y > 2.0f) {
        C->needed_cnts++;
        rect_test_size = vec2_div(rect_test_size, 2.0f);
        scale = vec2_div_v(rect_test_size, into_size);
        scale = vec2_ceil(scale);
    }
    return task;
}

struct oobj_opt MMat_resample_resize_smooth_rect_try(oobj obj, m_vec4 rect, m_ivec2 size, ou32 format)
{
    if (format == M_FORMAT_KEEP) {
        format = MMat_format(obj);
    }
    MMat *into = MMat_new_try(obj, NULL, m_2(size), format).o;
    if (!into) {
        return oobj_opt(NULL);
    }
    oobj task = MMat_resample_resize_smooth_rect_into_task(obj, into, rect);
    OTask_run_blocking(task);
    struct oobj_opt result = OTask_result(task);
    if (!result.o) {
        // allocation error, already log'ged
        o_del(into);
        // just be sure...
        into = NULL;
    }
    // dont forgert to delete the task
    o_del(task);
    return oobj_opt(into);
}




void MMat_blend_dab_rect(oobj obj, oobj img, int x, int y, m_ivec4 img_rect, m_vec4 color)
{
    MMat_assert(obj);
    MMat *self = obj;
    OObj_assert(img, MMat);
    MMat *read = img;

    assert(self->data_root != read->data_root);

    ivec4 res_rect = blitblend_rect(self, read, x, y, img_rect);
    int res_c = res_rect.v0;
    int res_r = res_rect.v1;
    int res_c2 = res_rect.v2;
    int res_r2 = res_rect.v3;


    if (res_c>=res_c2 || res_r>=res_r2) {
        // nothing to blit
        return;
    }

    int channels = m_format_channels(self->format);

    // fast path: bvec4, mask version
    if (self->format == M_FORMAT_RGBA_U8 && read->format == M_FORMAT_RGBA_U8 && channels < 4) {
       for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                obyte *restrict mask = MMat_at_raw(read, c, r);
                bvec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                float mask_f = *mask / 255.0f;
                // dab mask * color for each channel
                vec4 dabcolor = color;
                dabcolor.a *= mask_f;
                // dab mask controls the resulting blending
                *dst = bvec4_mix(*dst, bvec4_cast_float_1(dabcolor.v), mask_f);
            }
        }
        return;
    }
    // fast path: bvec4, bvec4 version
    if (self->format == M_FORMAT_RGBA_U8 && read->format == M_FORMAT_RGBA_U8 && channels >= 4) {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                bvec4 *restrict src = MMat_at_raw(read, c, r);
                bvec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                // dab image * color for each channel
                vec4 dabcolor = vec4_scale_v(vec4_cast_byte_1(src->v), color);
                // dab alpha controls the resulting blending
                *dst = bvec4_mix(*dst, bvec4_cast_float_1(dabcolor.v), src->a/255.0f);
            }
        }
        return;
    }
    // fast path: vec4, mask version
    if (self->format == M_FORMAT_RGBA_F32 && read->format == M_FORMAT_RGBA_F32 && channels < 4) {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                float *restrict mask = MMat_at_raw(read, c, r);
                vec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                // dab mask * color for each channel
                vec4 dabcolor = color;
                dabcolor.a *= *mask;
                // dab mask controls the resulting blending
                *dst = vec4_mix(*dst, dabcolor, *mask);
            }
        }
        return;
    }
    // fast path: vec4, vec4 version
    if (self->format == M_FORMAT_RGBA_F32 && read->format == M_FORMAT_RGBA_F32 && channels >= 4) {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                vec4 *restrict src = MMat_at_raw(read, c, r);
                vec4 *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                // dab image * color for each channel
                vec4 dabcolor = vec4_scale_v(*src, color);
                // dab alpha controls the resulting blending
                *dst = vec4_mix(*dst, dabcolor, src->a);
            }
        }
        return;
    }

    // generic format missmatch, mask version
    if (channels < 4) {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                void *restrict src = MMat_at_raw(read, c, r);
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                float mask = m_format_primitive_cast_to_float(src, read->format);
                vec4 dst_vec = m_format_cast_to_vec4(dst, self->format);
                // dab mask * color for each channel
                vec4 dabcolor = color;
                dabcolor.a *= mask;
                // dab mask controls the resulting blending
                dst_vec = vec4_mix(dst_vec, dabcolor, mask);
                m_format_cast_from_vec4(dst, self->format, dst_vec);
            }
        }
        return;
    }

    // generic format missmatch, vec4 version
    {
        for (int r = res_r; r < res_r2; r++) {
            int dst_r = y + r;
            for (int c = res_c; c < res_c2; c++) {
                int dst_c = x + c;
                void *restrict src = MMat_at_raw(read, c, r);
                void *restrict dst = MMat_at_raw(self, dst_c, dst_r);
                vec4 src_vec = m_format_cast_to_vec4(src, read->format);
                vec4 dst_vec = m_format_cast_to_vec4(dst, self->format);
                // dab image * color for each channel
                vec4 dabcolor = vec4_scale_v(src_vec, color);
                // dab alpha controls the resulting blending
                dst_vec = vec4_mix(dst_vec, dabcolor, src_vec.a);
                m_format_cast_from_vec4(dst, self->format, dst_vec);
            }
        }
        return;
    }
}
