
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
* @file      mesh.hpp
* @brief     mesh structures
*/

#ifndef GU_MESH_HPP
#define GU_MESH_HPP

#include <tml/mat.hpp>
#include <algorithm>
#include <vector>
#include <cstdint>
#include <string>
#include <limits>

namespace gu
{

    /**
    * @brief a vertex structure
    */
    struct vertex
    {
        tml::vec3 position{ 0,0,0 };
        tml::vec3 normal{ 0,0,0 };
        tml::vec3 tangent{ 0,0,0 };
        tml::vec2 uv{ 0,0 };
    };

    /**
    * @brief axis aligned bounding box
    */
    struct aabb
    {
        tml::vec3 a = tml::vec3(-std::numeric_limits<float>::max());
        tml::vec3 b = tml::vec3(std::numeric_limits<float>::max());
    };

    /**
    * @brief vertex buffer
    */
    using vertex_buffer = std::vector<vertex>;

    /**
    * @brief index buffer
    */
    using index_buffer = std::vector<std::uint32_t>;

    /**
    * @brief vertex and index buffer vbo
    */
    struct vertex_buffer_object
    {
        vertex_buffer vb;
        index_buffer  ib;
    };


    /**
    * @brief make a unit cube vbo
    * @return unit cube
    */
    vertex_buffer_object make_cube();

    /**
    * @brief make a unit sphere vbo
    * @param num_circle number of lattidude sections
    * @param num_pt number of longitude sections
    * @return unit sphere
    * @note error if num_circle < 2 || num_pt < 2
    */
    vertex_buffer_object make_sphere(std::size_t num_circle, std::size_t num_pt);

    /**
    * @brief make a torus vbo
    * @param major_radius major radius
    * @param minor_radius minor radius
    * @param num_circle number of tube sections
    * @param num_pt number of vertices per tube section
    * @return torus
    */
    vertex_buffer_object make_torus(float major_radius, float minor_radius, std::size_t num_circle, std::size_t num_pt);

    /**
    * @brief transform a vbo
    * @param vbo vbo to transform
    * @param t transformation matrix
    */
    void transform(vertex_buffer_object& vbo, tml::mat4 const& t);

    /**
    * @brief obtain the aabb of a mesh
    * @param vbo vertext buffer object
    * @return aabb of vbo
    */
    aabb get_aabb(vertex_buffer_object const& vbo);

    /**
    * @brief union of aabb
    * @param a fisrt aabb
    * @param b second aabb
    * @return union of aabb
    */
    inline aabb union_aabb(aabb const& a, aabb const& b)
    {
        aabb u{};
        for (int i = 0; i < 3; ++i)
        {
            u.a[i] = std::min(a.a[i], b.a[i]);
            u.b[i] = std::max(a.b[i], b.b[i]);
        }
        return u;
    }

} // namespace 

#endif // GU_MESH_HPP
