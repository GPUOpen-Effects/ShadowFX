
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
* @file      sphere.cpp
* @brief     sphere triangle mesh
*/

#include "gu_make_ib.hpp"
#include <stdexcept>

namespace
{
    float const pi = 3.1415926536f;
    float const pi_x_2 = 2.f * pi;
    float const pi_div_2 = pi / 2.f;

    void add_circle_vb(gu::vertex_buffer& vb, float r, float y, std::size_t num_pt)
    {
        assert(num_pt > 1);
        for (std::size_t i = 0; i < num_pt; ++i)
        {
            gu::vertex v{};
            v.uv = tml::vec2{ static_cast<float>(i) / static_cast<float>(num_pt - 1), .5f * y + .5f };

            float theta = v.uv.x * pi_x_2;
            float c = std::cos(theta);
            float s = std::sin(theta);

            v.position.x = r * c;
            v.position.y = y;
            v.position.z = -r * s;

            v.normal = v.position;

            v.tangent = tml::vec3{s, 0, c};

            vb.push_back(v);
        }
    }

    gu::vertex_buffer make_sphere_vb(std::size_t num_circle, std::size_t num_pt)
    {
        assert(num_circle > 1);
        gu::vertex_buffer vb( num_circle * num_pt );
        vb.resize(0);

        float step = pi / float(num_circle - 1);
        for (std::size_t i = 0; i < num_circle; ++i)
        {
            float phi = -pi_div_2 + float(i) * step; // from -pi/2 to pi/2
            float y = std::sin(phi);
            float r = std::cos(phi);
            add_circle_vb(vb, r, y, num_pt); // TODO use one point at the poles
        }
        
        assert(vb.size() == num_circle * num_pt);

        return vb;
    }

} // namespace

namespace gu
{
    vertex_buffer_object make_sphere(std::size_t num_circle, std::size_t num_pt)
    {
        assert(num_circle > 1 && num_pt > 1);
        if (num_circle < 2 || num_pt < 2)
        {
            throw std::runtime_error{ "wrong make_sphere parameters" };
        }

        return
        {
            make_sphere_vb(num_circle, num_pt),
            make_ib(num_circle, num_pt)
        };
    }


} // namespace

