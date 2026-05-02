#include "o/img.h"
#include "m/flt.h"
#include "m/byte.h"
#include "r/RTex_ex.h"
#include "r/tex.h"


#define test(expr) o_assume(expr, "test failed")

static void color(oobj obj)
{
    bvec4 src, dst;
    oobj img;
    
    src = bvec4_(250, 10, 107, 255);
    
    img = RTex_color(r_tex_white(), vec4_cast_byte_1(src.v), vec4_(0.0), R_FORMAT_RGBA_U8);

    //RTex_write_file(img, "dst.png");
    
    RTex_get(img, &dst);
    
    test(bvec4_equals_v(src, dst));
    
}

static void outline_run(oobj obj, int cols, int rows)
{
    struct o_img src, dst;
    bvec4 *col;
    oobj img;
    ivec2 pos = {{cols/2, rows/2}};
    bvec4 outline = {{66, 55, 99, 255}};
    
    src = o_img_new0(obj, cols, rows, o_img_RGBA);
    col = (bvec4*) o_img_at(src, m_2(pos));
    *col = bvec4_(10, 20, 30, 255);
    
    img = RTex_new(obj, src.data, cols, rows);

    img = RTex_outline(img, ivec2_(3), ivec2_(0), vec4_(0, 0, 0, 1), vec4_cast_byte_1(outline.v), R_FORMAT_RGBA_U8);
    
    dst = o_img_new(obj, cols, rows, o_img_RGBA);
    RTex_get(img, dst.data);
    
    //o_img_write_file(src, "src.png");
    //o_img_write_file(dst, "dst.png");
    
    // test dst
    test(memcmp(o_img_at(dst, pos.x-1, pos.y), outline.v, sizeof outline) == 0);
    test(memcmp(o_img_at(dst, pos.x+1, pos.y), outline.v, sizeof outline) == 0);
    test(memcmp(o_img_at(dst, pos.x, pos.y-1), outline.v, sizeof outline) == 0);
    test(memcmp(o_img_at(dst, pos.x, pos.y+1), outline.v, sizeof outline) == 0);
}

static void outline(oobj obj)
{
    outline_run(obj, 16, 16);
    outline_run(obj, 16, 33);
    outline_run(obj, 33, 16);
    outline_run(obj, 16, 49);
}

int RTex__test(oobj obj)
{
    color(obj);
    outline(obj);
    
    return 0;
}

