
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
* @file      device.cpp
* @brief     dx12 device implementation
*/

#include <device.hpp>

namespace
{
    template<typename A>
    dx12u::device make_device(A const& a)
    {
        dx12u::device dv{ nullptr };
        auto r = D3D12CreateDevice(a.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&dv));
        if (FAILED(r))
        {
            r = D3D12CreateDevice(a.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&dv));
        }
        dx12u::throw_if_error(r, "device creation failed. ");
        return dv;
    }
}

namespace dx12u
{

    device create_device(adapter a)
    {
        if (a.Get() == nullptr)
        {
            auto alist = get_hw_adapters();
            if (alist.empty())
            {
                throw std::runtime_error{ "failed to find a dx12 capable hw adapter for device creation" };
            }
            a = alist.front();
        }
        return make_device(a);
    }

    device create_warp_device()
    {
        auto a = get_warp_adapter();
        return make_device(a);
    }

} // namespace dx12u

