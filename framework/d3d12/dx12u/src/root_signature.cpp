
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
* @file      root_signature.cpp
* @brief     dx12 resource descriptor table implementation
*/

#include <root_signature.hpp>
#include <algorithm>
#include <iterator>
#include <cassert>

using std::uint32_t;
using std::int32_t;
using std::size_t;

namespace //anonymous
{
    using range_list = std::vector<CD3DX12_DESCRIPTOR_RANGE>;
    using range_list_list = std::vector<range_list>;

    uint32_t pop_count(uint32_t x) noexcept
    {
        // todo use pop_count instruction if available
        unsigned int c = 0;
        for (c = 0; x; c++)
        {
            x &= x - 1;
        }
        return c;
    }

    uint32_t mask_stage(dx12u::shader_mask stage, dx12u::shader_mask sm) noexcept
    {
        auto m = static_cast<uint32_t>(stage) & static_cast<uint32_t>(sm);
        return static_cast<uint32_t>(-static_cast<int32_t>(m != 0));
    }

    D3D12_SHADER_VISIBILITY get_visibility(dx12u::shader_mask sm)
    {
        if (sm == dx12u::shader_mask::none)
        {
            throw dx12u::error{ "0 shader visibility" };
        }
        if (pop_count(static_cast<uint32_t>(sm)) > 1)
        {
            return D3D12_SHADER_VISIBILITY_ALL;
        }
        return static_cast<D3D12_SHADER_VISIBILITY>( 0
            | ((mask_stage(dx12u::shader_mask::vs, sm) & D3D12_SHADER_VISIBILITY_VERTEX))
            | ((mask_stage(dx12u::shader_mask::hs, sm) & D3D12_SHADER_VISIBILITY_HULL))
            | ((mask_stage(dx12u::shader_mask::ds, sm) & D3D12_SHADER_VISIBILITY_DOMAIN))
            | ((mask_stage(dx12u::shader_mask::gs, sm) & D3D12_SHADER_VISIBILITY_GEOMETRY))
            | ((mask_stage(dx12u::shader_mask::ps, sm) & D3D12_SHADER_VISIBILITY_PIXEL))
            );
    }

    D3D12_ROOT_SIGNATURE_FLAGS make_signature_flag(dx12u::shader_mask sm) noexcept
    {
        return static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
            | (~mask_stage(dx12u::shader_mask::vs, sm) & D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS)
            | (~mask_stage(dx12u::shader_mask::hs, sm) & D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS)
            | (~mask_stage(dx12u::shader_mask::ds, sm) & D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS)
            | (~mask_stage(dx12u::shader_mask::gs, sm) & D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS)
            | (~mask_stage(dx12u::shader_mask::ps, sm) & D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS)
            );
    }

    constexpr D3D12_DESCRIPTOR_RANGE_TYPE cvt_range_type(dx12u::descriptor_type d) noexcept
    {
        // TODO: remove enum order dependedency
        return static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(d);
    }

    std::vector<CD3DX12_DESCRIPTOR_RANGE> make_descriptor_sig_table(std::vector<dx12u::descriptor_sig> const& table)
    {
        std::vector<CD3DX12_DESCRIPTOR_RANGE> out_table{};
        for (auto const& x : table)
        {
            auto t = x.type;
            auto reg_idx = x.register_idx;
            auto reg_space = x.register_space;

            if (t != dx12u::descriptor_type::srv && t != dx12u::descriptor_type::uav 
                && t != dx12u::descriptor_type::cbv && t != dx12u::descriptor_type::sampler)
            {
                throw dx12u::error{ "wrong type in descriptor table" };
            }

            out_table.emplace_back(cvt_range_type(t), 1, reg_idx, reg_space);
        }
        return out_table;
    }

} // namespace anonymous

namespace dx12u
{

    void descriptor_sig_list::append(descriptor_sig const & ds)
    {
        dsl.push_back({ ds });
        global_visibility = global_visibility | ds.visibility;
    }

    void descriptor_sig_list::append(descriptor_sig_list const & dst)
    {
        dsl.emplace_back();
        dsl.back().dst.resize(dst.dsl.size());
        std::transform(std::begin(dst.dsl), std::end(dst.dsl), std::begin(dsl.back().dst), [](any const& a)
        {
            if (!a.dst.empty())
            {
                throw error{"descriptor table indirections are not allowed"};
            }
            return a.ds;
        });

        dsl.back().ds.visibility = dst.global_visibility;

        global_visibility = global_visibility | dst.global_visibility;
    }

