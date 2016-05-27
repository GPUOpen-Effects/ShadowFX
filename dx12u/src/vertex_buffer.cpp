
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
* @file      vertex_buffer.cpp
* @brief     dx12 vertex buffer helpers
*/

#include <vertex_buffer.hpp>
#include <array>
#include <algorithm>

using std::size_t;
using std::uint8_t;
using std::uint16_t;
using std::uint32_t;
using std::uint64_t;

namespace
{

    dx12u::vb_resources make_vb_common(dx12u::device const& dvc, dx12u::gfx_cmd_list& cl,
        gu::vertex_buffer_object const& vbo, dx12u::heap const& h, std::size_t heap_offset)
    {
        auto vbr = dx12u::detail::make_buffer(dvc, cl, vbo.vb.data(), vbo.vb.size() * sizeof(gu::vertex), h, heap_offset, 
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
        auto ibr = dx12u::detail::make_buffer(dvc, cl, vbo.ib.data(), vbo.ib.size() * sizeof(uint32_t), h, heap_offset, 
            D3D12_RESOURCE_STATE_INDEX_BUFFER);

        D3D12_VERTEX_BUFFER_VIEW vb_view{};
        vb_view.BufferLocation = vbr.first->GetGPUVirtualAddress();
        vb_view.StrideInBytes = sizeof(gu::vertex);
        vb_view.SizeInBytes = static_cast<uint32_t>(vbo.vb.size() * sizeof(gu::vertex));

        D3D12_INDEX_BUFFER_VIEW  ib_view{};
        ib_view.BufferLocation = ibr.first->GetGPUVirtualAddress();
        ib_view.Format = DXGI_FORMAT_R32_UINT;
        ib_view.SizeInBytes = static_cast<uint32_t>(vbo.ib.size() * sizeof(uint32_t));

        return
        {
            vbr.first,
            ibr.first,
            vb_view,
            ib_view,
            vbr.second,
            ibr.second,
        };
    }

} // namespace

namespace dx12u
{

    vb_resources make_vb(device const& dvc, gfx_cmd_list& cl, gu::vertex_buffer_object const& vbo, heap const& h, std::size_t heap_offset)
    {
        if (h.Get() == nullptr)
        {
            throw error{ "failed to create resource. null heap" };
        }
        return make_vb_common(dvc, cl, vbo, h, heap_offset);
    }

    vb_resources make_vb(device const& dvc, gfx_cmd_list& cl, gu::vertex_buffer_object const& vbo)
    {
        return make_vb_common(dvc, cl, vbo, nullptr, 0);
    }

} // namespace dx12u
