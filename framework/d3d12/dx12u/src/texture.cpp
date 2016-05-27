
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
* @file      texture.cpp
* @brief     dx12 texture helpers
*/

#include <texture.hpp>
#include <array>
#include <algorithm>

using std::size_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

namespace 
{

    D3D12_SRV_DIMENSION dim2srv_dim(D3D12_RESOURCE_DIMENSION dimension, bool is_array, bool is_cube) noexcept
    {
        if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE1D)
        {
            return is_array ? D3D12_SRV_DIMENSION_TEXTURE1DARRAY : D3D12_SRV_DIMENSION_TEXTURE1D;
        }

        if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D &&  is_cube)
        {
            return is_array ? D3D12_SRV_DIMENSION_TEXTURECUBEARRAY : D3D12_SRV_DIMENSION_TEXTURECUBE;
        }

        if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE2D)
        {
            return is_array ? D3D12_SRV_DIMENSION_TEXTURE2DARRAY : D3D12_SRV_DIMENSION_TEXTURE2D;
        }

        if (dimension == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
        {
            return D3D12_SRV_DIMENSION_TEXTURE3D;
        }

        return D3D12_SRV_DIMENSION_UNKNOWN;
    }

    dx12u::tex_resources make_texture_common(dx12u::device const& dvc, dx12u::gfx_cmd_list& cl, gu::texture_view const& tex_img, dx12u::heap const& h, std::size_t heap_offset,
            bool is_array, bool is_cube,
            DXGI_FORMAT fmt, D3D12_RESOURCE_DIMENSION dimension, dx12u::resource texture_sys)
    {
        if (tex_img.num_lvl < 1 || tex_img.mip == nullptr || tex_img.bpp < 1)
        {
            throw dx12u::error{ "empty texture image" };
        }

        D3D12_RESOURCE_DESC tex_desc = {};
        tex_desc.MipLevels = static_cast<uint16_t>(tex_img.num_lvl);
        tex_desc.Format = fmt;
        tex_desc.Width = tex_img.mip[0].width;
        tex_desc.Height = static_cast<uint32_t>(tex_img.mip[0].height);
        tex_desc.Flags = D3D12_RESOURCE_FLAG_NONE; // TODO
        tex_desc.DepthOrArraySize = static_cast<uint16_t>(tex_img.mip[0].depth);
        tex_desc.SampleDesc.Count = 1;
        tex_desc.SampleDesc.Quality = 0;
        tex_desc.Dimension = dimension;

        dx12u::resource rsrc{ nullptr };
        if (h.Get() == nullptr)
        {
            auto heap_default = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT); // TODO GPU node
            auto r = dvc->CreateCommittedResource(&heap_default, D3D12_HEAP_FLAG_NONE,
                &tex_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&rsrc));
            dx12u::throw_if_error(r);
        }
        else
        {
            auto r = dvc->CreatePlacedResource(h.Get(), heap_offset,
                &tex_desc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&rsrc));
            dx12u::throw_if_error(r);
        }

        if (texture_sys.Get() == nullptr)
        {
            uint64_t texture_sys_size = GetRequiredIntermediateSize(rsrc.Get(), 0, tex_desc.MipLevels);
            auto heap_upload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD); // TODO GPU node
            auto texture_sys_buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(texture_sys_size);
            auto r = dvc->CreateCommittedResource(&heap_upload, D3D12_HEAP_FLAG_NONE,
                &texture_sys_buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&texture_sys));
            dx12u::throw_if_error(r);
        }

        auto sub_rsrc_data = std::vector<D3D12_SUBRESOURCE_DATA>(tex_img.num_lvl);
        for (size_t i = 0; i < tex_img.num_lvl; ++i)
        {
            sub_rsrc_data[i].pData = tex_img.mip[i].data;
            sub_rsrc_data[i].RowPitch = tex_img.mip[i].pitch;
            sub_rsrc_data[i].SlicePitch = tex_img.mip[i].slice_sz;
        }

        UpdateSubresources(cl.Get(), rsrc.Get(), texture_sys.Get(), 0, 0, tex_desc.MipLevels, sub_rsrc_data.data());

        auto transition = CD3DX12_RESOURCE_BARRIER::Transition(rsrc.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
        cl->ResourceBarrier(1, &transition);

        D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
        srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv.Format = fmt;
        srv.ViewDimension = dim2srv_dim(dimension, is_array, is_cube);
        srv.Texture2D.MipLevels = static_cast<uint32_t>(tex_img.num_lvl);

        return
        {
            rsrc,
            srv,
            texture_sys
        };
    }

} // namespace

namespace dx12u
{

    tex_resources make_texture(device const& dvc, gfx_cmd_list& cl, gu::texture_view const& tex_img, dx12u::heap const& h, std::size_t heap_offset, 
        bool is_array, bool is_cube, DXGI_FORMAT fmt, D3D12_RESOURCE_DIMENSION dimension, dx12u::resource texture_sys)
    {
        if (h.Get() == nullptr)
        {
            throw error{ "failed to create resource. null heap" };
        }
        return make_texture_common(dvc, cl, tex_img, h, heap_offset, is_array, is_cube, fmt, dimension, texture_sys);
    }

    tex_resources make_texture(device const& dvc, gfx_cmd_list& cl, gu::texture_view const& tex_img, 
        bool is_array, bool is_cube, DXGI_FORMAT fmt, D3D12_RESOURCE_DIMENSION dimension, dx12u::resource texture_sys)
    {
        return make_texture_common(dvc, cl, tex_img, nullptr, 0, is_array, is_cube, fmt, dimension, texture_sys);
    }

} // namespace dx12u
