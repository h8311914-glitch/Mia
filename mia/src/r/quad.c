#include "r/quad.h"
#include "m/flt.h"


struct r_quad r_quad_new(float w, float h)
{
    // minimum 1 to create a valid size
    w = m_max(1, w);
    h = m_max(1, h);

    struct r_quad q;
    // pose has a position of (0, 0) and a size equal to the tex size
    q.pose = mat4_eye();
    q.pose.m00 = w;
    q.pose.m11 = h;
    // uv maps the whole tex, so as the pose:
    //      position of (0, 0) and a size equal to the tex size
    q.uv_pose = q.pose;
    q.sprite = vec4_(0, 0, w, h);
    q.fx = vec4_(1);
    q.fy = vec4_(0);
    q.fz = vec4_(0);
    return q;
}
