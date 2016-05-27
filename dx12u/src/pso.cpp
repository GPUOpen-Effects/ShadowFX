
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
* @file      pso.hpp
* @brief     dx12 pso implementation
*/

#include <pso.hpp>

using std::uint32_t;

namespace dx12u
{

    pipeline_state_object::pipeline_state_object(device const& d, input_layout_list const& ill,
        root_signature const& rs, shader_blob const& vs, shader_blob const& hs, shader_blob const& ds, shader_blob const& gs, shader_blob const& ps)
        : ill_copy{ ill }, dev {
        d
    }, rs_copy{ rs }, vs_copy{ vs }, hs_copy{ hs }, ds_copy{ ds }, gs_copy{ gs }, ps_copy{ ps }
    {
        if (d.Get() == nullptr)
        {
            throw error{ "pso creation failed. null device" };
        }

        pso_desc.InputLayout = { nullptr, 0 };
        if (!ill_copy.empty())
        {
            pso_desc.InputLayout = { ill_copy.data(), static_cast<uint32_t>(ill_copy.size()) };
        }
        pso_desc.pRootSignature = rs_copy.Get();
        pso_desc.VS = vs_copy;
        pso_desc.HS = hs_copy;
        pso_desc.DS = ds_copy;
        pso_desc.GS = gs_copy;
        pso_desc.PS = ps_copy;
        pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        pso_desc.RasterizerState.FrontCounterClockwise = true;
        pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        pso_desc.SampleMask = 0xffffffffu;
        pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        pso_desc.NumRenderTargets = 1;
        pso_desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
        pso_desc.SampleDesc.Count = 1;
    }

    pipeline_state_object::pipeline_state_object(pipeline_state_object&& other) noexcept
        : dev{ other.dev } , pso { other.pso }, pso_desc(other.pso_desc), rs_copy{ other.rs_copy },
        vs_copy{ other.vs_copy }, hs_copy{ other.hs_copy }, ds_copy{ other.ds_copy }, gs_copy{ other.gs_copy }, ps_copy{ other.ps_copy },
        ill_copy{ std::move(ill_copy) }
    {
        other.rs_copy = nullptr;
        other.dev = nullptr;
        other.pso = nullptr;
        other.vs_copy = other.hs_copy = other.ds_copy = other.gs_copy = other.ps_copy = shader_blob{};
        ill_copy = {};
    }

    pipeline_state_object& pipeline_state_object::operator = (pipeline_state_object&& other) noexcept
    {
        if (this != &other)
        {
            dev = other.dev;
            pso = other.pso;
            rs_copy = other.rs_copy;
            pso_desc = other.pso_desc;
            vs_copy = other.vs_copy;
            hs_copy = other.hs_copy;
            ds_copy = other.ds_copy;
            gs_copy = other.gs_copy;
            ps_copy = other.ps_copy;
            ill_copy = std::move(other.ill_copy);
            other.dev = nullptr;
            other.pso = nullptr;
            other.rs_copy = nullptr;
            other.vs_copy = shader_blob{};
            other.hs_copy = shader_blob{};
            other.ds_copy = shader_blob{};
            other.gs_copy = shader_blob{};
            other.ps_copy = shader_blob{};
            other.ill_copy = {};
        }

        return *this;
    }

    void pipeline_state_object::set_gpu_node_mask(std::uint32_t mask)
    {
        if (pso_desc.NodeMask != mask)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.NodeMask = mask;
    }

    void pipeline_state_object::set_depth_stencil_states(D3D12_DEPTH_STENCIL_DESC const & dss)
    {
        if (memcmp(&dss, &pso_desc.DepthStencilState, sizeof(dss)) == 0)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.DepthStencilState = dss;
    }

    void pipeline_state_object::set_blend_states(D3D12_BLEND_DESC const & bs)
    {
        if (memcmp(&bs, &pso_desc.BlendState, sizeof(bs)) == 0)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.BlendState = bs;
    }

    void pipeline_state_object::set_cull_mode(D3D12_CULL_MODE mode)
    {
        if (pso_desc.RasterizerState.CullMode != mode)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.RasterizerState.CullMode = mode;
    }

    void pipeline_state_object::set_ccw(bool ccw)
    {
        if (pso_desc.RasterizerState.FrontCounterClockwise != static_cast<BOOL>(ccw))
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.RasterizerState.FrontCounterClockwise = ccw;
    }

    void pipeline_state_object::set_fill_mode(D3D12_FILL_MODE mode)
    {
        if (pso_desc.RasterizerState.FillMode != mode)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.RasterizerState.FillMode = mode;
    }

    void pipeline_state_object::set_depth_test(bool state)
    {
        if (!!pso_desc.DepthStencilState.DepthEnable != state)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.DepthStencilState.DepthEnable = state;
    }

    void pipeline_state_object::set_depth_write(bool state)
    {
        auto s = state ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
        if (pso_desc.DepthStencilState.DepthWriteMask != s)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.DepthStencilState.DepthWriteMask = s;
    }

    void pipeline_state_object::set_depth_func(D3D12_COMPARISON_FUNC f)
    {
        if (pso_desc.DepthStencilState.DepthFunc != f)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.DepthStencilState.DepthFunc = f;
    }

    void pipeline_state_object::set_depth_format(DXGI_FORMAT fmt)
    {
        if (pso_desc.DSVFormat != fmt)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.DSVFormat = fmt;
    }

    pipeline_state_object::pso_com_ptr const& pipeline_state_object::get_com_ptr()
    {
        std::unique_lock<std::mutex> lk{ mux };

        if (pso.Get() != nullptr)
        {
            return pso;
        }

        if (!vs_copy.is_valid())
        {
            throw error{ "pso creation failed. At least a root signature and a vs have to be valid" };
        }

        pso = nullptr; // make sure the previous pso is deleted
        auto r = dev->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pso));
        assert(pso.Get() != nullptr);
        dx12u::throw_if_error(r, "pso creation failed. ");

        return pso;
    }

    void pipeline_state_object::set_rt_format(DXGI_FORMAT format, std::size_t rt_index)
    {
        if (rt_index > 7)
        {
            throw error{ "invalid render target index" };
        }
        if (pso_desc.RTVFormats[rt_index] != format)
        {
            std::unique_lock<std::mutex> lk{ mux };
            pso = nullptr;
        }
        pso_desc.RTVFormats[rt_index] = format;
    }

    void pipeline_state_object::release_blob()
    {
        vs_copy.release();
        hs_copy.release();
        ds_copy.release();
        gs_copy.release();
        ps_copy.release();
    }

    void bind(ID3D12GraphicsCommandList* cl, pipeline_state_object& pso)
    {
        auto pso_ptr = pso.get_com_ptr().Get();
        auto rs_ptr = pso.get_root_signature().Get();
        bind(cl, rs_ptr, pso_ptr);
    }

    void bind(ID3D12GraphicsCommandList* cl, ID3D12RootSignature* rs, ID3D12PipelineState* pso)
    {
        assert(pso != nullptr && rs != nullptr && cl != nullptr);
        cl->SetPipelineState(pso);
        cl->SetGraphicsRootSignature(rs);
    }

    input_layout_list get_default_input_layout()
    {
        return
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "INSTANCEPOS", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
        };
    }

} // namespace dx12u

