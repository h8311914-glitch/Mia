#include "r/RBufferQuad.h"
#include "r/quad.h"

//
// public
//

RBufferQuad *RBufferQuad_init(oobj obj, oobj parent)
{
    RBufferQuad *self = obj;

    RBuffer_init(self, parent, sizeof (struct r_quad));

    const int in_pose = 0;
    const int in_uv_pose = 4;
    const int in_sprite = 8;
    const int in_fx = 9;
    const int in_fy = 10;
    const int in_fz = 11;

    RBuffer_location_mat4(self, in_pose, offsetof(struct r_quad, pose));
    RBuffer_location_mat4(self, in_uv_pose, offsetof(struct r_quad, uv_pose));
    RBuffer_location_vec4(self, in_sprite, offsetof(struct r_quad, sprite));
    RBuffer_location_vec4(self, in_fx, offsetof(struct r_quad, fx));
    RBuffer_location_vec4(self, in_fy, offsetof(struct r_quad, fy));
    RBuffer_location_vec4(self, in_fz, offsetof(struct r_quad, fz));
    
    return self;
}
