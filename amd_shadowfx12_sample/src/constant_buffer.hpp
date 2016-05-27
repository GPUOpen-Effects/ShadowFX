
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
* @file      constant_buffer.hpp
* @brief     definition of the constant buffers used in shadow_fx12 sample
*/

#ifndef SHADOWFX_SAMPLE_CONST_BUFFER_HPP
#define SHADOWFX_SAMPLE_CONST_BUFFER_HPP

#include <tml/mat.hpp>

// In the sample one point light is used and renders to 5 cube faces
// The 6th face: +Y is not used as the light is placed on the ceiling of a conoference room mesh
// Changing the mesh and the light position would require chaning this macro
#define NUM_CUBE_FACE 5


// simple plastic lighting ps constant buffer
struct plastic_ps_cb
{
    tml::vec4  ambient_color;
    tml::vec4  diffuse_color;
    tml::vec4  specular_color; // shininess in alpha
    tml::vec4  light_color[NUM_CUBE_FACE];
    tml::vec4  light_pos[NUM_CUBE_FACE]; // in view space
    uint32_t   num_light;

    void set(tml::vec3 const& a, tml::vec3 const& d, tml::vec4 const& s, tml::vec4 const& light_col)
    {
        ambient_color = tml::vec4(a, 1.f);
        diffuse_color = tml::vec4(d, 1.f);
        specular_color = s;
        num_light = NUM_CUBE_FACE;
        for (auto& l : light_color)
        {
            l = light_col;
        }
    }
};

// simple plastic lighting vs constant buffer
struct plastic_vs_cb
{
    tml::mat4 mvp; // model view projection
    tml::mat4 mv; // model view (assumed orthogonal)
};

// simple depth only passes vs constant buffer
struct depth_pass_cb
{
    tml::mat4 mvp; // model view projection
};


#endif // SHADOWFX_SAMPLE_CONST_BUFFER_HPP

