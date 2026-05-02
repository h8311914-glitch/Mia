#ifndef M_MAT_MAT4_H
#define M_MAT_MAT4_H

/**
 * @file mat/m_mat4.h
 *
 * functions working with m_mat4
 */


#include "m/m_types/flt.h"
#include "m/sca/flt.h"


/** dst = v0, v1, ... */
o_inline m_mat4 mat4_new(float v0, float v1, float v2, float v3,
                float v4, float v5, float v6, float v7,
                float v8, float v9, float v10, float v11,
                float v12, float v13, float v14, float v15)
{
    return (m_mat4){
        {
            v0, v1, v2, v3,
            v4, v5, v6, v7,
            v8, v9, v10, v11,
            v12, v13, v14, v15
        }
    };
}


/** dst = (m_mat4) v */
o_inline m_mat4 mat4_cast_double(const double *cast)
{
    return (m_mat4){
        {
            cast[0], cast[1], cast[2], cast[3],
            cast[4], cast[5], cast[6], cast[7],
            cast[8], cast[9], cast[10], cast[11],
            cast[12], cast[13], cast[14], cast[15]
        }
    };
}

/** dst = r==c ? 1 : 0 (identity)  */
o_inline m_mat4 mat4_eye(void)
{
    return (m_mat4){
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        }
    };
}

/** dst = m[:][row] (column major order) */
o_inline m_vec4 mat4_get_row(m_mat4 m, int row)
{
    assert(0 <= row && row < 4);
    return (m_vec4){
        {
            m.col[0].v[row], m.col[1].v[row], m.col[2].v[row], m.col[3].v[row]
        }
    };
}


/** dst = m[col][:] (column major order)*/
o_inline m_vec4 mat4_get_col(m_mat4 m, int col)
{
    assert(0 <= col && col < 4);
    return m.col[col];
}


/** dst = m; dst[:][row] = v (column major order)*/
o_inline m_mat4 mat4_set_row(m_mat4 m, m_vec4 v, int row)
{
    assert(0 <= row && row < 4);
    m.col[0].v[row] = v.v0;
    m.col[1].v[row] = v.v1;
    m.col[2].v[row] = v.v2;
    m.col[3].v[row] = v.v3;
    return m;
}

/** m[:][row] = v; dst = m (column major order)*/
o_inline m_mat4 mat4_set_this_row(m_mat4 *m, m_vec4 v, int row)
{
    assert(0 <= row && row < 4);
    m->col[0].v[row] = v.v0;
    m->col[1].v[row] = v.v1;
    m->col[2].v[row] = v.v2;
    m->col[3].v[row] = v.v3;
    return *m;
}

/** dst = m; dst[:][col] = v (column major order)*/
o_inline m_mat4 mat4_set_col(m_mat4 m, m_vec4 v, int col)
{
    assert(0 <= col && col < 4);
    m.col[col] = v;
    return m;
}

/** mat[:][col] = v; dst = m (column major order)*/
o_inline m_mat4 mat4_set_this_col(m_mat4 *m, m_vec4 v, int col)
{
    assert(0 <= col && col < 4);
    m->col[col] = v;
    return *m;
}


/** returns sum of diagonal form upper left to lower right */
o_inline float mat4_trace(m_mat4 m)
{
    return m.m00 + m.m11 + m.m22 + m.m33;
}


/** dst = m^t */
o_inline m_mat4 mat4_transpose(m_mat4 m)
{
    return (m_mat4){
        {
            m.m00, m.m10, m.m20, m.m30,
            m.m01, m.m11, m.m21, m.m31,
            m.m02, m.m12, m.m22, m.m32,
            m.m03, m.m13, m.m23, m.m33
        }
    };
}


/** dst = a @ b */
o_inline m_mat4 mat4_mul_mat(m_mat4 a, m_mat4 b)
{
    return (m_mat4){
        {
            a.v0 * b.v0 + a.v4 * b.v1 + a.v8 * b.v2 + a.v12 * b.v3,
            a.v1 * b.v0 + a.v5 * b.v1 + a.v9 * b.v2 + a.v13 * b.v3,
            a.v2 * b.v0 + a.v6 * b.v1 + a.v10 * b.v2 + a.v14 * b.v3,
            a.v3 * b.v0 + a.v7 * b.v1 + a.v11 * b.v2 + a.v15 * b.v3,

            a.v0 * b.v4 + a.v4 * b.v5 + a.v8 * b.v6 + a.v12 * b.v7,
            a.v1 * b.v4 + a.v5 * b.v5 + a.v9 * b.v6 + a.v13 * b.v7,
            a.v2 * b.v4 + a.v6 * b.v5 + a.v10 * b.v6 + a.v14 * b.v7,
            a.v3 * b.v4 + a.v7 * b.v5 + a.v11 * b.v6 + a.v15 * b.v7,

            a.v0 * b.v8 + a.v4 * b.v9 + a.v8 * b.v10 + a.v12 * b.v11,
            a.v1 * b.v8 + a.v5 * b.v9 + a.v9 * b.v10 + a.v13 * b.v11,
            a.v2 * b.v8 + a.v6 * b.v9 + a.v10 * b.v10 + a.v14 * b.v11,
            a.v3 * b.v8 + a.v7 * b.v9 + a.v11 * b.v10 + a.v15 * b.v11,

            a.v0 * b.v12 + a.v4 * b.v13 + a.v8 * b.v14 + a.v12 * b.v15,
            a.v1 * b.v12 + a.v5 * b.v13 + a.v9 * b.v14 + a.v13 * b.v15,
            a.v2 * b.v12 + a.v6 * b.v13 + a.v10 * b.v14 + a.v14 * b.v15,
            a.v3 * b.v12 + a.v7 * b.v13 + a.v11 * b.v14 + a.v15 * b.v15,
        }
    };
}


