
//
// Copyright (c) 2016 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

/**
* @file      mat.hpp
* @brief     temporary math library mat definition
* @note      this simple library will be replaced in a future version
*/

#ifndef TEMPORARY_MATH_LIB_MAT_HPP
#define TEMPORARY_MATH_LIB_MAT_HPP

#include "vec.hpp"
#include <array>

namespace tml
{

    /////////////////////////////////////////////////////////////////
    // mat3
    /////////////////////////////////////////////////////////////////

    struct mat3 : std::array<vec3, 3>
    {
        explicit mat3(float i = 1)
        {
            (*this)[0][0] = (*this)[1][1] = (*this)[2][2] = i;
        }
    };

    inline vec3 operator * (mat3 const& m, vec3 const& v)
    {
        return vec3{
            m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
            m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
            m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z };
    }


    /////////////////////////////////////////////////////////////////
    // mat4
    /////////////////////////////////////////////////////////////////

    struct mat4 : std::array<vec4, 4>
    {
        explicit mat4(float i = 1)
        {
            (*this)[0][0] = (*this)[1][1] = (*this)[2][2] = (*this)[3][3] = i;
        }

        explicit mat4(
            float const & x0, float const & y0, float const & z0, float const & w0,
            float const & x1, float const & y1, float const & z1, float const & w1,
            float const & x2, float const & y2, float const & z2, float const & w2,
            float const & x3, float const & y3, float const & z3, float const & w3
            )
        {
            (*this)[0] = vec4(x0, y0, z0, w0);
            (*this)[1] = vec4(x1, y1, z1, w1);
            (*this)[2] = vec4(x2, y2, z2, w2);
            (*this)[3] = vec4(x3, y3, z3, w3);
        }

        explicit mat4(vec4 const& v1, vec4 const& v2, vec4 const& v3, vec4 const& v4)
        {
            (*this)[0] = v1;
            (*this)[1] = v2;
            (*this)[2] = v3;
            (*this)[3] = v4;
        }

        explicit operator mat3()
        {
            mat3 m3{};
            m3[0] = vec3{ (*this)[0] };
            m3[1] = vec3{ (*this)[1] };
            m3[2] = vec3{ (*this)[2] };
            return m3;
        }
    };


    inline mat4 operator / (mat4 const& m, float f)
    {
        return mat4{ m[0] / f, m[1] / f, m[2] / f, m[3] / f };
    }

    inline mat4 operator* (mat4 const& m, float f)
    {
        return mat4{ m[0] * f, m[1] * f, m[2] * f, m[3] * f };
    }

    inline vec4 operator * (mat4 const& m, vec4 const& v)
    {
       vec4 mov0(v[0]);
       vec4 mov1(v[1]);
       vec4 Mul0 = m[0] * mov0;
       vec4 Mul1 = m[1] * mov1;
       vec4 add0 = Mul0 + Mul1;
       vec4 mov2(v[2]);
       vec4 mov3(v[3]);
       vec4 Mul2 = m[2] * mov2;
       vec4 Mul3 = m[3] * mov3;
       vec4 add1 = Mul2 + Mul3;
       return add0 + add1;
    }

    inline mat4 operator* (mat4 const& m1, mat4 const& m2)
    {
        vec4 const src_a0 = m1[0];
        vec4 const src_a1 = m1[1];
        vec4 const src_a2 = m1[2];
        vec4 const src_a3 = m1[3];

        vec4 const src_b0 = m2[0];
        vec4 const src_b1 = m2[1];
        vec4 const src_b2 = m2[2];
        vec4 const src_b3 = m2[3];

        mat4 res{};
        res[0] = src_a0 * src_b0[0] + src_a1 * src_b0[1] + src_a2 * src_b0[2] + src_a3 * src_b0[3];
        res[1] = src_a0 * src_b1[0] + src_a1 * src_b1[1] + src_a2 * src_b1[2] + src_a3 * src_b1[3];
        res[2] = src_a0 * src_b2[0] + src_a1 * src_b2[1] + src_a2 * src_b2[2] + src_a3 * src_b2[3];
        res[3] = src_a0 * src_b3[0] + src_a1 * src_b3[1] + src_a2 * src_b3[2] + src_a3 * src_b3[3];
        return res;
    }

