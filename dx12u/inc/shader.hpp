
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
* @file      shader.hpp
* @brief     dx12 shader utilities
*/

#ifndef DX12UTIL_SHADER_HPP
#define DX12UTIL_SHADER_HPP


#include <dx12u.hpp>
#include <string>
#include <iostream>
#include <cassert>
#include <cstdint>

namespace dx12u
{
    /**
    * @brief a shader blob wrapper
    */
    class shader_blob
    {
        using blob_com_ptr = com_ptr<ID3DBlob>;
        blob_com_ptr blob{ nullptr };
    public:
        /**
        * @brief default constructor
        */
        shader_blob() = default;

        /**
        * @brief default copy constructor
        */
        shader_blob(shader_blob const&) = default;

        /**
        * @brief default copy-assign
        */
        shader_blob& operator = (shader_blob const&) = default;

        /**
        * @brief construct from a blob
        * @param b blob
        */
        explicit shader_blob(blob_com_ptr const& b) : blob(b)
        {}

        /**
        * @brief construct from a blob
        * @param sz blob size in bytes
        * @param data blob data
        */
        explicit shader_blob(std::size_t sz, std::uint8_t const* data);

        /**
        * @brief access the blob data
        * @return blob data
        * @note a null D3D12_SHADER_BYTECODE is returned if the blob is invalid
        */
        D3D12_SHADER_BYTECODE get() const
        {
            if (is_valid())
            {
                return{ reinterpret_cast<std::uint8_t*>(blob->GetBufferPointer()), blob->GetBufferSize() };
            }
            // else
            return { nullptr, 0 };
        }

        /**
        * @brief access the blob data
        * @return blob data
        * @note a null D3D12_SHADER_BYTECODE is returned if the blob is invalid
        */
        operator D3D12_SHADER_BYTECODE() const
        {
            return get();
        }

        /**
        * @brief access the underlying blob com ptr
        * @return blob com ptr
        * @note the ptr is null if the blob is invalid
        */
        blob_com_ptr const& get_com_ptr() const noexcept
        {
            return blob;
        }

        /**
        * @brief check whether the blob is valid of not
        * @return true if the blob is valid; false otherwise
        */
        bool is_valid() const noexcept
        {
            return blob.Get() != nullptr;
        }

        /**
        * @brief explicitly release the blob
        */
        void release()
        {
            blob = nullptr;
        }
    };

    /**
    * @brief compile a shader
    * @param code hlsl code
    * @param main_func shader entry point
    * @param target shader target
    * @return shader bytecode
    */
    shader_blob compile(std::string const& code, std::string const& main_func, std::string const & target);

    /**
    * @brief compile a shader from file
    * @param file hlsl file
    * @param macro list of macro definitions. set to nullptr if not used
    * @param main_func shader entry point
    * @param target shader target
    * @return shader bytecode
    */
    shader_blob compile_from_file(std::string const& file, D3D_SHADER_MACRO const* macro, std::string const& main_func, std::string const& target);

} // namespace dx12u



#endif // DX12UTIL_SHADER_HPP
