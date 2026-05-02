#ifndef M_MAT_DMAT2_H
#define M_MAT_DMAT2_H

/**
 * @file mat/m_dmat2.h
 *
 * functions working with m_dmat2
 */


#include "m/m_types/dbl.h"


/** dst = v0, v1, ... */
o_inline m_dmat2 dmat2_new(double v0, double v1,
                  double v2, double v3)
{
    return (m_dmat2){
        {
            v0, v1,
            v2, v3
        }
    };
}


/** dst = (m_dmat2) v */
o_inline m_dmat2 dmat2_cast_float(const float *cast)
{
    return (m_dmat2){
        {
            cast[0], cast[1],
            cast[2], cast[3]
        }
    };
}

/** dst = r==c ? 1 : 0 (identity)  */
o_inline m_dmat2 dmat2_eye(void)
{
    return (m_dmat2){
        {
            1, 0,
            0, 1
        }
    };
}

/** dst = m[row][:] */
o_inline m_dvec2 dmat2_get_row(m_dmat2 m, int row)
{
    assert(0 <= row && row < 2);
    return (m_dvec2){
        {
            m.col[0].v[row], m.col[1].v[row]
        }
    };
}


/** dst = m[:][col] */
o_inline m_dvec2 dmat2_get_col(m_dmat2 m, int col)
{
    assert(0 <= col && col < 2);
    return m.col[col];
}


/** dst = m; dst[row][:] = v */
o_inline m_dmat2 dmat2_set_row(m_dmat2 m, m_dvec2 v, int row)
{
    assert(0 <= row && row < 2);
    m.col[0].v[row] = v.v0;
    m.col[1].v[row] = v.v1;
    return m;
}

/** m[row][:] = v; dst = m */
o_inline m_dmat2 dmat2_set_this_row(m_dmat2 *m, m_dvec2 v, int row)
{
    assert(0 <= row && row < 2);
    m->col[0].v[row] = v.v0;
    m->col[1].v[row] = v.v1;
    return *m;
}

/** dst = m; dst[:][col] = v */
o_inline m_dmat2 dmat2_set_col(m_dmat2 m, m_dvec2 v, int col)
{
    assert(0 <= col && col < 2);
    m.col[col] = v;
    return m;
}

/** dmat[:][col] = v; dst = m */
o_inline m_dmat2 dmat2_set_this_col(m_dmat2 *m, m_dvec2 v, int col)
{
    assert(0 <= col && col < 2);
    m->col[col] = v;
    return *m;
}


/** returns sum of diagonal form upper left to lower right */
o_inline double dmat2_trace(m_dmat2 m)
{
    return m.m00 + m.m11;
}


/** dst = m^t */
o_inline m_dmat2 dmat2_transpose(m_dmat2 m)
{
    return (m_dmat2){
        {
            m.m00, m.m10,
            m.m01, m.m11
        }
    };
}


/** dst = a @ b */
o_inline m_dmat2 dmat2_mul_mat(m_dmat2 a, m_dmat2 b)
{
    return (m_dmat2){
        {
            a.v0 * b.v0 + a.v2 * b.v1,
            a.v1 * b.v0 + a.v3 * b.v1,

            a.v0 * b.v2 + a.v2 * b.v3,
            a.v1 * b.v2 + a.v3 * b.v3
        }
    };
}


/** dst = a @ b */
o_inline m_dvec2 dmat2_mul_vec(m_dmat2 a, m_dvec2 b)
{
    return (m_dvec2){
        {
            a.v0 * b.v0 + a.v2 * b.v1,
            a.v1 * b.v0 + a.v3 * b.v1
        }
    };
}


/** dst = a @ b */
o_inline m_dvec2 dvec2_mul_mat(m_dvec2 a, m_dmat2 b)
{
    return (m_dvec2){
        {
            b.v0 * a.v0 + b.v1 * a.v1,
            b.v2 * a.v0 + b.v3 * a.v1
        }
    };
}


/** returns = determinant mm */
o_inline double dmat2_det(m_dmat2 mat)
{
    // from cglm/mat2.h/glm_mat2_det
    return mat.m[0][0] * mat.m[1][1] - mat.m[1][0] * mat.m[0][1];
}


/** dst = inverted mm */
static m_dmat2 dmat2_inv(m_dmat2 mat)
{
    m_dmat2 res;
    // from cglm/mat2.h/glm_mat2_inv
    double a = mat.m[0][0], b = mat.m[0][1];
    double c = mat.m[1][0], d = mat.m[1][1];

    res.m[0][0] = d;
    res.m[0][1] = -b;
    res.m[1][0] = -c;
    res.m[1][1] = a;

    double det = a * d - b * c;
    double inv_det = 1.0 / det;
    for (int vi = 0; vi < 4; vi++) {
        res.v[vi] *= inv_det;
    }

    return res;
}

#endif //M_MAT_DMAT2_H
