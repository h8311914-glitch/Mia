#include "m/discr.h"
#include "o/OArray.h"
#include "m/vec/vec2.h"
#include "m/vec/ivec2.h"
#include "m/types/int.h"


//
// dash/dist floating point descretize functions
//

void m_discr_point(oobj points_array, vec2 pt)
{
    OArray_push(points_array, &pt);
}

int m_discr_line(oobj points_array, vec2 a, vec2 b, float dist)
{
    vec2 ab = vec2_sub_v(b, a);
    float full_dist = vec2_length(ab);
    int samples = (int) m_ceil(full_dist/dist);
    samples = o_max(1, samples);
    // align dist for an equal density
    dist = full_dist / (float) samples;

    for(int i=0; i<samples; i++) {
        float t = (float) (i+1) / (float) samples;
        vec2 s = vec2_add_scaled(a, ab, t);
        m_discr_point(points_array, s);
    }

    return samples;
}

int m_discr_rect(oobj points_array, vec2 a, vec2 b, float dist)
{
    int sampled = 0;
    sampled += m_discr_line(points_array, vec2_(a.x, a.y), vec2_(b.x, a.y), dist);
    sampled += m_discr_line(points_array, vec2_(b.x, a.y), vec2_(b.x, b.y), dist);
    sampled += m_discr_line(points_array, vec2_(b.x, b.y), vec2_(a.x, b.y), dist);
    sampled += m_discr_line(points_array, vec2_(a.x, b.y), vec2_(a.x, a.y), dist);
    return sampled;
}


int m_discr_circle_bow(oobj points_array, vec2 center, float radius, float a_rad, float b_rad, float dist)
{
    float ab_rad = b_rad-a_rad;
    float full_dist = m_abs(radius * ab_rad);
    int samples = (int) m_ceil(full_dist/dist);
    samples = o_max(1, samples);
    // align dist for an equal density
    dist = full_dist / (float) samples;

    for(int i=0; i<samples; i++) {
        float t = (float) (i+1) / (float) samples;
        vec2 s = center;
        s.x += radius * m_cos(a_rad + ab_rad * t);
        s.y += radius * m_sin(a_rad + ab_rad * t);
        m_discr_point(points_array, s);
    }

    return samples;
}


int m_discr_bezier(oobj points_array, vec2 p0, vec2 p1, vec2 p2, float dist)
{
    int sampled = 0;
    float t = 0.0f;
    vec2 prev_point = p0;
    float step = 0.01f; // Initial step size

    while (t <= 1.0f) {
        float inv_t = 1.0f - t;
        // Quadratic Bezier formula
        vec2 point = vec2_add_v(
                vec2_add_v(vec2_scale(p0, inv_t * inv_t),
                         vec2_scale(p1, 2 * inv_t * t)),
                vec2_scale(p2, t * t));

        // Compute the distance from the previous point
        float segment_length = vec2_distance(prev_point, point);
        if (segment_length >= dist || t == 1.0f) {
            // Only add the point if the distance is sufficient or it's the last point
            m_discr_point(points_array, point);
            sampled++;
            prev_point = point;
            t += step;
        } else {
            // Increase the step to find the next suitable point
            t += step * (dist / segment_length);
        }
    }

    return sampled;
}


//
// integer grid based descretize functions
// returned as vec2 array
// always rounds to floor
//


void m_discr_grid_point(oobj points_array, vec2 pt)
{
    pt = vec2_floor(pt);
    OArray_push(points_array, &pt);
}


int m_discr_grid_line(oobj points_array, vec2 a, vec2 b)
{
    ivec2 from = ivec2_cast_float(vec2_floor(a).v);
    ivec2 to = ivec2_cast_float(vec2_floor(b).v);
    int sign_x = 1, sign_y = 1;
    if (from.x > to.x)
        sign_x = -1;
    if (from.y > to.y)
        sign_y = -1;

    int dy = mi_abs(to.y - from.y);
    int dx = mi_abs(to.x - from.x);

    if (dx == 0) {
        for (int i = 1; i <= dy; i++) {
            vec2 pt = vec2_(from.x, from.y+i*sign_y);
            OArray_push(points_array, &pt);
        }
        return dy;
    }

    float m = (float) dy / dx;
    if (m <= 1) {
        for (int i = 1; i <= dx; i++) {
            vec2 pt = vec2_(from.x+i*sign_x, m_floor(0.5+from.y+i*m*sign_y));
            OArray_push(points_array, &pt);
        }
        return dx;
    }

    for (int i = 1; i <= dy; i++) {
        vec2 pt = vec2_(m_floor(0.5f+from.x+i/m*sign_x), from.y+i*sign_y);
            OArray_push(points_array, &pt);
    }
    return dy;
}


int m_discr_grid_box(oobj points_array, vec2 a, vec2 b)
{
    vec2 e0 = vec2_floor(a);
    vec2 e2 = vec2_floor(b);
    vec2 e1 = {{e2.x, e0.y}};
    vec2 e3 = {{e0.x, e2.y}};
    
    // a~==b
    if(vec2_sqr_distance(e0, e2) < 0.5) {
        m_discr_grid_point(points_array, e0);
        return 1;
    }
    
    int pushed = 0;
    pushed += m_discr_grid_line(points_array, e0, e1);
    pushed += m_discr_grid_line(points_array, e1, e2);
    pushed += m_discr_grid_line(points_array, e2, e3);
    pushed += m_discr_grid_line(points_array, e3, e0);
    return pushed;
}


int m_discr_grid_circle_bow(oobj points_array, vec2 center, float radius, float a_rad, float b_rad)
{
    center = vec2_floor(center);
    int r = (int) radius;
    int d = -r;
    int x = r;
    int y = 0;
    
    if(r < 1) {
        OArray_push(points_array, &vec2_(center.x, center.y));
        return 1;
    }
    if(r == 1) {
        OArray_push(points_array, &vec2_(center.x, center.y-1));
        OArray_push(points_array, &vec2_(center.x+1, center.y));
        OArray_push(points_array, &vec2_(center.x, center.y+1));
        OArray_push(points_array, &vec2_(center.x-1, center.y));
        return 4;
    }
    
    
    int num = 0;
    do {
        num += 8;
        
        OArray_push(points_array, &vec2_(center.x+x, center.y+y));
        OArray_push(points_array, &vec2_(center.x+y, center.y+x));
        OArray_push(points_array, &vec2_(center.x+x, center.y-y));
        OArray_push(points_array, &vec2_(center.x+y, center.y-x));
        OArray_push(points_array, &vec2_(center.x-x, center.y+y));
        OArray_push(points_array, &vec2_(center.x-y, center.y+x));
        OArray_push(points_array, &vec2_(center.x-x, center.y-y));
        OArray_push(points_array, &vec2_(center.x-y, center.y-x));

        d += 2 * y + 1;
        y++;
        if (d > 0) {
            d += -2 * x + 2;
            x--;
        }
    } while (y <= x);

    return num;
}

