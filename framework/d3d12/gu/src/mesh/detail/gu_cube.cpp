
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
* @file      cube.cpp
* @brief     cube triangle mesh
*/

#include <mesh/gu_mesh.hpp>


namespace gu
{
    vertex_buffer_object make_cube()
    {
        vertex_buffer vb{ 24 };
        vertex v{};

        // front
        v.normal = tml::vec3{ 0, 0, 1 };
        v.tangent = tml::vec3{ 1, 0, 0 };
        v.position = tml::vec3{ -1.f, -1.f, 1.f }; v.uv = tml::vec2{ 0, 0 };
        vb[0] = v;
        v.position = tml::vec3{ 1.f, -1.f, 1.f }; v.uv = tml::vec2{ 1, 0 };
        vb[1] = v;
        v.position = tml::vec3{ 1.f, 1.f, 1.f }; v.uv = tml::vec2{ 1, 1 };
        vb[2] = v;
        v.position = tml::vec3{ -1.f, 1.f, 1.f }; v.uv = tml::vec2{ 0, 1 };
        vb[3] = v;

        // back
        v.normal = tml::vec3{ 0, 0, -1 };
        v.tangent = tml::vec3{ 1, 0, 0 };
        v.position = tml::vec3{ 1.f, -1.f, -1.f }; v.uv = tml::vec2{ 0, 0 };
        vb[4] = v;
        v.position = tml::vec3{ -1.f, -1.f, -1.f }; v.uv = tml::vec2{ 1, 0 };
        vb[5] = v;
        v.position = tml::vec3{ -1.f, 1.f, -1.f }; v.uv = tml::vec2{ 1, 1 };
        vb[6] = v;
        v.position = tml::vec3{ 1.f, 1.f, -1.f }; v.uv = tml::vec2{ 0, 1 };
        vb[7] = v;

        // top
        v.normal = tml::vec3{ 0, 1, 0 };
        v.tangent = tml::vec3{ 1, 0, 0 };
        v.position = tml::vec3{ -1.f, 1.f, 1.f }; v.uv = tml::vec2{ 0, 0 };
        vb[8] = v;
        v.position = tml::vec3{ 1.f, 1.f, 1.f }; v.uv = tml::vec2{ 1, 0 };
        vb[9] = v;
        v.position = tml::vec3{ 1.f, 1.f, -1.f }; v.uv = tml::vec2{ 1, 1 };
        vb[10] = v;
        v.position = tml::vec3{ -1.f, 1.f, -1.f }; v.uv = tml::vec2{ 0, 1 };
        vb[11] = v;

        // bottom
        v.normal = tml::vec3{ 0, -1, 0 };
        v.tangent = tml::vec3{ 1, 0, 0 };
        v.position = tml::vec3{ -1.f, -1.f, -1.f }; v.uv = tml::vec2{ 0, 0 };
        vb[12] = v;
        v.position = tml::vec3{ 1.f, -1.f, -1.f }; v.uv = tml::vec2{ 1, 0 };
        vb[13] = v;
        v.position = tml::vec3{ 1.f, -1.f, 1.f }; v.uv = tml::vec2{ 1, 1 };
        vb[14] = v;
        v.position = tml::vec3{ -1.f, -1.f, 1.f }; v.uv = tml::vec2{ 0, 1 };
        vb[15] = v;

        // right
        v.normal = tml::vec3{ 1, 0, 0 };
        v.tangent = tml::vec3{ 0, 0, -1 };
        v.position = tml::vec3{ 1.f, -1.f, 1.f }; v.uv = tml::vec2{ 0, 0 };
        vb[16] = v;
        v.position = tml::vec3{ 1.f, -1.f, -1.f }; v.uv = tml::vec2{ 1, 0 };
        vb[17] = v;
        v.position = tml::vec3{ 1.f, 1.f, -1.f }; v.uv = tml::vec2{ 1, 1 };
        vb[18] = v;
        v.position = tml::vec3{ 1.f, 1.f, 1.f }; v.uv = tml::vec2{ 0, 1 };
        vb[19] = v;

        // left
        v.normal = tml::vec3{ -1, 0, 0 };
        v.tangent = tml::vec3{ 0, 0, 1 };
        v.position = tml::vec3{ -1.f, -1.f, -1.f }; v.uv = tml::vec2{ 0, 0 };
        vb[20] = v;
        v.position = tml::vec3{ -1.f, -1.f, 1.f }; v.uv = tml::vec2{ 1, 0 };
        vb[21] = v;
        v.position = tml::vec3{ -1.f, 1.f, 1.f }; v.uv = tml::vec2{ 1, 1 };
        vb[22] = v;
        v.position = tml::vec3{ -1.f, 1.f, -1.f }; v.uv = tml::vec2{ 0, 1 };
        vb[23] = v;

        index_buffer ib = 
        {
            0, 1, 3, 3, 1, 2,
            4, 5, 7, 7, 5, 6,
            8, 9, 11, 11, 9, 10,
            12, 13, 15, 15, 13, 14,
            16, 17, 19, 19, 17, 18,
            20, 21, 23, 23, 21, 22
        };
       
        return
        {
            vb, ib
        };
    }


} // namespace

