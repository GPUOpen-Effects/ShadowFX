
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

#ifndef DX12UTIL_RENDER_TARGET_HPP
#define DX12UTIL_RENDER_TARGET_HPP

#include <cmd_mgr.hpp>
#include <memory.hpp>
#include <utility>
#include <tml/vec.hpp>

namespace dx12u
{
    /**
    * @brief make_render_target result
    */
    struct render_target_resource
    {
        resource rt_rsrc; //!< render target resource
        D3D12_RENDER_TARGET_VIEW_DESC rtv_desc; //!< render target view desc
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;// shader resource view desc
    };

    /**
    * @brief create a placed render target resource
    * @param dvc device
    * @param width render target width
    * @param height render target height
    * @param format render target format
    * @param h heap where to place the resource
    * @param heap_offset offset in the heap where to place the resource
    * @param gpu_ordinal gpu ordinal. a negative value can be used if there's only one gpu in the chain
    * @param visible_mask ordinal of all GPUs that can potentially use the resource
    * @param clear_color optimized clear color
    * @return render target resource
    */
    render_target_resource make_render_target(device const& dvc, std::size_t width, std::size_t height, DXGI_FORMAT format, 
        heap const& h, std::size_t heap_offset, int gpu_ordinal = -1, std::uint32_t visible_mask = 0, tml::vec4 const& clear_color = tml::vec4(0, 0, 0, 1));

    /**
    * @brief create a commited render target resource
    * @param dvc device
    * @param width render target width
    * @param height render target height
    * @param format render target format
    * @param gpu_ordinal gpu ordinal. a negative value can be used if there's only one gpu in the chain
    * @param visible_mask ordinal of all GPUs that can potentially use the resource
    * @param clear_color optimized clear color
    * @return render target resource
    */
    render_target_resource make_render_target(device const& dvc, std::size_t width, std::size_t height, 
        DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, int gpu_ordinal = -1, std::uint32_t visible_mask = 0, tml::vec4 const& clear_color = tml::vec4(0, 0, 0, 1));

} // namespace dx12u



#endif // DX12UTIL_RENDER_TARGET_HPP
