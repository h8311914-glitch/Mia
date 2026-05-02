#ifndef M_MAT_MAT3_H
#define M_MAT_MAT3_H

/**
 * @file mat/m_mat3.h
 *
 * functions working with m_mat3
 */


#include "m/m_types/flt.h"
#include "m/sca/flt.h"


/** dst = v0, v1, ... */
o_inline m_mat3 mat3_new(float v0, float v1, float v2,
                float v3, float v4, float v5,
                float v6, float v7, float v8)
{
    return (m_mat3){
        {
            v0, v1, v2,
            v3, v4, v5,
            v6, v7, v8
        }
    };
}


/** dst = (m_mat3) v */
o_inline m_mat3 mat3_cast_double(const double *cast)
{
    return (m_mat3){
        {
            cast[0], cast[1], cast[2],
            cast[3], cast[4], cast[5],
            cast[6], cast[7], cast[8],
        }
    };
}

/** dst = r==c ? 1 : 0 (identity)  */
o_inline m_mat3 mat3_eye(void)
{
    return (m_mat3){
        {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
        }
    };
}

/** dst = m[row][:] */
o_inline m_vec3 mat3_get_row(m_mat3 m, int row)
{
    assert(0 <= row && row < 3);
    return (m_vec3){
        {
            m.col[0].v[row], m.col[1].v[row], m.col[2].v[row],
        }
    };
}


/** dst = m[:][col] (column major order) */
o_inline m_vec3 mat3_get_col(m_mat3 m, int col)
{
    assert(0 <= col && col < 3);
    return m.col[col];
}


/** dst = m; dst[row][:] = v (column major order) */
o_inline m_mat3 mat3_set_row(m_mat3 m, m_vec3 v, int row)
{
    assert(0 <= row && row < 3);
    m.col[0].v[row] = v.v0;
    m.col[1].v[row] = v.v1;
    m.col[2].v[row] = v.v2;
    return m;
}

/** m[row][:] = v; dst = m (column major order) */
o_inline m_mat3 mat3_set_this_row(m_mat3 *m, m_vec3 v, int row)
{
    assert(0 <= row && row < 3);
    m->col[0].v[row] = v.v0;
    m->col[1].v[row] = v.v1;
    m->col[2].v[row] = v.v2;
    return *m;
}

/** dst = m; dst[:][col] = v (column major order) */
o_inline m_mat3 mat3_set_col(m_mat3 m, m_vec3 v, int col)
{
    assert(0 <= col && col < 3);
    m.col[col] = v;
    return m;
}

/** mat[:][col] = v; dst = m (column major order) */
o_inline m_mat3 mat3_set_this_col(m_mat3 *m, m_vec3 v, int col)
{
    assert(0 <= col && col < 3);
    m->col[col] = v;
    return *m;
}


/** returns sum of diagonal form upper left to lower right */
o_inline float mat3_trace(m_mat3 m)
{
    return m.m00 + m.m11 + m.m22;
}


/** dst = m^t */
o_inline m_mat3 mat3_transpose(m_mat3 m)
{
    return (m_mat3){
        {
            m.m00, m.m10, m.m20,
            m.m01, m.m11, m.m21,
            m.m02, m.m12, m.m22,
        }
    };
}


/** dst = a @ b */
o_inline m_mat3 mat3_mul_mat(m_mat3 a, m_mat3 b)
{
    return (m_mat3){
        {
            a.v0 * b.v0 + a.v3 * b.v1 + a.v6 * b.v2,
            a.v1 * b.v0 + a.v4 * b.v1 + a.v7 * b.v2,
            a.v2 * b.v0 + a.v5 * b.v1 + a.v8 * b.v2,

            a.v0 * b.v3 + a.v3 * b.v4 + a.v6 * b.v5,
            a.v1 * b.v3 + a.v4 * b.v4 + a.v7 * b.v5,
            a.v2 * b.v3 + a.v5 * b.v4 + a.v8 * b.v5,

            a.v0 * b.v6 + a.v3 * b.v7 + a.v6 * b.v8,
            a.v1 * b.v6 + a.v4 * b.v7 + a.v7 * b.v8,
            a.v2 * b.v6 + a.v5 * b.v7 + a.v8 * b.v8
        }
    };
}


/** dst = a @ b */
o_inline m_vec3 mat3_mul_vec(m_mat3 a, m_vec3 b)
{
    return (m_vec3){
        {
            a.v0 * b.v0 + a.v3 * b.v1 + a.v6 * b.v2,
            a.v1 * b.v0 + a.v4 * b.v1 + a.v7 * b.v2,
            a.v2 * b.v0 + a.v5 * b.v1 + a.v8 * b.v2
        }
    };
}


/** dst = a @ b */
o_inline m_vec3 vec3_mul_mat(m_vec3 a, m_mat3 b)
{
    return (m_vec3){
        {
            b.v0 * a.v0 + b.v1 * a.v1 + b.v2 * a.v2,
            b.v3 * a.v0 + b.v4 * a.v1 + b.v5 * a.v2,
            b.v6 * a.v0 + b.v7 * a.v1 + b.v8 * a.v2
        }
    };
}