    root_signature make_root_signature(ID3D12Device* dev, descriptor_sig_list dsl, static_sampler_list const& sl, std::uint32_t node_mask)
    {
        range_list_list rll{}; // used to keep range_list objects in memory until the function returns

        if (dev == nullptr)
        {
            throw error{ "root signature creation failed. null device" };
        }

        // convert dsl to D3D12_ROOT_PARAMETER array
        std::vector<CD3DX12_ROOT_PARAMETER> root_param{ dsl.dsl.size() };
        for (size_t i = 0; i < root_param.size(); ++i)
        {
            auto v = get_visibility(dsl.dsl[i].ds.visibility);

            if (dsl.dsl[i].dst.empty()) // root argument rather than a descriptor table
            {
                auto reg_idx = dsl.dsl[i].ds.register_idx;
                auto reg_space = dsl.dsl[i].ds.register_space;
                auto cv = dsl.dsl[i].ds.constant_value;
                switch (dsl.dsl[i].ds.type)
                {
                case descriptor_type::srv:
                    root_param[i].InitAsShaderResourceView(reg_idx, reg_space, v); break;
                case descriptor_type::cbv:
                    root_param[i].InitAsConstantBufferView(reg_idx, reg_space, v); break;
                case descriptor_type::uav:
                    root_param[i].InitAsUnorderedAccessView(reg_idx, reg_space, v); break;
                case descriptor_type::constant:
                    root_param[i].InitAsConstants(cv, reg_idx, reg_space, v); break;
                case descriptor_type::sampler:
                    throw error{ "root signature creation failed. non static root samplers are not allowed" };
                default:
                    throw error{ "root signature creation failed. unknown root descriptor type" };
                }
            }
            else // descriptor table
            {
                assert(dsl.dsl[i].ds.type == descriptor_type::invalid);
                auto table = make_descriptor_sig_table(dsl.dsl[i].dst);
                rll.emplace_back(std::move(table));
                root_param[i].InitAsDescriptorTable(static_cast<uint32_t>(rll.back().size()), rll.back().data(), v);
            }
        }

        // make the root signature
        CD3DX12_ROOT_SIGNATURE_DESC rs_desc{ 
            static_cast<uint32_t>(root_param.size()), root_param.empty() ? nullptr : root_param.data(),
            static_cast<uint32_t>(sl.size()), sl.empty() ? nullptr : sl.data(),
            make_signature_flag(dsl.global_visibility)
        };

        com_ptr<ID3DBlob> blob{ nullptr };
        com_ptr<ID3DBlob> error{ nullptr };
        auto r = D3D12SerializeRootSignature(&rs_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
        dx12u::throw_if_error(r, "root signature serialization failed. ");

        root_signature rs{nullptr};
        r = dev->CreateRootSignature(node_mask, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rs));
        dx12u::throw_if_error(r, "root signature creation failed. ");

        return rs;
    }

    root_signature make_empty_root_signature(ID3D12Device* dev, std::uint32_t node_mask)
    {
        return make_root_signature(dev, {}, {}, node_mask);
    }

    D3D12_STATIC_SAMPLER_DESC make_default_static_sampler(std::uint32_t reg, D3D12_FILTER filter, D3D12_SHADER_VISIBILITY visibility)
    {
        D3D12_STATIC_SAMPLER_DESC sampler = {};
        sampler.Filter = filter;
        sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        sampler.MipLODBias = 0;
        sampler.MaxAnisotropy = 0;
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler.MinLOD = 0.0f;
        sampler.MaxLOD = D3D12_FLOAT32_MAX;
        sampler.ShaderRegister = reg;
        sampler.RegisterSpace = 0;
        sampler.ShaderVisibility = visibility;
        return sampler;
    }

    D3D12_STATIC_SAMPLER_DESC make_default_cmp_static_sampler(std::uint32_t reg, D3D12_FILTER filter)
    {
        auto sampler = make_default_static_sampler(reg, filter);
        sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
        return sampler;
    }

} // namespace dx12u


