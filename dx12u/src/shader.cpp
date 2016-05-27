
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
* @brief     dx12 shader implementation
*/

#include <shader.hpp>
#include <D3Dcompiler.h>
#include <fstream>
#include <iterator>
#include <locale>
#include <codecvt>

namespace dx12u
{

    shader_blob compile(std::string const & code, std::string const & main_func, std::string const & target)
    {
        std::uint32_t compiler_flag = 0;
#ifdef MODE_DEBUG
        compiler_flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        com_ptr<ID3DBlob> blob{ nullptr };
        com_ptr<ID3DBlob> error{ nullptr };
        auto r = D3DCompile(code.c_str(), code.size(), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, main_func.c_str(), target.c_str(), compiler_flag, 0, &blob, &error);

        if (error)
        {
            throw dx12u::error{ reinterpret_cast<char const*>(error->GetBufferPointer()) };
        }

        dx12u::throw_if_error(r);

        return shader_blob{ blob };
    }

    shader_blob compile_from_file(std::string const& file, D3D_SHADER_MACRO const* macro, std::string const & main_func, std::string const & target)
    {
        std::uint32_t compiler_flag = 0;
#ifdef MODE_DEBUG
        compiler_flag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

        com_ptr<ID3DBlob> blob{ nullptr };
        com_ptr<ID3DBlob> error{ nullptr };
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wfile = converter.from_bytes(file);
        auto r = D3DCompileFromFile(wfile.c_str(), macro, D3D_COMPILE_STANDARD_FILE_INCLUDE, main_func.c_str(), target.c_str(), compiler_flag, 0, &blob, &error);

        if (error)
        {
            std::string msg = reinterpret_cast<char const*>(error->GetBufferPointer());
            throw dx12u::error{ msg };
        }

        dx12u::throw_if_error(r);

        return shader_blob{ blob };
    }
    shader_blob::shader_blob(std::size_t sz, std::uint8_t const * data)
    {
        auto r = D3DCreateBlob(sz, &blob);
        dx12u::throw_if_error(r);
        assert(blob->GetBufferSize() == sz);
        std::copy(data, data + sz, reinterpret_cast<std::uint8_t*>(blob->GetBufferPointer()));
    }
}
