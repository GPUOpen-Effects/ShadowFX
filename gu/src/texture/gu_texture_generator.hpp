
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
* @file      texture_generator.hpp
* @brief     texture generator
*/

#ifndef GU_TEXTURE_GENERATOR_HPP
#define GU_TEXTURE_GENERATOR_HPP

#include <../src/texture/gu_texture.hpp>
#include <tml/vec.hpp>
// #include <glm/fwd.hpp>
#include <vector>
#include <cstdint>
#include <cstddef>

namespace gu
{

    /**
    * @brief texture container of rgba8 data
    */
    struct rbga8_texture_data
    {
        // container types
        using texel_data = std::vector<std::uint32_t>; //!< data container
        using mip_data = std::vector<texel_data>; //!< mip data container
        using mip_info_data = std::vector<mip_lvl_texture_view>;  //!< mip levels container
                                                                 
        // data
        texture_view view; //!< view
        mip_data      data; //!< data
        mip_info_data mip_info; //!< mip info
    };

    /**
    * @brief common material
    */
    struct common_texture
    {
        rbga8_texture_data albedo;  //!< albedo data
        rbga8_texture_data normal_and_bump; //!< normal data and bump in alpha channel (empty vector if not used)
        rbga8_texture_data ambient; //!< ambient data
    };

    /**
    * @brief make a checkerboard texture
    * @param block_width block width in pixels
    * @param block_height block height in pixels
    * @param num_x_block number of blocks in the x direction
    * @param num_y_block number of blocks in the y direction
    * @return checkerboard texture data
    */
    common_texture make_checkerboard(std::size_t block_width, std::size_t block_height, std::size_t num_x_block, std::size_t num_y_block);

    /**
    * @brief make a brick texture
    * @param brick_width brick width in pixels
    * @param brick_height brick height in pixels
    * @param num_x_brick number of bricks in the x direction
    * @param num_y_brick number of bricks in the y direction
    * @param mortar_tickness value in [0-1] defining the thickness of the mortar
    * @return checkerboard texture data
    */
    common_texture make_bricks(std::size_t brick_width, std::size_t brick_height,
        std::size_t num_x_brick, std::size_t num_y_brick, 
        float mortar_tickness = .1f,
        tml::u8vec3 const& brick_col = tml::u8vec3{ 127, 38, 36 }, tml::u8vec3 const& mortar_col = tml::u8vec3{ 125, 125, 125 });

    /**
    * @brief make a test texture with one color per mip level
    * @param width width in pixels
    * @param height height in pixels
    * @return test texture data
    */
    common_texture make_test_texture(std::size_t width, std::size_t height);

} // namespace 

#endif // GU_MESH_HPP