/** dst = a @ b */
o_inline m_vec4 mat4_mul_vec(m_mat4 a, m_vec4 b)
{
    return (m_vec4){
        {
            a.v0 * b.v0 + a.v4 * b.v1 + a.v8 * b.v2 + a.v12 * b.v3,
            a.v1 * b.v0 + a.v5 * b.v1 + a.v9 * b.v2 + a.v13 * b.v3,
            a.v2 * b.v0 + a.v6 * b.v1 + a.v10 * b.v2 + a.v14 * b.v3,
            a.v3 * b.v0 + a.v7 * b.v1 + a.v11 * b.v2 + a.v15 * b.v3
        }
    };
}


/** dst = a @ b */
o_inline m_vec4 vec4_mul_mat(m_vec4 a, m_mat4 b)
{
    return (m_vec4){
        {
            b.v0 * a.v0 + b.v1 * a.v1 + b.v2 * a.v2 + b.v3 * a.v3,
            b.v4 * a.v0 + b.v5 * a.v1 + b.v6 * a.v2 + b.v7 * a.v3,
            b.v8 * a.v0 + b.v9 * a.v1 + b.v10 * a.v2 + b.v11 * a.v3,
            b.v12 * a.v0 + b.v13 * a.v1 + b.v14 * a.v2 + b.v15 * a.v3
        }
    };
}


/** returns = determinant mm */
o_inline float mat4_det(m_mat4 mat)
{
    // from cglm/mat4.h/glm_mat4_det
    /* [square] det(A) = det(At) */
    float t[6];
    float a = mat.m[0][0], b = mat.m[0][1], c = mat.m[0][2], d = mat.m[0][3],
            e = mat.m[1][0], f = mat.m[1][1], g = mat.m[1][2], h = mat.m[1][3],
            i = mat.m[2][0], j = mat.m[2][1], k = mat.m[2][2], l = mat.m[2][3],
            m = mat.m[3][0], n = mat.m[3][1], o = mat.m[3][2], p = mat.m[3][3];

    t[0] = k * p - o * l;
    t[1] = j * p - n * l;
    t[2] = j * o - n * k;
    t[3] = i * p - m * l;
    t[4] = i * o - m * k;
    t[5] = i * n - m * j;

    return a * (f * t[0] - g * t[1] + h * t[2])
           - b * (e * t[0] - g * t[3] + h * t[4])
           + c * (e * t[1] - f * t[3] + h * t[5])
           - d * (e * t[2] - f * t[4] + g * t[5]);
}