/** returns = determinant mm */
o_inline float mat3_det(m_mat3 mat)
{
    // from cglm/mat3.h/glm_mat3_det
    float a = mat.m[0][0], b = mat.m[0][1], c = mat.m[0][2];
    float d = mat.m[1][0], e = mat.m[1][1], f = mat.m[1][2];
    float g = mat.m[2][0], h = mat.m[2][1], i = mat.m[2][2];

    return a * (e * i - h * f) - d * (b * i - c * h) + g * (b * f - c * e);
}


/** dst = inverted mm */
static m_mat3 mat3_inv(m_mat3 mat)
{
    m_mat3 res;

    // from cglm/mat3.h/glm_mat3_inv
    float a = mat.m[0][0], b = mat.m[0][1], c = mat.m[0][2];
    float d = mat.m[1][0], e = mat.m[1][1], f = mat.m[1][2];
    float g = mat.m[2][0], h = mat.m[2][1], i = mat.m[2][2];

    res.m[0][0] = e * i - f * h;
    res.m[0][1] = -(b * i - h * c);
    res.m[0][2] = b * f - e * c;
    res.m[1][0] = -(d * i - g * f);
    res.m[1][1] = a * i - c * g;
    res.m[1][2] = -(a * f - d * c);
    res.m[2][0] = d * h - g * e;
    res.m[2][1] = -(a * h - g * b);
    res.m[2][2] = a * e - b * d;

    float det = a * res.m[0][0] + b * res.m[1][0] + c * res.m[2][0];
    float inv_det = 1.0f / det;
    for (int vi = 0; vi < 9; vi++) {
        res.v[vi] *= inv_det;
    }
    return res;
}

/** dst = m[col:col+2, row:row+2] */
o_inline m_mat2 mat3_get_block2(m_mat3 m, int row, int col)
{
    assert(0 <= col && col <= 1);
    assert(0 <= row && row <= 1);
    return (m_mat2){
        {
            m.col[col + 0].v[row + 0], m.col[col + 0].v[row + 1],
            m.col[col + 1].v[row + 0], m.col[col + 1].v[row + 1]
        }
    };
}


/** dst = m; dst[col:col+2, row:row+2] = block */
o_inline m_mat3 mat3_set_block2(m_mat3 m, m_mat2 block_2, int row, int col)
{
    assert(0 <= col && col <= 1);
    assert(0 <= row && row <= 1);
    m.col[col + 0].v[row + 0] = block_2.m00;
    m.col[col + 0].v[row + 1] = block_2.m01;
    m.col[col + 1].v[row + 0] = block_2.m10;
    m.col[col + 1].v[row + 1] = block_2.m11;
    return m;
}


/** m[col:col+2, row:row+2] = block; dst = m */
o_inline m_mat3 mat3_set_this_block2(m_mat3 *m, m_mat2 block_2, int row, int col)
{
    assert(0 <= col && col <= 1);
    assert(0 <= row && row <= 1);
    m->col[col + 0].v[row + 0] = block_2.m00;
    m->col[col + 0].v[row + 1] = block_2.m01;
    m->col[col + 1].v[row + 0] = block_2.m10;
    m->col[col + 1].v[row + 1] = block_2.m11;
    return *m;
}


/** dst = m[:2, :2] */
o_inline m_mat2 mat3_get_upper_left2(m_mat3 m)
{
    return (m_mat2){
        {
            m.m00, m.m01,
            m.m10, m.m11,
        }
    };
}


/** dst = m; dst[:2, :2] = block */
o_inline m_mat3 mat3_set_upper_left2(m_mat3 m, m_mat2 block_2)
{
    m.m00 = block_2.m00;
    m.m01 = block_2.m01;
    m.m10 = block_2.m10;
    m.m11 = block_2.m11;
    return m;
}

/** m[:2, :2] = block; dst = m */
o_inline m_mat3 mat3_set_this_upper_left2(m_mat3 *m, m_mat2 block_2)
{
    m->m00 = block_2.m00;
    m->m01 = block_2.m01;
    m->m10 = block_2.m10;
    m->m11 = block_2.m11;
    return *m;
}

/** rotation matrix around the x-axis */
o_inline m_mat3 mat3_rx(float angle_rad)
{
    float c = m_cos(angle_rad);
    float s = m_sin(angle_rad);
    return (m_mat3) {
        {
            1, 0, 0,
            0, c, s,
            0,-s, c
        }
    };
}

/** rotation matrix around the y-axis */
o_inline m_mat3 mat3_ry(float angle_rad)
{
    float c = m_cos(angle_rad);
    float s = m_sin(angle_rad);
    return (m_mat3) {
        {
            c, 0,-s,
            0, 1, 0,
            s, 0, c
        }
    };
}

/** rotation matrix around the z-axis */
o_inline m_mat3 mat3_rz(float angle_rad)
{
    float c = m_cos(angle_rad);
    float s = m_sin(angle_rad);
    return (m_mat3) {
            {
                c, s, 0,
                -s,c, 0,
                0, 0, 1
            }
    };
}

#endif //M_MAT_MAT3_H
