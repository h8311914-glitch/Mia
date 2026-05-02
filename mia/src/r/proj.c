#include "r/proj.h"

#include <stdlib.h>

#include "m/vec/vec2.h"
#include "m/vec/ivec2.h"
#include "m/utils/camera.h"

struct r_proj r_proj_new(ivec2 viewport_size, float fixed_scale, vec2 min_units_size, bool pixelperfect)
{
    struct r_proj p = {0};

    p.viewport_size = viewport_size;
    p.vpsh = vec2_(viewport_size.x/2.0f, viewport_size.y/2.0f);
    p.vpeo = vec2_div(vec2_(viewport_size.x % 2, -viewport_size.y % 2), 2.0f);

    bool identity = fixed_scale <= 0 && min_units_size.x <= 0 && min_units_size.y <= 0;

    if (identity) {
        p.scale = 1;
        p.size = vec2_(m_2(viewport_size));
    } else {
        p.scale = fixed_scale;
        if (p.scale <= 0) {
            if(min_units_size.x <= 0) {
                p.scale = viewport_size.y / min_units_size.y;
            } else if(min_units_size.y <= 0) {
                p.scale = viewport_size.x / min_units_size.x;
            } else {
                float width_scale = viewport_size.x / min_units_size.x;
                float height_scale = viewport_size.y / min_units_size.y;
                p.scale = o_min(width_scale, height_scale);
            }
        }
        if (pixelperfect && p.scale > 1) {
            p.scale = m_floor(p.scale);
        }
        p.size = vec2_div(vec2_cast_int(viewport_size.v), p.scale);
    }
    
    // as left right bottom top
    vec4 bounds = vec4_(
        0,
        +p.size.x,
        +p.size.y,
        0
    );


    p.cam = mat4_camera_ortho(m_4(bounds), -1, 1);

    return p;
}
