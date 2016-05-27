
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
* @file      mesh.cpp
* @brief     mesh helpers
*/

#include <mesh/gu_mesh.hpp>
#include <stdexcept>
#include <cassert>
// #include <glm/gtc/matrix_inverse.hpp>
#include <tml/mat.hpp>


namespace gu
{
    void transform(vertex_buffer_object& vbo, tml::mat4 const& t)
    {
        auto tn = tml::mat3{ tml::inverseTranspose(t) };

        for (auto& v : vbo.vb)
        {
            v.position = tml::vec3(t * tml::vec4(v.position, 1.f));
            v.normal = tn * v.normal;
            v.tangent = tn * v.tangent;
        }
    }

    aabb get_aabb(vertex_buffer_object const & vbo)
    {
        aabb box{};

        for (auto& v : vbo.vb)
        {
            for (int i = 0; i < 3; ++i)
            {
                box.a[i] = std::min(box.a[i], v.position[i]);
                box.b[i] = std::max(box.a[i], v.position[i]);
            }
        }

        return box;
    }


} // namespace

