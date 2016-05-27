
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
* @file      utility.hpp
* @brief     utility functions
*/

#ifndef GU_UTILITY_HPP
#define GU_UTILITY_HPP

#include <tml/mat.hpp>
#include <algorithm>
#include <array>

namespace gu
{

/**
* @brief orthographic projection parameters
*/
struct ortho_param
{
    float l; //!< l left edge
    float r; //!< r right edge
    float b; //!< b bottom edge
    float t; //!< t top edge
    float n; //!< n distance to near plane
    float f; //!< f distance to far plane
};

/**
* @brief align v to a
* @param v value to align
* @param a alignment
* @tparam T value type
* @return aligned value
*/
template <typename T>
T align(T v, T a)
{
    return (v + (a - 1)) & ~(a - 1);
}


/**
* @brief make a perspective projection matrix
* @param fov field of view
* @param a aspect ratio
* @param n distance to near plane
* @param f distance to far plane
* @return homogeneous space perspective matrix
*/
inline tml::mat4 perspective_dx(float fov, float a, float n, float f)
{
    assert(fov > 0.f && a > 0.f && f > n);
    auto d = 1.f / std::tan(fov * .5f); // focal length
    auto i = 1.f / (n - f);
    return tml::mat4
    {
        d/a, 0,   0,     0,
        0,   d,   0,     0, 
        0,   0,   f*i,   -1.f, 
        0,   0,   f*n*i, 0
    };
}


/**
* @brief get the axes aligned bounding box a clip space frustum in light space
* @param p2w inverse of the view projection transform
* @param w2l world to light transform
* @param n distance to near plane
* @param f distance to far plane
* @param zmin min z in clip space
* @param zmax max z in clip space
* @tparam T floating point type
* @return orhtographic projection matrix
*/
inline ortho_param get_light_space_frustum_bbox(tml::mat4 const& p2w, tml::mat4 const& w2l, float zmin = 0.f, float zmax = 1.f)
{
    using frustum_vertex_list = std::array<tml::vec4, 8>;
    // clip space frustum vertices
    frustum_vertex_list frustum =
    {
        tml::vec4(-1.f, -1.f, zmin, 1.f),
        tml::vec4(1.f, -1.f, zmin, 1.f),
        tml::vec4(1.f,  1.f, zmin, 1.f),
        tml::vec4(-1.f,  1.f, zmin, 1.f),

        tml::vec4(-1.f, -1.f, zmax, 1.f),
        tml::vec4(1.f, -1.f, zmax, 1.f),
        tml::vec4(1.f,  1.f, zmax, 1.f),
        tml::vec4(-1.f,  1.f, zmax, 1.f),
    };

    ortho_param res
    {
        std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),
        -std::numeric_limits<float>::max(),
        std::numeric_limits<float>::max()
    };

    for (auto const& p : frustum)
    {
        auto ws_p = p2w * p;
        assert(ws_p.w != 0.f);
        ws_p = ws_p / ws_p.w;
        auto ls_p = w2l * ws_p;
        res.l = std::min(res.l, ls_p.x);
        res.r = std::max(res.r, ls_p.x);
        res.b = std::min(res.b, ls_p.y);
        res.t = std::max(res.t, ls_p.y);
        res.n = std::max(res.n, ls_p.z); // n and f should be negative in light space
        res.f = std::min(res.f, ls_p.z);
    }

    // near and far should be positive in the ortho function
    res.n = -res.n;
    res.f = -res.f;

    return res;
}

/**
* @brief make an orhtographic projection matrix
* @param l left edge
* @param r right edge
* @param b bottom edge
* @param t top edge
* @param n distance to near plane
* @param f distance to far plane
* @return orhtographic projection matrix
*/
inline tml::mat4 ortho_dx(float l, float r, float b, float t, float n, float f)
{
    auto rl = 1.f / (r - l);
    auto tb = 1.f / (t - b);
    auto nf = 1.f / (n - f);
    auto tx = -(r + l)  * rl;
    auto ty = -(t + b) * tb;
    auto tz = n * nf;
    return tml::mat4
    {
        2.f*rl,  0,      0,   0,
        0,       2.f*tb, 0,   0,
        0,       0,      nf,  0,
        tx,      ty,     tz,  1.f
    };
}

/**
* @brief make an orhtographic projection matrix
* @param param orhtographic projection parameters
* @return orhtographic projection matrix
*/
inline tml::mat4 ortho_dx(ortho_param const& param)
{
    return ortho_dx(param.l, param.r, param.b, param.t, param.n, param.f);
}

} // namespace


#endif // include guard