    inline mat4 inverseTranspose(mat4 const& m)
    {
        auto factor00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
        auto factor01 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
        auto factor02 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
        auto factor03 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
        auto factor04 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
        auto factor05 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
        auto factor06 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
        auto factor07 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
        auto factor08 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
        auto factor09 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
        auto factor10 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
        auto factor11 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
        auto factor12 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
        auto factor13 = m[1][2] * m[2][3] - m[2][2] * m[1][3];
        auto factor14 = m[1][1] * m[2][3] - m[2][1] * m[1][3];
        auto factor15 = m[1][1] * m[2][2] - m[2][1] * m[1][2];
        auto factor16 = m[1][0] * m[2][3] - m[2][0] * m[1][3];
        auto factor17 = m[1][0] * m[2][2] - m[2][0] * m[1][2];
        auto factor18 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

        mat4 inv;
        inv[0][0] = +(m[1][1] * factor00 - m[1][2] * factor01 + m[1][3] * factor02);
        inv[0][1] = -(m[1][0] * factor00 - m[1][2] * factor03 + m[1][3] * factor04);
        inv[0][2] = +(m[1][0] * factor01 - m[1][1] * factor03 + m[1][3] * factor05);
        inv[0][3] = -(m[1][0] * factor02 - m[1][1] * factor04 + m[1][2] * factor05);

        inv[1][0] = -(m[0][1] * factor00 - m[0][2] * factor01 + m[0][3] * factor02);
        inv[1][1] = +(m[0][0] * factor00 - m[0][2] * factor03 + m[0][3] * factor04);
        inv[1][2] = -(m[0][0] * factor01 - m[0][1] * factor03 + m[0][3] * factor05);
        inv[1][3] = +(m[0][0] * factor02 - m[0][1] * factor04 + m[0][2] * factor05);

        inv[2][0] = +(m[0][1] * factor06 - m[0][2] * factor07 + m[0][3] * factor08);
        inv[2][1] = -(m[0][0] * factor06 - m[0][2] * factor09 + m[0][3] * factor10);
        inv[2][2] = +(m[0][0] * factor11 - m[0][1] * factor09 + m[0][3] * factor12);
        inv[2][3] = -(m[0][0] * factor08 - m[0][1] * factor10 + m[0][2] * factor12);

        inv[3][0] = -(m[0][1] * factor13 - m[0][2] * factor14 + m[0][3] * factor15);
        inv[3][1] = +(m[0][0] * factor13 - m[0][2] * factor16 + m[0][3] * factor17);
        inv[3][2] = -(m[0][0] * factor14 - m[0][1] * factor16 + m[0][3] * factor18);
        inv[3][3] = +(m[0][0] * factor15 - m[0][1] * factor17 + m[0][2] * factor18);

        auto det =
            + m[0][0] * inv[0][0]
            + m[0][1] * inv[0][1]
            + m[0][2] * inv[0][2]
            + m[0][3] * inv[0][3];

        return inv / det;
    }