/** dst = inverted mm */
static m_mat4 mat4_inv(m_mat4 mat)
{
    m_mat4 res;
    // from cglm/mat4.h/glm_mat4_inv
    float t[6];
    float a = mat.m[0][0], b = mat.m[0][1], c = mat.m[0][2], d = mat.m[0][3];
    float e = mat.m[1][0], f = mat.m[1][1], g = mat.m[1][2], h = mat.m[1][3];
    float i = mat.m[2][0], j = mat.m[2][1], k = mat.m[2][2], l = mat.m[2][3];
    float m = mat.m[3][0], n = mat.m[3][1], o = mat.m[3][2], p = mat.m[3][3];

    t[0] = k * p - o * l;
    t[1] = j * p - n * l;
    t[2] = j * o - n * k;
    t[3] = i * p - m * l;
    t[4] = i * o - m * k;
    t[5] = i * n - m * j;

    res.m[0][0] = f * t[0] - g * t[1] + h * t[2];
    res.m[1][0] = -(e * t[0] - g * t[3] + h * t[4]);
    res.m[2][0] = e * t[1] - f * t[3] + h * t[5];
    res.m[3][0] = -(e * t[2] - f * t[4] + g * t[5]);

    res.m[0][1] = -(b * t[0] - c * t[1] + d * t[2]);
    res.m[1][1] = a * t[0] - c * t[3] + d * t[4];
    res.m[2][1] = -(a * t[1] - b * t[3] + d * t[5]);
    res.m[3][1] = a * t[2] - b * t[4] + c * t[5];

    t[0] = g * p - o * h;
    t[1] = f * p - n * h;
    t[2] = f * o - n * g;
    t[3] = e * p - m * h;
    t[4] = e * o - m * g;
    t[5] = e * n - m * f;

    res.m[0][2] = b * t[0] - c * t[1] + d * t[2];
    res.m[1][2] = -(a * t[0] - c * t[3] + d * t[4]);
    res.m[2][2] = a * t[1] - b * t[3] + d * t[5];
    res.m[3][2] = -(a * t[2] - b * t[4] + c * t[5]);

    t[0] = g * l - k * h;
    t[1] = f * l - j * h;
    t[2] = f * k - j * g;
    t[3] = e * l - i * h;
    t[4] = e * k - i * g;
    t[5] = e * j - i * f;

    res.m[0][3] = -(b * t[0] - c * t[1] + d * t[2]);
    res.m[1][3] = a * t[0] - c * t[3] + d * t[4];
    res.m[2][3] = -(a * t[1] - b * t[3] + d * t[5]);
    res.m[3][3] = a * t[2] - b * t[4] + c * t[5];

    float det = a * res.m[0][0] + b * res.m[1][0] + c * res.m[2][0] + d * res.m[3][0];
    float inv_det = 1.0f / det;
    for (int vi = 0; vi < 16; vi++) {
        res.v[vi] *= inv_det;
    }
    return res;
}

/** dst = m[col:col+2, row:row+2] */
o_inline m_mat2 mat4_get_block2(m_mat4 m, int row, int col)
{
    assert(0 <= col && col <= 2);
    assert(0 <= row && row <= 2);
    return (m_mat2){
        {
            m.col[col + 0].v[row + 0], m.col[col + 0].v[row + 1],
            m.col[col + 1].v[row + 0], m.col[col + 1].v[row + 1]
        }
    };
}


/** dst = m; dst[col:col+2, row:row+2] = block */
o_inline m_mat4 mat4_set_block2(m_mat4 m, m_mat2 block_2, int row, int col)
{
    assert(0 <= col && col <= 2);
    assert(0 <= row && row <= 2);
    m.col[col + 0].v[row + 0] = block_2.m00;
    m.col[col + 0].v[row + 1] = block_2.m01;
    m.col[col + 1].v[row + 0] = block_2.m10;
    m.col[col + 1].v[row + 1] = block_2.m11;
    return m;
}


/** m[col:col+2, row:row+2] = block; dst = m */
o_inline m_mat4 mat4_set_this_block2(m_mat4 *m, m_mat2 block_2, int row, int col)
{
    assert(0 <= col && col <= 2);
    assert(0 <= row && row <= 2);
    m->col[col + 0].v[row + 0] = block_2.m00;
    m->col[col + 0].v[row + 1] = block_2.m01;
    m->col[col + 1].v[row + 0] = block_2.m10;
    m->col[col + 1].v[row + 1] = block_2.m11;
    return *m;
}


/** dst = m[col:col+2, row:row+2] */
o_inline m_mat3 mat4_get_block3(m_mat4 m, int row, int col)
{
    assert(0 <= col && col <= 1);
    assert(0 <= row && row <= 1);
    return (m_mat3){
        {
            m.col[col + 0].v[row + 0], m.col[col + 0].v[row + 1], m.col[col + 0].v[row + 2],
            m.col[col + 1].v[row + 0], m.col[col + 1].v[row + 1], m.col[col + 1].v[row + 2],
            m.col[col + 2].v[row + 0], m.col[col + 2].v[row + 1], m.col[col + 2].v[row + 2]
        }
    };
}


/** dst = m; dst[col:col+2, row:row+2] = block */
o_inline m_mat4 mat4_set_block3(m_mat4 m, m_mat3 block_3, int row, int col)
{
    assert(0 <= col && col <= 1);
    assert(0 <= row && row <= 1);
    m.col[col + 0].v[row + 0] = block_3.m00;
    m.col[col + 0].v[row + 1] = block_3.m01;
    m.col[col + 0].v[row + 2] = block_3.m02;
    m.col[col + 1].v[row + 0] = block_3.m10;
    m.col[col + 1].v[row + 1] = block_3.m11;
    m.col[col + 1].v[row + 2] = block_3.m12;
    m.col[col + 2].v[row + 0] = block_3.m20;
    m.col[col + 2].v[row + 1] = block_3.m21;
    m.col[col + 2].v[row + 2] = block_3.m22;
    return m;
}

