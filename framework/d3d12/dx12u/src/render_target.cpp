
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
* @file      depth_stencil.cpp
* @brief     dx12 depth stencil helpers
*/

#include <render_target.hpp>

namespace
{

    dx12u::render_target_resource make_render_target_common(dx12u::device const& dvc, 
        std::size_t width, std::size_t height, DXGI_FORMAT format, dx12u::heap const& h, std::size_t heap_offset, int gpu_ordinal, std::uint32_t visible_mask, tml::vec4 const& clear_color)
    {

        if (!width || !height)
        {
            throw dx12u::error{ "empty render target" };
        }

        // TODO move to param and check what exactly makes the driver use fast clear
        D3D12_CLEAR_VALUE clear_value = {};
        clear_value.Format = format;
        std::copy(&clear_color.x, &clear_color.x + 4, std::begin(clear_value.Color));

        std::uint32_t gpu_mask = gpu_ordinal < 0 ? 1 : (1u << static_cast<std::uint32_t>(gpu_ordinal));
        visible_mask |= gpu_mask;

        dx12u::resource rsrc{ nullptr };
        if (h.Get() == nullptr)
        {
            auto heap_default = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT, gpu_mask, visible_mask);
            auto tex_2d_desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, static_cast<uint32_t>(height), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
            auto r = dvc->CreateCommittedResource(&heap_default, D3D12_HEAP_FLAG_NONE,
                &tex_2d_desc,
                D3D12_RESOURCE_STATE_RENDER_TARGET, &clear_value, IID_PPV_ARGS(&rsrc));
            dx12u::throw_if_error(r);
        }
        else
        {
            auto tex_2d_desc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, static_cast<uint32_t>(height), 1, 1, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET);
            auto r = dvc->CreatePlacedResource(h.Get(), heap_offset,
                &tex_2d_desc,
                D3D12_RESOURCE_STATE_RENDER_TARGET, &clear_value, IID_PPV_ARGS(&rsrc));
            dx12u::throw_if_error(r);
        }

        D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
        rtv_desc.Format = format;
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
        rtv_desc.Texture2D.MipSlice = 0;
        rtv_desc.Texture2D.PlaneSlice = 0;

        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc.Format = format;
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srv_desc.Texture2D.MipLevels = 1;

        return 
        {
            rsrc,
            rtv_desc,
            srv_desc
        };
    }

} // namespace

namespace dx12u
{

    render_target_resource make_render_target(device const& dvc, std::size_t width, std::size_t height, DXGI_FORMAT format, 
        heap const& h, std::size_t heap_offset, int gpu_ordinal, std::uint32_t visible_mask, tml::vec4 const& clear_color)
    {
        if (h.Get() == nullptr)
        {
            throw error{ "failed to create resource. null heap" };
        }
        return make_render_target_common(dvc, width, height, format, h, heap_offset, gpu_ordinal, visible_mask, clear_color);
    }

    render_target_resource make_render_target(device const& dvc, std::size_t width, std::size_t height, DXGI_FORMAT format, 
        int gpu_ordinal, std::uint32_t visible_mask, tml::vec4 const& clear_color)
    {
        return make_render_target_common(dvc, width, height, format, nullptr, 0, gpu_ordinal, visible_mask, clear_color);
    }

} // namespace dx12u
