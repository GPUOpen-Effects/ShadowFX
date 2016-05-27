
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
* @file      root_signature.hpp
* @brief     dx12 resource descriptor table utilities
*/

#ifndef DX12UTIL_ROOT_SIGNATURE_HPP
#define DX12UTIL_ROOT_SIGNATURE_HPP


#include <device.hpp>
#include <cstdint>
#include <vector>

namespace dx12u
{
    /**
    * @brief root signature type
    */
    using root_signature = com_ptr<ID3D12RootSignature>;

    /**
    * @brief sampler list
    */
    using static_sampler_list = std::vector<D3D12_STATIC_SAMPLER_DESC>;

    /**
    * @brief descriptor type
    */
    enum class descriptor_type : std::uint32_t
    {
        srv,
        uav,
        cbv,
        sampler,
        constant,
        invalid
    };

    /**
    * @brief shader mask
    */
    enum class shader_mask : std::uint32_t
    {
        vs = 1,
        hs = 2,
        ds = 4,
        gs = 8,
        ps = 16,
        all = vs | hs | ds | gs | ps,
        none = 0
    };

    /**
    * @brief shader mask or
    * @param m1 fisrt mask
    * @param m2 second mask
    * @return combined mask m1 | m2
    */
    inline constexpr shader_mask operator | (shader_mask m1, shader_mask m2) noexcept
    {
        return static_cast<shader_mask>(static_cast<std::uint32_t>(m1) | static_cast<std::uint32_t>(m2));
    }

    /**
    * @brief descriptor signature
    */
    struct descriptor_sig
    {
        descriptor_type type = descriptor_type::invalid;
        std::uint32_t register_idx = 0;
        std::uint32_t register_space = 0;
        shader_mask visibility = shader_mask::all;
        std::uint32_t constant_value = 0;
    public:
        /**
        * @brief descriptor signature creation
        * @param t descriptor type
        * @param reg_idx shader register
        * @param reg_space shader space
        * @param v shader visibility mask
        */
        explicit descriptor_sig(descriptor_type t = descriptor_type::invalid, std::uint32_t reg_idx = 0, std::uint32_t reg_space = 0, shader_mask v = shader_mask::all) noexcept
            : type(t), register_idx(reg_idx), register_space(reg_space), visibility(v)
        {
        }

        /**
        * @brief default copy
        */
        descriptor_sig(descriptor_sig const&) noexcept = default;

        /**
        * @brief default assign
        */
        descriptor_sig& operator = (descriptor_sig const&) noexcept = default;

        /**
        * @brief set a constant
        * @param value constant value
        */
        void set_constant(std::uint32_t value) noexcept
        {
            constant_value = value;
        }
    };

    /**
    * @brief descriptor signature list
    */
    class descriptor_sig_list
    {
        using descriptor_sig_table = std::vector<descriptor_sig>;
        struct any
        {
            descriptor_sig ds;
            descriptor_sig_table dst;
        };

        std::vector<any> dsl;

        shader_mask global_visibility = shader_mask::none;

    public:
        /**
        * @brief default ctor
        */
        descriptor_sig_list() = default;

        /**
        * @brief default copy
        */
        descriptor_sig_list(descriptor_sig_list const&) = default;

        /**
        * @brief default copy assign
        */
        descriptor_sig_list& operator = (descriptor_sig_list const&) = default;

        /**
        * @brief default move
        */
        descriptor_sig_list(descriptor_sig_list&&) noexcept = default;

        /**
        * @brief default move assign
        */
        descriptor_sig_list& operator = (descriptor_sig_list&&) noexcept = default;

        /**
        * @brief append a descriptor signature
        */
        void append(descriptor_sig const& ds);

        /**
        * @brief append a descriptor signature table
        */
        void append(descriptor_sig_list const& dst);

        /**
        * @brief create a root signature from a descriptor signature list
        * @param dev device
        * @param dsl descriptor signature list
        * @param sl list of static samplers
        * @param node_mask bits set to 1 for GPU nodes that use the pso
        * @return root signature
        */
        friend root_signature make_root_signature(ID3D12Device* dev, descriptor_sig_list dsl, static_sampler_list const& sl = {}, std::uint32_t node_mask = 0);

        /**
        * @brief create an empty root signature
        * @param dev device
        * @param node_mask bits set to 1 for GPU nodes that use the pso
        * @return root signature
        */
        friend root_signature make_empty_root_signature(ID3D12Device* dev, std::uint32_t node_mask = 0);
    };

    /**
    * @brief initializes a default static sampler desc
    * @param reg sampler register
    * @param filter filtering method
    * @param visibility shader visibility
    * @return static sampler desc
    */
    D3D12_STATIC_SAMPLER_DESC make_default_static_sampler(
        std::uint32_t reg = 0, D3D12_FILTER filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_PIXEL);

    /**
    * @brief initializes a default comparison static sampler desc
    * @param reg sampler register
    * @param filter filtering method
    * @param visibility shader visibility
    * @return static sampler desc
    */
    D3D12_STATIC_SAMPLER_DESC make_default_cmp_static_sampler(std::uint32_t reg = 0, D3D12_FILTER filter = D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT);

} // namespace dx12u



#endif // DX12UTIL_ROOT_SIGNATURE_HPP
