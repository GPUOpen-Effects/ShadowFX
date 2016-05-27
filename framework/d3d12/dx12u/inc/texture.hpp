
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
* @file      texture.hpp
* @brief     dx12 texture helpers
*/

#ifndef DX12UTIL_TEXTURE_HPP
#define DX12UTIL_TEXTURE_HPP

#include <cmd_mgr.hpp>
#include <memory.hpp>
#include <utility>
#include <gu/gu.hpp>

namespace dx12u
{

    /**
    * @brief make_tex result
    */
    struct tex_resources
    {
        resource texture_rsrc; //!< texture resource
        D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;// resource view desc
        resource upload_rsrc; //!< upload intermediate buffer
    };

    /**
    * @brief create a placed texture resource
    * @param dvc device
    * @param cl command list used for copying
    * @param tex_img texture data
    * @param h heap where to place the resource
    * @param heap_offset offset in the heap where to place the resource
    * @param fmt texture format
    * @param dimension texture dimension
    * @param texture_sys upload intermediate texture
    * @return a texture resource and an intermediate resource
    * @note the upload resource is an intermediate resource used by cl and can be freed after executing cl
    */
    tex_resources make_texture(device const& dvc, gfx_cmd_list& cl,
        gu::texture_view const& tex_img,
        dx12u::heap const& h, std::size_t heap_offset, 
        bool is_array = false, bool is_cube = false,
        DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D, dx12u::resource texture_sys = nullptr);

    /**
    * @brief create a committed texture resource
    * @param dvc device
    * @param cl command list used for copying
    * @param tex_img texture data
    * @param fmt texture format
    * @param dimension texture dimension
    * @param texture_sys upload intermediate texture
    * @return a texture resource and an intermediate resource
    * @note the upload resource is an intermediate resource used by cl and can be freed after executing cl
    */
    tex_resources make_texture(device const& dvc, gfx_cmd_list& cl,
        gu::texture_view const& tex_img,
        bool is_array = false, bool is_cube = false,
        DXGI_FORMAT fmt = DXGI_FORMAT_R8G8B8A8_UNORM, D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D, dx12u::resource texture_sys = nullptr);

} // namespace dx12u



#endif // DX12UTIL_TEXTURE_HPP
