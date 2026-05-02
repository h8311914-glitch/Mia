#ifndef M_MAT_DMAT3_H
#define M_MAT_DMAT3_H

/**
 * @file mat/m_dmat3.h
 *
 * functions working with m_dmat3
 */


#include "m/m_types/dbl.h"
#include "m/sca/dbl.h"


/** dst = v0, v1, ... */
o_inline m_dmat3 dmat3_new(double v0, double v1, double v2,
                  double v3, double v4, double v5,
                  double v6, double v7, double v8)
{
    return (m_dmat3){
        {
            v0, v1, v2,
            v3, v4, v5,
            v6, v7, v8
        }
    };
}


/** dst = (m_dmat3) v */
o_inline m_dmat3 dmat3_cast_float(const float *cast)
{
    return (m_dmat3){
        {
            cast[0], cast[1], cast[2],
            cast[3], cast[4], cast[5],
            cast[6], cast[7], cast[8],
        }
    };
}

/** dst = r==c ? 1 : 0 (identity)  */
o_inline m_dmat3 dmat3_eye(void)
{
    return (m_dmat3){
        {
            1, 0, 0,
            0, 1, 0,
            0, 0, 1,
        }
    };
}

/** dst = m[row][:] */
o_inline m_dvec3 dmat3_get_row(m_dmat3 m, int row)
{
    assert(0 <= row && row < 3);
    return (m_dvec3){
        {
            m.col[0].v[row], m.col[1].v[row], m.col[2].v[row],
        }
    };
}


/** dst = m[:][col] (column major order) */
o_inline m_dvec3 dmat3_get_col(m_dmat3 m, int col)
{
    assert(0 <= col && col < 3);
    return m.col[col];
}


/** dst = m; dst[row][:] = v (column major order) */
o_inline m_dmat3 dmat3_set_row(m_dmat3 m, m_dvec3 v, int row)
{
    assert(0 <= row && row < 3);
    m.col[0].v[row] = v.v0;
    m.col[1].v[row] = v.v1;
    m.col[2].v[row] = v.v2;
    return m;
}

/** m[row][:] = v; dst = m (column major order) */
o_inline m_dmat3 dmat3_set_this_row(m_dmat3 *m, m_dvec3 v, int row)
{
    assert(0 <= row && row < 3);
    m->col[0].v[row] = v.v0;
    m->col[1].v[row] = v.v1;
    m->col[2].v[row] = v.v2;
    return *m;
}

/** dst = m; dst[:][col] = v (column major order) */
o_inline m_dmat3 dmat3_set_col(m_dmat3 m, m_dvec3 v, int col)
{
    assert(0 <= col && col < 3);
    m.col[col] = v;
    return m;
}

/** dmat[:][col] = v; dst = m (column major order) */
o_inline m_dmat3 dmat3_set_this_col(m_dmat3 *m, m_dvec3 v, int col)
{
    assert(0 <= col && col < 3);
    m->col[col] = v;
    return *m;
}


/** returns sum of diagonal form upper left to lower right */
o_inline double dmat3_trace(m_dmat3 m)
{
    return m.m00 + m.m11 + m.m22;
}


/** dst = m^t */
o_inline m_dmat3 dmat3_transpose(m_dmat3 m)
{
    return (m_dmat3){
        {
            m.m00, m.m10, m.m20,
            m.m01, m.m11, m.m21,
            m.m02, m.m12, m.m22,
        }
    };
}


