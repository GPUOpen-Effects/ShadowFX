
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
* @file      device.hpp
* @brief     dx12 device utilities
*/

#ifndef DX12UTIL_DEVICE_HPP
#define DX12UTIL_DEVICE_HPP

#include <dx12u.hpp>

namespace dx12u
{

    /**
    * @brief a dx12 device
    */
    using device = com_ptr<ID3D12Device>;

    /**
    * @brief create a device
    * @param a adapter. if null the first valid hw adapter will be used
    * @return device
    */
    device create_device(adapter a = nullptr);

    /**
    * @brief create a warp device
    * @return device
    */
    device create_warp_device();

} // namespace dx12u


#endif // DX12UTIL_DEVICE_HPP