    inline mat4 rotate(mat4 const& m, float angle, vec3 const& v)
    {
        float const a = angle;
        float const c = std::cos(a);
        float const s = std::sin(a);

        vec3 axis(normalize(v));
        vec3 temp((1 - c) * axis);

        mat4 rot{};
        rot[0][0] = c + temp[0] * axis[0];
        rot[0][1] = 0 + temp[0] * axis[1] + s * axis[2];
        rot[0][2] = 0 + temp[0] * axis[2] - s * axis[1];

        rot[1][0] = 0 + temp[1] * axis[0] - s * axis[2];
        rot[1][1] = c + temp[1] * axis[1];
        rot[1][2] = 0 + temp[1] * axis[2] + s * axis[0];

        rot[2][0] = 0 + temp[2] * axis[0] + s * axis[1];
        rot[2][1] = 0 + temp[2] * axis[1] - s * axis[0];
        rot[2][2] = c + temp[2] * axis[2];

        mat4 res{};
        res[0] = m[0] * rot[0][0] + m[1] * rot[0][1] + m[2] * rot[0][2];
        res[1] = m[0] * rot[1][0] + m[1] * rot[1][1] + m[2] * rot[1][2];
        res[2] = m[0] * rot[2][0] + m[1] * rot[2][1] + m[2] * rot[2][2];
        res[3] = m[3];
        return res;
    }

    inline mat4 translate(mat4 const& m, vec3 const& v)
    {
        mat4 res(m);
        res[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
        return res;
    }

    inline mat4 scale(mat4 const& m, vec3 const& v)
    {
        mat4 res{};
        res[0] = m[0] * v[0];
        res[1] = m[1] * v[1];
        res[2] = m[2] * v[2];
        res[3] = m[3];
        return res;
    }

    inline mat4 transpose(mat4 const& m)
    {
        mat4 result{};
        result[0][0] = m[0][0];
        result[0][1] = m[1][0];
        result[0][2] = m[2][0];
        result[0][3] = m[3][0];

        result[1][0] = m[0][1];
        result[1][1] = m[1][1];
        result[1][2] = m[2][1];
        result[1][3] = m[3][1];

        result[2][0] = m[0][2];
        result[2][1] = m[1][2];
        result[2][2] = m[2][2];
        result[2][3] = m[3][2];

        result[3][0] = m[0][3];
        result[3][1] = m[1][3];
        result[3][2] = m[2][3];
        result[3][3] = m[3][3];
        return result;
    }

    inline mat4 inverse(mat4 const& m)
    {
        float coef00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
        float coef02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
        float coef03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

        float coef04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
        float coef06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
        float coef07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

        float coef08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
        float coef10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
        float coef11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

        float coef12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
        float coef14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
        float coef15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

        float coef16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
        float coef18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
        float coef19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

        float coef20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
        float coef22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
        float coef23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

        vec4 fac0(coef00, coef00, coef02, coef03);
        vec4 fac1(coef04, coef04, coef06, coef07);
        vec4 fac2(coef08, coef08, coef10, coef11);
        vec4 fac3(coef12, coef12, coef14, coef15);
        vec4 fac4(coef16, coef16, coef18, coef19);
        vec4 fac5(coef20, coef20, coef22, coef23);

        vec4 v0(m[1][0], m[0][0], m[0][0], m[0][0]);
        vec4 v1(m[1][1], m[0][1], m[0][1], m[0][1]);
        vec4 v2(m[1][2], m[0][2], m[0][2], m[0][2]);
        vec4 v3(m[1][3], m[0][3], m[0][3], m[0][3]);

        vec4 inv0(v1 * fac0 - v2 * fac1 + v3 * fac2);
        vec4 inv1(v0 * fac0 - v2 * fac3 + v3 * fac4);
        vec4 inv2(v0 * fac1 - v1 * fac3 + v3 * fac5);
        vec4 inv3(v0 * fac2 - v1 * fac4 + v2 * fac5);

        vec4 signA(+1, -1, +1, -1);
        vec4 signB(-1, +1, -1, +1);
        mat4 inv(inv0 * signA, inv1 * signB, inv2 * signA, inv3 * signB);

        vec4 row0(inv[0][0], inv[1][0], inv[2][0], inv[3][0]);

        vec4 dot0(m[0] * row0);
        float dot1 = (dot0.x + dot0.y) + (dot0.z + dot0.w);

        float one_over_det = 1.f / dot1;

        return inv * one_over_det;
    }

} // namespace


#endif // include guard


