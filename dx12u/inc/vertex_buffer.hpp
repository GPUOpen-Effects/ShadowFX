
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
* @file      vertex_buffer.hpp
* @brief     dx12 vertex buffer helpers
*/

#ifndef DX12UTIL_VERTEX_BUFFER_HPP
#define DX12UTIL_VERTEX_BUFFER_HPP

#include <cmd_mgr.hpp>
#include <memory.hpp>
#include <utility>
#include <gu/gu.hpp>
#include "detail/buffer.ipp"

namespace dx12u
{
    /**
    * @brief vertex and index buffer pair
    */
    struct vbo
    {
        dx12u::resource vb; //!< vertex buffer resource
        dx12u::resource ib; //!< index buffer resource
        D3D12_VERTEX_BUFFER_VIEW vbv;
        D3D12_INDEX_BUFFER_VIEW ibv;
    };

    /**
    * @brief make_vb result
    */
    struct vb_resources
    {
        resource vb; //!< vertex buffer resource
        resource ib; //!< index buffer resource
        D3D12_VERTEX_BUFFER_VIEW vb_view; //!< vertex buffer view
        D3D12_INDEX_BUFFER_VIEW  ib_view; //!< index buffer view
        resource vb_sys; //!< intermediate upload system memory for vertex buffer resource
        resource ib_sys; //!< intermediate upload system memory for vertex buffer resource
    };

    /**
    * @brief make_instance_data result
    */
    struct instance_resource
    {
        resource rsrc; //!< instance buffer resource
        D3D12_VERTEX_BUFFER_VIEW inst_view; //!< instance buffer view
        resource rsrc_sys; //!< intermediate upload system memory for instance buffer resource
    };

    /**
    * @brief create a placed vertex buffer resource
    * @param dvc device
    * @param cl command list used for copying
    * @param vbo vbo data
    * @param h heap where to place the resource
    * @param heap_offset offset in the heap where to place the resource
    * @return vertex buffer resources and the upload system memory intermediate resources
    */
    vb_resources make_vb(device const& dvc, gfx_cmd_list& cl, gu::vertex_buffer_object const& vbo, heap const& h, std::size_t heap_offset);

    /**
    * @brief create a committed vertex buffer resource
    * @param dvc device
    * @param cl command list used for copying
    * @param vbo vbo data
    * @return vertex buffer resources and the upload system memory intermediate resources
    */
    vb_resources make_vb(device const& dvc, gfx_cmd_list& cl, gu::vertex_buffer_object const& vbo);

    /**
    * @brief create a placed instance buffer resource
    * @param dvc device
    * @param cl command list used for copying
    * @param data data to store in the buffer
    * @param data_sz size of the data to store in the buffer
    * @param stride element stride
    * @param h heap where to place the resource
    * @param heap_offset offset in the heap where to place the resource
    * @return vertex buffer resources and the upload system memory intermediate resources
    */
    inline instance_resource make_instance_data(device const& dvc, gfx_cmd_list& cl, void const* data, std::size_t data_sz, std::size_t stride, heap const& h, std::size_t heap_offset)
    {
        auto rsrc = detail::make_buffer(dvc, cl, data, data_sz, h, heap_offset,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        D3D12_VERTEX_BUFFER_VIEW vb_view{};
        vb_view.BufferLocation = rsrc.first->GetGPUVirtualAddress();
        vb_view.StrideInBytes = static_cast<std::uint32_t>(stride);
        vb_view.SizeInBytes = static_cast<std::uint32_t>(data_sz);

        return
        {
            rsrc.first,
            vb_view,
            rsrc.second
        };
    }

    /**
    * @brief create a committed instance  buffer resource
    * @param dvc device
    * @param cl command list used for copying
    * @param data data to store in the buffer
    * @param data_sz size of the data to store in the buffer
    * @param stride element stride
    * @return vertex buffer resources and the upload system memory intermediate resources
    */
    inline instance_resource make_instance_data(device const& dvc, gfx_cmd_list& cl, void const* data, std::size_t data_sz, std::size_t stride)
    {
        auto rsrc = detail::make_buffer(dvc, cl, data, data_sz, nullptr, 0,
            D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

        D3D12_VERTEX_BUFFER_VIEW vb_view{};
        vb_view.BufferLocation = rsrc.first->GetGPUVirtualAddress();
        vb_view.StrideInBytes = static_cast<std::uint32_t>(stride);
        vb_view.SizeInBytes = static_cast<std::uint32_t>(data_sz);

        return
        {
            rsrc.first,
            vb_view,
            rsrc.second
        };
    }

} // namespace dx12u



#endif // DX12UTIL_VERTEX_BUFFER_HPP
