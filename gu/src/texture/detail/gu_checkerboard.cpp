
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
* @file      checkerboard.cpp
* @brief     checkerboard 
*/


#include <texture/gu_texture_generator.hpp>

namespace gu
{
    common_texture make_checkerboard(std::size_t block_width, std::size_t block_height, std::size_t num_x_block, std::size_t num_y_block)
    {
        using std::size_t;

        auto width = block_width * num_x_block;
        auto height = block_height * num_y_block;

        common_texture tex{};
        tex.albedo.data.resize(1);
        tex.albedo.data[0].resize(width * width);

        tex.albedo.mip_info.resize(1);
        tex.albedo.mip_info[0].width = width;
        tex.albedo.mip_info[0].height = height;
        tex.albedo.mip_info[0].pitch = width * sizeof(std::uint32_t);
        tex.albedo.mip_info[0].slice_sz = tex.albedo.mip_info[0].pitch * height;
        tex.albedo.mip_info[0].data = tex.albedo.data[0].data();

        tex.albedo.view.num_lvl = 1;
        tex.albedo.view.bpp = 32;
        tex.albedo.view.mip = tex.albedo.mip_info.data();

        if (tex.albedo.data[0].empty())
        {
            return tex;
        }

        for (size_t y = 0, i = 0; y < height; ++y)
        {
            auto by = y / block_height;
            for (size_t x = 0; x < width; ++x, ++i)
            {
                auto bx = x / block_width;

                tex.albedo.data[0][i] = static_cast<std::uint32_t>((-static_cast<std::int32_t>((bx & 1) ^ (by & 1))));
            }
        }

        return tex;
    }

} // namespace

