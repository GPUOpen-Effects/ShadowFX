
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
* @file      brick.cpp
* @brief     brick texture
*/


#include <texture/gu_texture_generator.hpp>
#include <algorithm>
// #include <glm/geometric.hpp>
// #include <glm/common.hpp>
#include <array>
#include <cassert>

using std::size_t;
using std::uint32_t;

namespace
{

    void normal_from_bump(gu::rbga8_texture_data::texel_data& tex, size_t width, size_t height)
    {
        assert(width * height == tex.size());

        if (width < 2 || height < 2) // TODO 1D texture case instead of oring
        {
            for (auto& t : tex)
            {
                t |= 255u << 16; // (0,0,1,1)
            }
        }

        float u = 1.f / static_cast<float>(width - 1);
        float v = 1.f / static_cast<float>(height - 1);

        for (size_t i = 0; i < tex.size(); ++i)
        {
            float b = (tex[i] >> 24u) / 255.f;
            float bu = (tex[(i + 1) % tex.size()] >> 24u) / 255.f - b;
            float bv = (tex[(i + width) % tex.size()] >> 24u) / 255.f - b;
            tml::vec3 du = tml::vec3{ u, 0, bu };
            tml::vec3 dv = tml::vec3{ 0, v, bv };
            auto n = tml::normalize(tml::cross(du, dv));
            auto nx = static_cast<uint32_t>(n[0] * 255.f);
            auto ny = static_cast<uint32_t>(n[1] * 255.f);
            auto nz = static_cast<uint32_t>(n[2] * 255.f);
            tex[i] |= nx | (ny << 8u) | (nz << 16u);
        }
    }

    
    tml::u32vec4 get_texel(gu::rbga8_texture_data::texel_data const& tex, size_t x, size_t y, size_t w, size_t h)
    {
        (void*)h; // unused
        auto t = tex[x + y * w];
        return tml::u32vec4(t & 0xffu, (t >> 8u) & 0xffu, (t >> 16u) & 0xffu, (t >> 24u) & 0xffu);
    }

    gu::rbga8_texture_data::texel_data downsample(gu::rbga8_texture_data::texel_data const& up, size_t uw, size_t uh, size_t w, size_t h)
    {
        auto down = gu::rbga8_texture_data::texel_data(w * h);

        for (size_t i = 0, y = 0; y < h; ++y)
        {
            auto uy = y * 2;
            for (size_t x = 0; x < w; ++x, ++i)
            {
                auto ux = x * 2;
                auto c = get_texel(up, ux, uy, uw, uh) + get_texel(up, ux+1, uy, uw, uh) 
                    + get_texel(up, ux, uy + 1, uw, uh) + get_texel(up, ux + 1, uy + 1, uw, uh);
                c = c / 4u;
                down[i] = c.x | (c.y << 8u) | (c.z << 16u) | (c.w << 24u);
            }
        }

        return down;
    }

    void downsample(gu::rbga8_texture_data::texel_data const& up, gu::mip_lvl_texture_view const& up_info, gu::rbga8_texture_data::texel_data& down, gu::mip_lvl_texture_view& down_info)
    {
        assert(up_info.width > 1 && up_info.height > 1);

        down_info.width = up_info.width / 2;
        down_info.height = up_info.height / 2;
        down_info.depth = 1;
        down_info.pitch = down_info.width * sizeof(uint32_t);
        down_info.slice_sz = down_info.pitch * down_info.height;
        if (!up.empty())
        {
            down = downsample(up, up_info.width, up_info.height, down_info.width, down_info.height);
        }
        down_info.data = down.data();
    }

    gu::rbga8_texture_data gen_mipmaps(gu::rbga8_texture_data::texel_data lv0, size_t width, size_t height)
    {
        auto num_mip = static_cast<size_t>(std::log2(static_cast<float>(std::min(width, height)))) + 1;
        if (!num_mip)
        {
            return{};
        }

        gu::rbga8_texture_data tex{};
        tex.data.resize(num_mip);
        tex.data[0] = std::move(lv0);
        tex.mip_info.resize(num_mip);
        tex.mip_info[0].width = width;
        tex.mip_info[0].height = height;
        tex.mip_info[0].pitch = width * sizeof(uint32_t);
        tex.mip_info[0].slice_sz = tex.mip_info[0].pitch * height;
        tex.mip_info[0].data = tex.data[0].data();

        for (size_t i = 1; i < num_mip; ++i)
        {
            downsample(tex.data[i - 1], tex.mip_info[i - 1], tex.data[i], tex.mip_info[i]);
        }

        tex.view.bpp = 32;
        tex.view.num_lvl = num_mip;
        tex.view.mip = tex.mip_info.data();

        return tex;
    }

