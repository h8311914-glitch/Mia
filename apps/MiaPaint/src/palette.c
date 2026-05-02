#include "mp/palette.h"
#include "o/log.h"
#include "m/vec/ivec2.h"
#include "m/vec/vec2.h"
#include "a/AView.h"
#include "o/str.h"
#include "r/RObjBox.h"
#include "u/scroll.h"
#include "r/RTex.h"
#include "r/RCam.h"
#include "a/common.h"

#include "mp/surface.h"

#define PALETTE_VIEW_NAME "MpPalette"

struct palette_context {
    struct mp_surface *surface;

    oobj ro;
    int num;

    struct u_scroll scroll;
    vec2 needed_size;

    // -1 for hidden
    int current;

    ivec2 layout;
};

static void palette_current_set(struct palette_context *C, int set)
{
    if (set < 0 || set >= C->num) {
        // set to hidden and ignore
        C->current = -1;
        return;
    }
    C->current = set;
    struct r_box *current = o_at(C->ro, C->num + set);
    C->surface->color = current->fx;
}

static void palette_pointer(struct palette_context *C, struct a_pointer pointer, vec2 box_size, ivec2 layout)
{
    if (a_pointer_pressed(pointer)) {
        ivec2 pos = ivec2_cast_float(vec2_div_v(pointer.pos.xy, box_size).v);
        if (pos.x >= 0 && pos.x < layout.x
            && pos.y >= 0 && pos.y < layout.y) {
            int curr = pos.x + pos.y * layout.x;
            palette_current_set(C, curr);
        }
    }
}

static void palette_setup(oobj view)
{
    struct palette_context *C = o_user(view);

    C->scroll = u_scroll_new(u_scroll_XY, a_cam());
}

static void palette_update(oobj view, oobj tex, float dt)
{
    struct palette_context *C = o_user(view);

    // update layout if needed
    struct r_box *boxes = o_at(C->ro, 0);

    oobj cam = a_cam();

    vec2 box_size = boxes[0].rect.zw;
    vec2 cam_size = RCam_size(cam);

    ivec2 layout;
    layout.x = (int) m_floor(cam_size.x / box_size.x);
    layout.y = (int) m_ceil((float) C->num / layout.x);

    if (!ivec2_equals_v(layout, C->layout)) {
        C->layout = layout;

        for (int i = 0; i < C->num; i++) {
            struct r_box *bg = boxes + i;
            struct r_box *color = boxes + C->num + i;

            int column = i % C->layout.x;
            int row = i / C->layout.x;

            bg->rect.xy = color->rect.xy =
                          vec2_scale_v(box_size, vec2_(column, row));
        }
    }

    // check pressed for both pointer 0 and pointer 1
    palette_pointer(C, a_pointer(0, 0), box_size, layout);
    palette_pointer(C, a_pointer(1, 0), box_size, layout);


    // set current
    struct r_box *current = boxes + 2 * C->num;
    if (C->current>=0 && C->current<C->num) {
        current->rect.xy = boxes[C->current].rect.xy;
    } else {
        current->rect.xy = vec2_(m_MAX);
    }

    C->needed_size = vec2_scale(vec2_cast_int(layout.v), MP_PALETTE_DROP_SIZE);
    C->needed_size = vec2_max(C->needed_size, MP_PALETTE_DROP_SIZE);


    C->scroll.cam_limits_rect = u_rect_new(0, 0, m_2(C->needed_size));
    u_scroll_update(&C->scroll, dt);
}

static void palette_render(oobj view, oobj tex, float dt)
{
    struct palette_context *C = o_user(view);
    RTex_clear(tex, R_BLACK);
    RTex_ro(tex, C->ro);
}

//
// public
//

oobj mp_palette_new(oobj parent, struct mp_surface *surface, const vec4 *colors, int num)
{
    oobj view = AView_new(parent, palette_setup, palette_update, palette_render);
    OObj_name_set(view, PALETTE_VIEW_NAME);
    struct palette_context *C = o_user_new0(view, *C, 1);

    C->surface = surface;

    C->ro = RObjBox_new_rgba(view, 1, "$mp/palette/color.png", 2, 2);
    mp_palette_set(view, colors, num);

    return view;
}

vec2 mp_palette_needed_size(oobj view)
{
    // assert the passed view is ours.
    assert(o_str_equals(OObj_name(view), PALETTE_VIEW_NAME));
    struct palette_context *C = o_user(view);
    return C->needed_size;
}

void mp_palette_current_hide(oobj view)
{
    assert(o_str_equals(OObj_name(view), PALETTE_VIEW_NAME));
    struct palette_context *C = o_user(view);
    palette_current_set(C, -1);
}

void mp_palette_set(oobj view, const vec4 *colors, int num)
{
    // assert the passed view is ours.
    assert(o_str_equals(OObj_name(view), PALETTE_VIEW_NAME));
    assert(num>0);
    struct palette_context *C = o_user(view);

    oobj boxes_array = RObjBox_boxes(C->ro);
    OArray_resize(boxes_array, num * 2 + 1);
    C->num = num;

    struct r_box *boxes = o_at(C->ro, 0);

    struct r_box def = boxes[0];
    def.fx = R_WHITE;

    for (int i = 0; i < num; i++) {
        struct r_box *bg = boxes + i;
        struct r_box *color = boxes + num + i;
        *bg = *color = def;
        color->sprite.x = 1;
        color->fx = colors[i];
    }
    struct r_box *current = boxes + num * 2;
    *current = def;
    current->sprite.xy = vec2_(1, 1);

    C->layout = ivec2_(0);
    palette_current_set(C, o_min(2, num-1));
}