/** dst = a @ b */
o_inline m_dmat3 dmat3_mul_mat(m_dmat3 a, m_dmat3 b)
{
    return (m_dmat3){
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
o_inline m_dvec3 dmat3_mul_vec(m_dmat3 a, m_dvec3 b)
{
    return (m_dvec3){
        {
            a.v0 * b.v0 + a.v3 * b.v1 + a.v6 * b.v2,
            a.v1 * b.v0 + a.v4 * b.v1 + a.v7 * b.v2,
            a.v2 * b.v0 + a.v5 * b.v1 + a.v8 * b.v2
        }
    };
}


/** dst = a @ b */
o_inline m_dvec3 dvec3_mul_mat(m_dvec3 a, m_dmat3 b)
{
    return (m_dvec3){
        {
            b.v0 * a.v0 + b.v1 * a.v1 + b.v2 * a.v2,
            b.v3 * a.v0 + b.v4 * a.v1 + b.v5 * a.v2,
            b.v6 * a.v0 + b.v7 * a.v1 + b.v8 * a.v2
        }
    };
}


/** returns = determinant mm */
o_inline double dmat3_det(m_dmat3 mat)
{
    // from cglm/mat3.h/glm_mat3_det
    double a = mat.m[0][0], b = mat.m[0][1], c = mat.m[0][2];
    double d = mat.m[1][0], e = mat.m[1][1], f = mat.m[1][2];
    double g = mat.m[2][0], h = mat.m[2][1], i = mat.m[2][2];

    return a * (e * i - h * f) - d * (b * i - c * h) + g * (b * f - c * e);
}


/** dst = inverted mm */
static m_dmat3 dmat3_inv(m_dmat3 mat)
{
    m_dmat3 res;

    // from cglm/mat3.h/glm_mat3_inv
    double a = mat.m[0][0], b = mat.m[0][1], c = mat.m[0][2];
    double d = mat.m[1][0], e = mat.m[1][1], f = mat.m[1][2];
    double g = mat.m[2][0], h = mat.m[2][1], i = mat.m[2][2];

    res.m[0][0] = e * i - f * h;
    res.m[0][1] = -(b * i - h * c);
    res.m[0][2] = b * f - e * c;
    res.m[1][0] = -(d * i - g * f);
    res.m[1][1] = a * i - c * g;
    res.m[1][2] = -(a * f - d * c);
    res.m[2][0] = d * h - g * e;
    res.m[2][1] = -(a * h - g * b);
    res.m[2][2] = a * e - b * d;

    double det = a * res.m[0][0] + b * res.m[1][0] + c * res.m[2][0];
    double inv_det = 1.0 / det;
    for (int vi = 0; vi < 9; vi++) {
        res.v[vi] *= inv_det;
    }
    return res;
}

/** dst = m[col:col+2, row:row+2] */
o_inline m_dmat2 dmat3_get_block2(m_dmat3 m, int row, int col)
{
    assert(0 <= col && col <= 1);
    assert(0 <= row && row <= 1);
    return (m_dmat2){
        {
            m.col[col + 0].v[row + 0], m.col[col + 0].v[row + 1],
            m.col[col + 1].v[row + 0], m.col[col + 1].v[row + 1]
        }
    };
}


/** dst = m; dst[col:col+2, row:row+2] = block */
o_inline m_dmat3 dmat3_set_block2(m_dmat3 m, m_dmat2 block_2, int row, int col)
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
o_inline m_dmat3 dmat3_set_this_block2(m_dmat3 *m, m_dmat2 block_2, int row, int col)
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
o_inline m_dmat2 dmat3_get_upper_left2(m_dmat3 m)
{
    return (m_dmat2){
        {
            m.m00, m.m01,
            m.m10, m.m11,
        }
    };
}


/** dst = m; dst[:2, :2] = block */
o_inline m_dmat3 dmat3_set_upper_left2(m_dmat3 m, m_dmat2 block_2)
{
    m.m00 = block_2.m00;
    m.m01 = block_2.m01;
    m.m10 = block_2.m10;
    m.m11 = block_2.m11;
    return m;
}

/** m[:2, :2] = block; dst = m */
o_inline m_dmat3 dmat3_set_this_upper_left2(m_dmat3 *m, m_dmat2 block_2)
{
    m->m00 = block_2.m00;
    m->m01 = block_2.m01;
    m->m10 = block_2.m10;
    m->m11 = block_2.m11;
    return *m;
}

/** rotation matrix around the x-axis */
o_inline m_dmat3 dmat3_rx(double angle_rad)
{
    double c = md_cos(angle_rad);
    double s = md_sin(angle_rad);
    return (m_dmat3) {
            {
                1, 0, 0,
                0, c, s,
                0,-s, c
            }
    };
}

/** rotation matrix around the y-axis */
o_inline m_dmat3 dmat3_ry(double angle_rad)
{
    double c = md_cos(angle_rad);
    double s = md_sin(angle_rad);
    return (m_dmat3) {
            {
                c, 0,-s,
                0, 1, 0,
                s, 0, c
            }
    };
}

/** rotation matrix around the z-axis */
o_inline m_dmat3 dmat3_rz(double angle_rad)
{
    double c = md_cos(angle_rad);
    double s = md_sin(angle_rad);
    return (m_dmat3) {
                {
                    c, s, 0,
                    -s,c, 0,
                    0, 0, 1
                }
    };
}


#endif //M_MAT_DMAT3_H