    uint32_t ambient_from_bump(uint32_t bump, uint32_t col)
    {
        float a = bump * .15f / 255.f;
        auto r = static_cast<uint32_t>((col & 255) * a);
        auto g = static_cast<uint32_t>(((col >> 8) & 255) * a);
        auto b = static_cast<uint32_t>(((col >> 16) & 255) * a);
        return r | (g << 8u) | (b << 16u) | 0xff000000;
    }

} // namespace

namespace gu
{
    common_texture make_bricks(std::size_t brick_width, std::size_t brick_height,
        std::size_t num_x_brick, std::size_t num_y_brick, float mortar_tickness,
        tml::u8vec3 const& brick_col, tml::u8vec3 const& mortar_col)
    {
        auto width = brick_width * num_x_brick;
        auto height = brick_height * num_y_brick;

        rbga8_texture_data::texel_data albedo_lv0(width * height);
        rbga8_texture_data::texel_data normal_and_bump_lv0(width * height);
        rbga8_texture_data::texel_data ambient_lv0(width * height);

        if (albedo_lv0.empty())
        {
            return {};
        }

        // at least 2 pixels of mortars otherwise ismoothstep will fail
        auto const mortar = std::max<size_t>(2, static_cast<size_t>( static_cast<float>(std::min(brick_width, brick_height)) * mortar_tickness * .5f ));
        auto const bcol = static_cast<uint32_t>(brick_col.x) | (static_cast<uint32_t>(brick_col.y) << 8u) | (static_cast<uint32_t>(brick_col.z) << 16u) | (255u << 24u);
        auto const mcol = static_cast<uint32_t>(mortar_col.x) | (static_cast<uint32_t>(mortar_col.y) << 8u) | (static_cast<uint32_t>(mortar_col.z) << 16u) | (255u << 24u);

        if (brick_width <= mortar || brick_height <= mortar)
        {
            return {}; // otherwise ismoothstep will fail
        }

        for (size_t y = 0, i = 0; y < height; ++y)
        {
            auto by = y / brick_height;

            auto v = y % brick_height;
            bool is_mortar_y = v < mortar || v >= brick_height - mortar;
            float vbump = tml::smoothstep(0.f, static_cast<float>(mortar - 1), static_cast<float>(v))
                - tml::smoothstep(static_cast<float>(brick_height - mortar), static_cast<float>(brick_height - 1), static_cast<float>(v));

            for (size_t x = 0; x < width; ++x, ++i)
            {
                auto shift_x = (by & 1) * brick_width / 2 + x;

                auto u = shift_x % brick_width;
                bool is_mortar = is_mortar_y || u < mortar || u >= brick_width - mortar;
                float ubump = tml::smoothstep(0.f, static_cast<float>(mortar - 1), static_cast<float>(u))
                    - tml::smoothstep(static_cast<float>(brick_width - mortar), static_cast<float>(brick_width - 1), static_cast<float>(u));

                auto bump = static_cast<uint32_t>(tml::clamp(ubump * vbump * 255.f + 5.f, 0.f, 255.f));

                albedo_lv0[i] = is_mortar ? mcol : bcol;
                normal_and_bump_lv0[i] = bump << 24u;
                ambient_lv0[i] = ambient_from_bump(bump, albedo_lv0[i]);
            }
        }

        normal_from_bump(normal_and_bump_lv0, width, height);

        common_texture out;
        out.albedo = gen_mipmaps(std::move(albedo_lv0), width, height);
        out.normal_and_bump = gen_mipmaps(std::move(normal_and_bump_lv0), width, height);
        out.ambient = gen_mipmaps(std::move(ambient_lv0), width, height);

        return out;
    }

    common_texture make_test_texture(std::size_t width, std::size_t height)
    {
        std::array<uint32_t, 4> col_map =
        {
            0xff0000ff,
            0xff00ff00,
            0xffff0000,
            0xff0000ff
        };

        common_texture tex{};
        tex.albedo.view.bpp = 32;
        for (std::size_t i = 0; width > 1 && height > 1; ++i)
        {
            rbga8_texture_data::texel_data data(width * height, col_map[i & 3]);
            mip_lvl_texture_view info{};
            info.width = width;
            info.height = height;
            info.pitch = width * sizeof(std::uint32_t);
            info.slice_sz = info.pitch * height;
            info.data = data.data();
            width /= 2;
            height /= 2;

            tex.albedo.data.push_back(std::move(data));
            tex.albedo.mip_info.push_back(info);
            ++tex.albedo.view.num_lvl;
        }

        tex.albedo.view.mip = tex.albedo.mip_info.data();

        return tex;
    }

} // namespace

