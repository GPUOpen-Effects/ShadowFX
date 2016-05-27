
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
* @file      depth_stencil.hpp
* @brief     dx12 depth stencil helpers
*/

#ifndef DX12UTIL_DEPTH_STENCIL_HPP
#define DX12UTIL_DEPTH_STENCIL_HPP

#include <cmd_mgr.hpp>
#include <memory.hpp>
#include <utility>

namespace dx12u
{
    using depth_array_view = std::vector<D3D12_DEPTH_STENCIL_VIEW_DESC>; //!< depth stencil view of individual layers in a texture array
    /**
    * @brief make_depth_stencil result
    */
    struct depth_stencil_resource
    {
        resource db_rsrc; //!< depth stencil buffer resource
        depth_array_view dbv_desc; //!< depth stencil buffer view desc
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;//!< shader resource view desc
    };

    /**
    * @brief create a placed depth stencil resource
    * @param dvc device
    * @param width depth stencil buffer width
    * @param height depth stencil buffer height
    * @param format depth stencil buffer format
    * @param h heap where to place the resource
    * @param heap_offset offset in the heap where to place the resource
    * @return depth stencil resource
    */
    depth_stencil_resource make_depth_stencil(device const& dvc, std::size_t width, std::size_t height, DXGI_FORMAT format, heap const& h, std::size_t heap_offset);

    /**
    * @brief create a commited depth stencil resource
    * @param dvc device
    * @param width depth stencil buffer width
    * @param height depth stencil buffer height
    * @param format depth stencil buffer format
    * @return depth stencil resource
    */
    depth_stencil_resource make_depth_stencil(device const& dvc, std::size_t width, std::size_t height, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT);

    /**
    * @brief create a placed depth stencil resource array
    * @param dvc device
    * @param width depth stencil buffer width
    * @param height depth stencil buffer height
    * @param array_sz number of depth layer in the array
    * @param format depth stencil buffer format
    * @param h heap where to place the resource
    * @param heap_offset offset in the heap where to place the resource
    * @return depth stencil resource
    */
    depth_stencil_resource make_depth_stencil(device const& dvc, std::size_t width, std::size_t height, std::size_t array_sz, DXGI_FORMAT format, heap const& h, std::size_t heap_offset);

    /**
    * @brief create a commited depth stencil resource
    * @param dvc device
    * @param width depth stencil buffer width
    * @param height depth stencil buffer height
    * @param array_sz number of depth layer in the array
    * @param format depth stencil buffer format
    * @return depth stencil resource
    */
    depth_stencil_resource make_depth_stencil(device const& dvc, std::size_t width, std::size_t height, std::size_t array_sz, DXGI_FORMAT format = DXGI_FORMAT_D24_UNORM_S8_UINT);

} // namespace dx12u



#endif // DX12UTIL_DEPTH_STENCIL_HPP
