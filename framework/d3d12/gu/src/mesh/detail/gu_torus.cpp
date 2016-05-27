
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
* @file      torus.cpp
* @brief     torus triangle mesh
*/

#include "gu_make_ib.hpp"
#include <stdexcept>

namespace
{
    float const pi = 3.1415926536f;
    float const pi_x_2 = 2.f * pi;
    float const pi_div_2 = pi / 2.f;


    void add_circle_vb(gu::vertex_buffer& vb, float major, float minor, size_t circle_id, size_t num_circle, size_t num_pt)
    {
        assert(num_pt > 1);

        gu::vertex v{};
        v.uv.x = static_cast<float>(circle_id) / static_cast<float>(num_circle - 1);

        float phi = v.uv.x * pi_x_2;
        float cphi = std::cos(phi);
        float sphi = std::sin(phi);

        for (std::size_t i = 0; i < num_pt; ++i)
        {
            v.uv.y = static_cast<float>(i) / static_cast<float>(num_pt - 1);

            float theta = v.uv.y * pi_x_2;
            float ctheta = std::cos(theta);
            float stheta = std::sin(theta);

            v.normal.x = ctheta * cphi;
            v.normal.y = ctheta * sphi;
            v.normal.z = -stheta;

            v.tangent.x = -stheta * cphi;
            v.tangent.y = -stheta * sphi;
            v.tangent.z = ctheta;

            v.position = tml::vec3{ major * cphi, major * sphi, 0 } + minor * v.normal; 

            vb.push_back(v);
        }
    }

    gu::vertex_buffer make_torus_vb(float major, float minor, std::size_t num_circle, std::size_t num_pt)
    {
        assert(num_circle > 1);
        gu::vertex_buffer vb(num_circle * num_pt);
        vb.resize(0);

        for (std::size_t i = 0; i < num_circle; ++i)
        {
            add_circle_vb(vb, major, minor, i, num_circle, num_pt);
        }

        assert(vb.size() == num_circle * num_pt);

        return vb;
    }

} // namespace

namespace gu
{
    vertex_buffer_object make_torus(float major_radius, float minor_radius, std::size_t num_circle, std::size_t num_pt)
    {
        assert(num_circle > 1 && num_pt > 1);
        if (num_circle < 2 || num_pt < 2)
        {
            throw std::runtime_error{ "wrong make_torus parameters" };
        }

        return
        {
            make_torus_vb(major_radius, minor_radius, num_circle, num_pt),
            make_ib(num_circle, num_pt)
        };
    }


} // namespace

