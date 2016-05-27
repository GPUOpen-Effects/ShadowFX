
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
* @file      dx12u.hpp
* @brief     dx12 utilities
*/

#ifndef DX12UTIL_HPP
#define DX12UTIL_HPP


#include <d3dx12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <vector>
#include <stdexcept>

namespace dx12u
{
    /**
    * @brief a dx12u exception
    */
    struct error : std::runtime_error
    {
        using base = std::runtime_error;
        using base::base;
    };

    /**
    * @brief a com ptr
    * @tparam T any type
    */
    template <typename T>
    using com_ptr = Microsoft::WRL::ComPtr<T>;

    /**
    * @brief a dx12 factory
    */
    using factory = com_ptr<IDXGIFactory4>;

    /**
    * @brief a dx12 capable adapter
    */
    using adapter = com_ptr<IDXGIAdapter1>;

    /**
    * @brief a dx12 warp adapter
    */
    using warp_adapter = com_ptr<IDXGIAdapter>;

    /**
    * @brief a list of adapters
    */
    using adapter_list = std::vector<adapter>;

    /**
    * @brief get a list of available hw adapters
    * @return list of available hw adapters
    * @note software adapters are exluded from the list
    */
    adapter_list get_hw_adapters();

    /**
    * @brief get the warp adapter
    * @return warp adapter
    */
    warp_adapter get_warp_adapter();

    /**
    * @brief throw if r represents an error
    * @param r a dx call result
    * @param extra_msg optional error message added if r represents an error
    */
    void throw_if_error(HRESULT r, std::string const& extra_msg = "");

    /**
    * @brief throw if a win32 error has occured
    * @param extra_msg optional error message added if a win32 error has occured
    */
    void throw_if_win32_error(std::string const& extra_msg = "");

    /**
    * @brief get dxgi factory
    * @return factory
    */
    factory get_factory();

    /**
    * @brief get the page size
    * @return page size
    */
    inline std::size_t get_page_size()
    {
        return 4096;
    }

    /**
    * @brief get constant buffer alignement required by dx12
    * @return constant buffer alignement 
    */
    inline std::size_t get_const_buffer_alignment()
    {
        return 256; // TODO use dx12 macro or query function
    }

} // namespace dx12u



#endif // DX12UTIL_HPP
