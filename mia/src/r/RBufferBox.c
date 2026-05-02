#include "r/RBufferBox.h"
#include "r/box.h"

//
// public
//

RBufferBox *RBufferBox_init(oobj obj, oobj parent)
{
    RBufferBox *self = obj;

    RBuffer_init(self, parent, sizeof (struct r_box));

    const int in_rect = 0;
    const int in_uv_rect = 1;
    const int in_sprite = 2;
    const int in_fx = 3;

    RBuffer_location_vec4(self, in_rect, offsetof(struct r_box, rect));
    RBuffer_location_vec4(self, in_uv_rect, offsetof(struct r_box, uv_rect));
    RBuffer_location_vec4(self, in_sprite, offsetof(struct r_box, sprite));
    RBuffer_location_vec4(self, in_fx, offsetof(struct r_box, fx));

    return self;
}