/** m[col:col+2, row:row+2] = block; dst = m */
o_inline m_mat4 mat4_set_this_block3(m_mat4 *m, m_mat3 block_3, int row, int col)
{
    assert(0 <= col && col <= 1);
    assert(0 <= row && row <= 1);
    m->col[col + 0].v[row + 0] = block_3.m00;
    m->col[col + 0].v[row + 1] = block_3.m01;
    m->col[col + 0].v[row + 2] = block_3.m02;
    m->col[col + 1].v[row + 0] = block_3.m10;
    m->col[col + 1].v[row + 1] = block_3.m11;
    m->col[col + 1].v[row + 2] = block_3.m12;
    m->col[col + 2].v[row + 0] = block_3.m20;
    m->col[col + 2].v[row + 1] = block_3.m21;
    m->col[col + 2].v[row + 2] = block_3.m22;
    return *m;
}


/** dst = m[:2, :2] */
o_inline m_mat2 mat4_get_upper_left2(m_mat4 m)
{
    return (m_mat2){
        {
            m.m00, m.m01,
            m.m10, m.m11,
        }
    };
}


/** dst = m; dst[:2, :2] = block */
o_inline m_mat4 mat4_set_upper_left2(m_mat4 m, m_mat2 block_2)
{
    m.m00 = block_2.m00;
    m.m01 = block_2.m01;
    m.m10 = block_2.m10;
    m.m11 = block_2.m11;
    return m;
}

/** m[:2, :2] = block; dst = m */
o_inline m_mat4 mat4_set_this_upper_left2(m_mat4 *m, m_mat2 block_2)
{
    m->m00 = block_2.m00;
    m->m01 = block_2.m01;
    m->m10 = block_2.m10;
    m->m11 = block_2.m11;
    return *m;
}


/** dst = m[:3, :3] */
o_inline m_mat3 mat4_get_upper_left3(m_mat4 m)
{
    return (m_mat3){
        {
            m.m00, m.m01, m.m02,
            m.m10, m.m11, m.m12,
            m.m20, m.m21, m.m22,
        }
    };
}


/** dst = m; dst[:3, :3] = block */
o_inline m_mat4 mat4_set_upper_left3(m_mat4 m, m_mat3 block_3)
{
    m.m00 = block_3.m00;
    m.m01 = block_3.m01;
    m.m02 = block_3.m02;
    m.m10 = block_3.m10;
    m.m11 = block_3.m11;
    m.m12 = block_3.m12;
    m.m20 = block_3.m20;
    m.m21 = block_3.m21;
    m.m22 = block_3.m22;
    return m;
}

/** m[:3, :3] = block; dst = m */
o_inline m_mat4 mat4_set_this_upper_left3(m_mat4 *m, m_mat3 block_3)
{
    m->m00 = block_3.m00;
    m->m01 = block_3.m01;
    m->m02 = block_3.m02;
    m->m10 = block_3.m10;
    m->m11 = block_3.m11;
    m->m12 = block_3.m12;
    m->m20 = block_3.m20;
    m->m21 = block_3.m21;
    m->m22 = block_3.m22;
    return *m;
}

/** rotation matrix around the x-axis */
o_inline m_mat4 mat4_rx(float angle_rad)
{
    float c = m_cos(angle_rad);
    float s = m_sin(angle_rad);
    return (m_mat4) {
            {
                1, 0, 0, 0,
                0, c, s, 0,
                0,-s, c, 0,
                0, 0, 0, 1
            }
    };
}

/** rotation matrix around the y-axis */
o_inline m_mat4 mat4_ry(float angle_rad)
{
    float c = m_cos(angle_rad);
    float s = m_sin(angle_rad);
    return (m_mat4) {
            {
                c, 0,-s, 0,
                0, 1, 0, 0,
                s, 0, c, 0,
                0, 0, 0, 1
            }
    };
}

/** rotation matrix around the z-axis */
o_inline m_mat4 mat4_rz(float angle_rad)
{
    float c = m_cos(angle_rad);
    float s = m_sin(angle_rad);
    return (m_mat4) {
                {
                    c, s, 0, 0,
                    -s,c, 0, 0,
                    0, 0, 1, 0,
                    0, 0, 0, 1
                }
    };
}

/** translation matrix along the x-axis */
o_inline m_mat4 mat4_tx(float t)
{
    return (m_mat4) {
                {
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    t, 0, 0, 1
                }
    };
}

/** translation matrix along the y-axis */
o_inline m_mat4 mat4_ty(float t)
{
    return (m_mat4) {
                {
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, t, 0, 1
                }
    };
}

/** translation matrix along the z-axis */
o_inline m_mat4 mat4_tz(float t)
{
    return (m_mat4) {
                {
                    1, 0, 0, 0,
                    0, 1, 0, 0,
                    0, 0, 1, 0,
                    0, 0, t, 1
                }
    };
}

#endif //M_MAT_MAT4_H
