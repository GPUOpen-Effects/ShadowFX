
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
* @file      dx12u.cpp
* @brief     dx12u implementation
*/

#include <dx12u.hpp>
#include <string>
#include <sstream>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>

using std::uint32_t;

namespace // anonymous
{
    template< typename T >
    std::string to_hex(T i)
    {
        std::stringstream stream{};
        stream << "0x" << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << i;
        return stream.str();
    }

    dx12u::factory create_factory()
    {
        uint32_t flag = 0;
#if  defined(MODE_DEBUG) || defined(_DEBUG)
        flag = DXGI_CREATE_FACTORY_DEBUG;
#endif

        dx12u::com_ptr<IDXGIFactory4> fctory{ nullptr };
        auto r = CreateDXGIFactory2(flag, IID_PPV_ARGS(&fctory));
        dx12u::throw_if_error(r, "IDXGIFactory4 creation failed. ");
        return fctory;
    }

    struct debug_layer
    {
        debug_layer(bool force = false)
        {
#if  defined(MODE_DEBUG) || defined(_DEBUG)
            force = true;
#endif
            if (!force)
            {
                return;
            }
            dx12u::com_ptr<ID3D12Debug> debugController{ nullptr };
            if (D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)) == S_OK)
            {
                debugController->EnableDebugLayer();
            }
        }
    };

} // namespace anonymous

namespace dx12u
{
    adapter_list get_hw_adapters()
    {
        auto fctory = get_factory();
        assert(fctory.Get() != nullptr);

        adapter adptr{ nullptr };
        adapter_list alist;

        for (uint32_t i = 0; fctory->EnumAdapters1(i, &adptr) != DXGI_ERROR_NOT_FOUND; ++i)
        {
            DXGI_ADAPTER_DESC1 desc;
            adptr->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // skip software adapters
                continue;
            }

            if (FAILED(D3D12CreateDevice(adptr.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr))) // the device is nullptr so it won't be created
            {
                // skip non dx12 adapters
                continue;
            }

            alist.push_back(adptr);
        }

        return alist;
    }

    warp_adapter get_warp_adapter()
    {
        auto fctory = get_factory();
        assert(fctory.Get() != nullptr);

        warp_adapter a{ nullptr };
        auto r = fctory->EnumWarpAdapter(IID_PPV_ARGS(&a));
        throw_if_error(r, "failed to get the warp adapter. ");

        return a;
    }

    void throw_if_error(HRESULT r, std::string const& extra_msg)
    {
        if (FAILED(r))
        {
            auto err_msg = extra_msg + "error code : " + to_hex(static_cast<uint32_t>(r));
            throw dx12u::error{ err_msg };
        }
    }

    void throw_if_win32_error(std::string const& extra_msg)
    {
        char* raw_msg{ nullptr };
        auto err = GetLastError();
        auto ret = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
            NULL, err, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), reinterpret_cast<char*>(&raw_msg), 0, NULL);

        if (!ret && !raw_msg)
        {
            return;
        }

        auto msg = extra_msg + raw_msg;
        std::replace(std::begin(msg), std::end(msg), '\n', ' ');
        std::replace(std::begin(msg), std::end(msg), '\r', ' ');
        msg += "win32 error code: " + std::to_string(err);

        {
            auto r = LocalFree(raw_msg);
            assert(r == 0);
            if (r != 0)
            {
                throw error{ "failed to release win32 error handler message\n" };
            }
        }

        throw error{msg};
    }

    factory get_factory()
    {
        static debug_layer dbg_layer{}; // the factory is the first object created
        static dx12u::com_ptr<IDXGIFactory4> fctory = create_factory();
        return fctory;
    }

} // namespace dx12u

