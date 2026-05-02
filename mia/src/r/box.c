#include "r/box.h"
#include "m/flt.h"


struct r_box r_box_new(float w, float h)
{
    // minimum 1 to create a valid size
    w = m_max(1, w);
    h = m_max(1, h);

    struct r_box b;
    // rect at top left with given size
    // integer division
    b.rect.x = 0.0f;
    b.rect.y = 0.0f;
    b.rect.v2 = w;
    b.rect.v3 = h;
    b.uv_rect = b.rect;
    b.sprite = vec4_(0, 0, w, h);
    b.fx = vec4_(1);
    return b;
}
