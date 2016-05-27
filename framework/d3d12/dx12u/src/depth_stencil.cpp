
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

#include <depth_stencil.hpp>

using std::uint16_t;
using std::uint32_t;
using std::size_t;

namespace
{
    DXGI_FORMAT get_srv_format(DXGI_FORMAT depth_format) noexcept
    {
        switch (depth_format)
        {
        case DXGI_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
        case DXGI_FORMAT_D16_UNORM: return DXGI_FORMAT_R16_UNORM;
        case DXGI_FORMAT_D32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
        default: break;
        }
        return DXGI_FORMAT_UNKNOWN;
    }

    DXGI_FORMAT get_tex_format(DXGI_FORMAT depth_format) noexcept
    {
        switch (depth_format)
        {
        case DXGI_FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_R24G8_TYPELESS;
        case DXGI_FORMAT_D16_UNORM: return DXGI_FORMAT_R16_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT: return DXGI_FORMAT_R32_TYPELESS;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_R32G32_TYPELESS;
        default: break;
        }
        return DXGI_FORMAT_UNKNOWN;
    }
    
    dx12u::depth_stencil_resource make_depth_stencil_common(
        dx12u::device const& dvc, size_t width, size_t height, size_t array_sz, DXGI_FORMAT format, dx12u::heap const& h, size_t heap_offset)
    {

        if (!(width && height && array_sz))
        {
            throw dx12u::error{ "empty depth buffer" };
        }

        auto srv_format = get_srv_format(format);
        auto tex_format = get_tex_format(format);
        if (srv_format == DXGI_FORMAT_UNKNOWN || tex_format == DXGI_FORMAT_UNKNOWN)
        {
            throw dx12u::error{ "invalid depth format" };
        }

        D3D12_CLEAR_VALUE depth_clear_value = {};
        depth_clear_value.Format = format;
        depth_clear_value.DepthStencil.Depth = 1.0f;
        depth_clear_value.DepthStencil.Stencil = 0;

        dx12u::resource rsrc{ nullptr };
        if (h.Get() == nullptr)
        {
            auto default_heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
            auto tex_desc = CD3DX12_RESOURCE_DESC::Tex2D(tex_format, width, static_cast<uint32_t>(height), static_cast<uint16_t>(array_sz), 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            auto r = dvc->CreateCommittedResource(&default_heap, D3D12_HEAP_FLAG_NONE,
                &tex_desc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE, &depth_clear_value, IID_PPV_ARGS(&rsrc));
            dx12u::throw_if_error(r);
        }
        else
        {
            auto tex_desc = CD3DX12_RESOURCE_DESC::Tex2D(tex_format, width, static_cast<uint32_t>(height), static_cast<uint16_t>(array_sz), 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
            auto r = dvc->CreatePlacedResource(h.Get(), heap_offset,
                &tex_desc,
                D3D12_RESOURCE_STATE_DEPTH_WRITE, &depth_clear_value, IID_PPV_ARGS(&rsrc));
            dx12u::throw_if_error(r);
        }


        auto dbv_desc = dx12u::depth_array_view(array_sz);
        for (size_t i = 0; i < dbv_desc.size(); ++i)
        {
            dbv_desc[i].Format = format;
            dbv_desc[i].ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
            dbv_desc[i].Flags = D3D12_DSV_FLAG_NONE;
            dbv_desc[i].Texture2DArray.MipSlice = 0;
            dbv_desc[i].Texture2DArray.FirstArraySlice = static_cast<uint32_t>(i);
            dbv_desc[i].Texture2DArray.ArraySize = 1;
        }

        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc.Format = srv_format;
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        srv_desc.Texture2DArray.FirstArraySlice = 0;
        srv_desc.Texture2DArray.MostDetailedMip = 0;
        srv_desc.Texture2DArray.PlaneSlice = 0;
        srv_desc.Texture2DArray.ResourceMinLODClamp = 0;
        srv_desc.Texture2DArray.MipLevels = 1;
        srv_desc.Texture2DArray.ArraySize = static_cast<uint32_t>(array_sz);

        return 
        {
            rsrc,
            dbv_desc,
            srv_desc
        };
    }

} // namespace

namespace dx12u
{

    depth_stencil_resource make_depth_stencil(device const& dvc, size_t width, size_t height, DXGI_FORMAT format, heap const& h, size_t heap_offset)
    {
        if (h.Get() == nullptr)
        {
            throw error{ "failed to create resource. null heap" };
        }
        return make_depth_stencil_common(dvc, width, height, 1, format, h, heap_offset);
    }

    depth_stencil_resource make_depth_stencil(device const& dvc, size_t width, size_t height, DXGI_FORMAT format)
    {
        return make_depth_stencil_common(dvc, width, height, 1, format, nullptr, 0);
    }

    depth_stencil_resource make_depth_stencil(device const & dvc, size_t width, size_t height, size_t array_sz, DXGI_FORMAT format, heap const & h, size_t heap_offset)
    {
        if (h.Get() == nullptr)
        {
            throw error{ "failed to create resource. null heap" };
        }
        return make_depth_stencil_common(dvc, width, height, array_sz, format, h, heap_offset);
    }

    depth_stencil_resource make_depth_stencil(device const & dvc, size_t width, size_t height, size_t array_sz, DXGI_FORMAT format)
    {
        return make_depth_stencil_common(dvc, width, height, array_sz, format, nullptr, 0);
    }

} // namespace dx12u
