#ifndef M_MAT_MAT2_H
#define M_MAT_MAT2_H

/**
 * @file mat/m_mat2.h
 *
 * functions working with m_mat2
 */


#include "m/m_types/flt.h"


/** dst = v0, v1, ... */
o_inline m_mat2 mat2_new(float v0, float v1,
                float v2, float v3)
{
    return (m_mat2){
        {
            v0, v1,
            v2, v3
        }
    };
}


/** dst = (m_mat2) v */
o_inline m_mat2 mat2_cast_double(const double *cast)
{
    return (m_mat2){
        {
            cast[0], cast[1],
            cast[2], cast[3]
        }
    };
}

/** dst = r==c ? 1 : 0 (identity)  */
o_inline m_mat2 mat2_eye(void)
{
    return (m_mat2){
        {
            1, 0,
            0, 1
        }
    };
}

/** dst = m[row][:] */
o_inline m_vec2 mat2_get_row(m_mat2 m, int row)
{
    assert(0 <= row && row < 2);
    return (m_vec2){
        {
            m.col[0].v[row], m.col[1].v[row]
        }
    };
}


/** dst = m[:][col] */
o_inline m_vec2 mat2_get_col(m_mat2 m, int col)
{
    assert(0 <= col && col < 2);
    return m.col[col];
}


/** dst = m; dst[row][:] = v */
o_inline m_mat2 mat2_set_row(m_mat2 m, m_vec2 v, int row)
{
    assert(0 <= row && row < 2);
    m.col[0].v[row] = v.v0;
    m.col[1].v[row] = v.v1;
    return m;
}

/** m[row][:] = v; dst = m */
o_inline m_mat2 mat2_set_this_row(m_mat2 *m, m_vec2 v, int row)
{
    assert(0 <= row && row < 2);
    m->col[0].v[row] = v.v0;
    m->col[1].v[row] = v.v1;
    return *m;
}

/** dst = m; dst[:][col] = v */
o_inline m_mat2 mat2_set_col(m_mat2 m, m_vec2 v, int col)
{
    assert(0 <= col && col < 2);
    m.col[col] = v;
    return m;
}

/** mat[:][col] = v; dst = m */
o_inline m_mat2 mat2_set_this_col(m_mat2 *m, m_vec2 v, int col)
{
    assert(0 <= col && col < 2);
    m->col[col] = v;
    return *m;
}


/** returns sum of diagonal form upper left to lower right */
o_inline float mat2_trace(m_mat2 m)
{
    return m.m00 + m.m11;
}


/** dst = m^t */
o_inline m_mat2 mat2_transpose(m_mat2 m)
{
    return (m_mat2){
        {
            m.m00, m.m10,
            m.m01, m.m11
        }
    };
}


/** dst = a @ b */
o_inline m_mat2 mat2_mul_mat(m_mat2 a, m_mat2 b)
{
    return (m_mat2){
        {
            a.v0 * b.v0 + a.v2 * b.v1,
            a.v1 * b.v0 + a.v3 * b.v1,

            a.v0 * b.v2 + a.v2 * b.v3,
            a.v1 * b.v2 + a.v3 * b.v3
        }
    };
}


/** dst = a @ b */
o_inline m_vec2 mat2_mul_vec(m_mat2 a, m_vec2 b)
{
    return (m_vec2){
        {
            a.v0 * b.v0 + a.v2 * b.v1,
            a.v1 * b.v0 + a.v3 * b.v1
        }
    };
}


/** dst = a @ b */
o_inline m_vec2 vec2_mul_mat(m_vec2 a, m_mat2 b)
{
    return (m_vec2){
        {
            b.v0 * a.v0 + b.v1 * a.v1,
            b.v2 * a.v0 + b.v3 * a.v1
        }
    };
}


/** returns = determinant mm */
o_inline float mat2_det(m_mat2 mat)
{
    // from cglm/mat2.h/glm_mat2_det
    return mat.m[0][0] * mat.m[1][1] - mat.m[1][0] * mat.m[0][1];
}


/** dst = inverted mm */
static m_mat2 mat2_inv(m_mat2 mat)
{
    m_mat2 res;
    // from cglm/mat2.h/glm_mat2_inv
    float a = mat.m[0][0], b = mat.m[0][1];
    float c = mat.m[1][0], d = mat.m[1][1];

    res.m[0][0] = d;
    res.m[0][1] = -b;
    res.m[1][0] = -c;
    res.m[1][1] = a;

    float det = a * d - b * c;
    float inv_det = 1.0f / det;
    for (int vi = 0; vi < 4; vi++) {
        res.v[vi] *= inv_det;
    }

    return res;
}

#endif //M_MAT_MAT2_H
