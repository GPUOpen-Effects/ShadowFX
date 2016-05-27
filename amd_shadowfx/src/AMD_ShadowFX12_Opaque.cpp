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

#include <string>
#include <fstream>
#include <cassert>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef AMD_SHADOWFX_COMPILE_STATIC_LIB
#   define AMD_DLL_EXPORTS
#endif

#include "AMD_ShadowFX12_Opaque.h"
#include "AMD_ShadowFX_Precompiled.h"

#pragma warning( disable : 4100 ) // disable unreference formal parameter warnings for /W4 builds

namespace
{
    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    // different permutations of shadow_fx parameters use different shaders
    // compiling PSOs for all permutations would be very slow
    // this structure keeps the data needed to create the PSOs and create them in a lazy manner
    // returns the PSO or nullptr in case of error
    ID3D12PipelineState* get(ID3D12Device* dev, AMD::shadowfx_pipeline_state_object& pso)
    {
        if (pso.pso.Get() == nullptr)
        {
            assert(dev != nullptr);
            HRESULT r = dev->CreateGraphicsPipelineState(&pso.pso_desc, IID_PPV_ARGS(&pso.pso));
            if (FAILED(r))
            {
                return nullptr;
            }
        }
        return pso.pso.Get();
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    // function creating a shader resource descriptor heap. SHADOWFX_RETURN_CODE_D3D12_CALL_FAILED is returned if CreateDescriptorHeap fails
    AMD::SHADOWFX_RETURN_CODE make_srd_heap(ID3D12Device* dev, std::size_t n, AMD::shadowfx_descriptor_heap& heap)
    {
        assert(dev != nullptr && n > 0 && heap.heap.Get() == nullptr);

        std::uint32_t const gpu_mask = 1; // GPU 1
        D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {};
        heap_desc.NumDescriptors = static_cast<uint32_t>(n);
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        heap_desc.NodeMask = gpu_mask;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

        HRESULT r = dev->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&heap.heap));
        if (FAILED(r))
        {
            return AMD::SHADOWFX_RETURN_CODE_D3D12_CALL_FAILED;
        }

        heap.num_slot = n;
        heap.slot_size = dev->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        return AMD::SHADOWFX_RETURN_CODE_SUCCESS;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    // get cpu descriptor handle at slot i in heap
    D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_handle(AMD::shadowfx_descriptor_heap const& heap, std::size_t i)
    {
        return { heap.heap->GetCPUDescriptorHandleForHeapStart().ptr + i * heap.slot_size };
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    // get gpu descriptor handle at slot i in heap
    D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(AMD::shadowfx_descriptor_heap const& heap, std::size_t i)
    {
        return{ heap.heap->GetGPUDescriptorHandleForHeapStart().ptr + static_cast<std::uint64_t>(i * heap.slot_size) };
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    // init root signature rs. SHADOWFX_RETURN_CODE_D3D12_CALL_FAILED is returned if a the root creation function fails
    AMD::SHADOWFX_RETURN_CODE make_root_signature(ID3D12Device* dev, Microsoft::WRL::ComPtr<ID3D12RootSignature>& rs)
    {
        std::uint32_t const gpu_mask = 1; // GPU 1

        // descriptor table
        CD3DX12_DESCRIPTOR_RANGE descriptor_table[AMD::ShadowFX_OpaqueDesc::m_num_srd_heap_slot] =
        {
            CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0),
            CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0),
            CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1),
            CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2),
        };

        // root table
        CD3DX12_ROOT_PARAMETER root_param[1];
        root_param[0].InitAsDescriptorTable(static_cast<uint32_t>(AMD::ShadowFX_OpaqueDesc::m_num_srd_heap_slot), descriptor_table, D3D12_SHADER_VISIBILITY_PIXEL);

        // static samplers
        static std::uint32_t const num_static_sampler = 4;
        D3D12_STATIC_SAMPLER_DESC sampler[num_static_sampler] = {};

        sampler[0].Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
        sampler[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        sampler[0].MipLODBias = 0;
        sampler[0].MaxAnisotropy = 0;
        sampler[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
        sampler[0].BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
        sampler[0].MinLOD = 0.0f;
        sampler[0].MaxLOD = D3D12_FLOAT32_MAX;
        sampler[0].ShaderRegister = 0;
        sampler[0].RegisterSpace = 0;
        sampler[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

        sampler[1] = sampler[0];
        sampler[1].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler[1].ShaderRegister = 1;

        sampler[2] = sampler[1];
        sampler[2].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS;
        sampler[2].Filter = D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
        sampler[2].ShaderRegister = 2;

        sampler[3] = sampler[2];
        sampler[3].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        sampler[3].ShaderRegister = 3;

        // flags
        D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
            | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

        // root desc
        CD3DX12_ROOT_SIGNATURE_DESC rs_desc
        {
            1, root_param,
            num_static_sampler, sampler,
            flag
        };

        // serialize
        Microsoft::WRL::ComPtr<ID3DBlob> blob{ nullptr };
        Microsoft::WRL::ComPtr<ID3DBlob> error{ nullptr };
        HRESULT r = D3D12SerializeRootSignature(&rs_desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
        if (FAILED(r))
        {
            return AMD::SHADOWFX_RETURN_CODE_D3D12_CALL_FAILED;
        }

        r = dev->CreateRootSignature(gpu_mask, blob->GetBufferPointer(), blob->GetBufferSize(), IID_PPV_ARGS(&rs));
        if (FAILED(r))
        {
            return AMD::SHADOWFX_RETURN_CODE_D3D12_CALL_FAILED;
        }

        return AMD::SHADOWFX_RETURN_CODE_SUCCESS;
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    std::size_t align_to_page(std::size_t v)
    {
        std::size_t const page = 4096;
        return (v + (page - 1)) & ~(page - 1);
    }

    ///////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////////////////

    void set_viewport_scissor(ID3D12GraphicsCommandList* cl, std::size_t w, std::size_t h)
    {
        D3D12_RECT scissor
        {
            0, 0,
            static_cast<int>(w), static_cast<int>(h)
        };
        cl->RSSetScissorRects(1, &scissor);

        D3D12_VIEWPORT viewport
        {
            0, 0,
            static_cast<float>(w), static_cast<float>(h),
            0.f, 1.f
        };
        cl->RSSetViewports(1, &viewport);
    }
}

namespace AMD
{

ShadowFX_OpaqueDesc::ShadowFX_OpaqueDesc(const ShadowFX_Desc & desc)
{
}

ShadowFX_OpaqueDesc::~ShadowFX_OpaqueDesc()
{
    release();
}

SHADOWFX_RETURN_CODE ShadowFX_OpaqueDesc::init(const ShadowFX_Desc & desc)
{
    m_num_cb_instance = desc.m_MaxInstance;

    SHADOWFX_RETURN_CODE r = make_srd_heap(desc.m_pDevice, m_num_srd_heap_slot * m_num_cb_instance, m_srd_heap);
    if(r != SHADOWFX_RETURN_CODE_SUCCESS)
    {
        return r;
    }

    r = cbInitialize(desc);
    if (r != SHADOWFX_RETURN_CODE_SUCCESS)
    {
        return r;
    }

    r = createPSO(desc);

    return r;
}

SHADOWFX_RETURN_CODE ShadowFX_OpaqueDesc::cbInitialize(const ShadowFX_Desc & desc)
{
    auto dev = desc.m_pDevice;
    auto sz = align_to_page(sizeof(ShadowsData));
    auto total_sz = sz * m_num_cb_instance;

    // alloc m_sh_mask_cb_mem
    auto upload_heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto buffer_desc = CD3DX12_RESOURCE_DESC::Buffer(total_sz);
    HRESULT r = dev->CreateCommittedResource(&upload_heap, D3D12_HEAP_FLAG_NONE,
        &buffer_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_sh_mask_cb_mem));
    if (FAILED(r))
    {
        return SHADOWFX_RETURN_CODE_D3D12_CALL_FAILED;
    }

    // sh_mask_cb views
    for (size_t i = 0; i < m_num_cb_instance; ++i)
    {
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc = {};
        cbv_desc.SizeInBytes = static_cast<uint32_t>(sz);
        cbv_desc.BufferLocation = m_sh_mask_cb_mem->GetGPUVirtualAddress() + i * sz;
        dev->CreateConstantBufferView(&cbv_desc, get_cpu_handle(m_srd_heap, i * m_num_srd_heap_slot + 0));
    }

    // map m_sh_mask_cb_mem
    assert(m_sh_mask_cb_ptr == nullptr);
    r = m_sh_mask_cb_mem->Map(0, nullptr, reinterpret_cast<void**>(&m_sh_mask_cb_ptr));
    if (FAILED(r))
    {
        return SHADOWFX_RETURN_CODE_D3D12_CALL_FAILED;
    }

    return SHADOWFX_RETURN_CODE_SUCCESS;
}

SHADOWFX_RETURN_CODE ShadowFX_OpaqueDesc::createPSO(const ShadowFX_Desc & desc)
{
    SHADOWFX_RETURN_CODE r = make_root_signature(desc.m_pDevice, m_sh_mask_rs);
    if (r != SHADOWFX_RETURN_CODE_SUCCESS)
    {
        return r;
    }

    // pso common
    D3D12_SHADER_BYTECODE vs_blob = { VS_FULLSCREEN_Data , sizeof(VS_FULLSCREEN_Data) };
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
    pso_desc.InputLayout = { nullptr, 0 };
    pso_desc.pRootSignature = m_sh_mask_rs.Get();
    pso_desc.VS = vs_blob;
    pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pso_desc.BlendState = desc.m_pOutputBS != nullptr ? *desc.m_pOutputBS : CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pso_desc.DepthStencilState.DepthEnable = false;
    if (desc.m_pOutputDSS != nullptr)
    {
        pso_desc.DepthStencilState = *desc.m_pOutputDSS;
    }
    pso_desc.SampleMask = 0xffffffffu;
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = desc.m_OutputFormat;
    pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    pso_desc.SampleDesc.Count = 1;
    pso_desc.pRootSignature = m_sh_mask_rs.Get();

    // pso ps
    for (int filter = 0; filter < SHADOWFX_FILTERING_COUNT; filter++)
    {
        for (int execution = 0; execution < SHADOWFX_EXECUTION_COUNT; execution++)
        {
            for (int textureFetch = 0; textureFetch < SHADOWFX_TEXTURE_FETCH_COUNT; textureFetch++)
            {
                for (int tapType = 0; tapType < SHADOWFX_TAP_TYPE_COUNT; tapType++)
                {
                    for (int normalOption = 0; normalOption < SHADOWFX_NORMAL_OPTION_COUNT; normalOption++)
                    {
                        for (int filterSize = 0; filterSize < SHADOWFX_FILTER_SIZE_COUNT; filterSize++)
                        {
                            int idx = filterSize;
                            idx += normalOption * SHADOWFX_FILTER_SIZE_COUNT;
                            idx += tapType * SHADOWFX_FILTER_SIZE_COUNT * SHADOWFX_NORMAL_OPTION_COUNT;
                            idx += textureFetch * SHADOWFX_FILTER_SIZE_COUNT * SHADOWFX_NORMAL_OPTION_COUNT * SHADOWFX_TAP_TYPE_COUNT;
                            idx += execution * SHADOWFX_FILTER_SIZE_COUNT * SHADOWFX_NORMAL_OPTION_COUNT * SHADOWFX_TAP_TYPE_COUNT * SHADOWFX_TEXTURE_FETCH_COUNT;
                            idx += filter * SHADOWFX_FILTER_SIZE_COUNT * SHADOWFX_NORMAL_OPTION_COUNT * SHADOWFX_TAP_TYPE_COUNT * SHADOWFX_TEXTURE_FETCH_COUNT * SHADOWFX_EXECUTION_COUNT;

                            assert(idx < std::extent<decltype(PS_SF_T2D_Size)>::value);
                            assert(idx < std::extent<decltype(PS_SF_T2D_Data)>::value);
                            assert(idx < std::extent<decltype(PS_SF_T2DA_Size)>::value);
                            assert(idx < std::extent<decltype(PS_SF_T2DA_Data)>::value);

                            // T2D
                            {
                                auto& pso = psoShadowT2D[filter][execution][textureFetch][tapType][normalOption][filterSize];
                                pso.pso_desc = pso_desc;
                                pso.pso_desc.PS = { PS_SF_T2D_Data[idx] , static_cast<size_t>(PS_SF_T2D_Size[idx]) };
                            }

                            // T2DA
                            {
                                auto& pso = psoShadowT2DA[filter][execution][textureFetch][tapType][normalOption][filterSize];
                                pso.pso_desc = pso_desc;
                                pso.pso_desc.PS = { PS_SF_T2DA_Data[idx] , static_cast<size_t>(PS_SF_T2DA_Size[idx]) };
                            }
                        }
                    }
                }
            }
        }
    }

    for (int execution = 0; execution < SHADOWFX_EXECUTION_COUNT; execution++)
    {

        for (int normalOption = 0; normalOption < SHADOWFX_NORMAL_OPTION_COUNT; normalOption++)
        {
            int idx = normalOption;
            idx += execution * SHADOWFX_NORMAL_OPTION_COUNT;

            // T2D
            {
                auto& pso = psoShadowPointDebugT2D[execution][normalOption];
                pso.pso_desc = pso_desc;
                pso.pso_desc.PS = { PS_SF_T2D_POINT_Data[idx] , static_cast<size_t>(PS_SF_T2D_POINT_Size[idx]) };
            }

            // T2DA
            {
                auto& pso = psoShadowPointDebugT2DA[execution][normalOption];
                pso.pso_desc = pso_desc;
                pso.pso_desc.PS = { PS_SF_T2DA_POINT_Data[idx] , static_cast<size_t>(PS_SF_T2DA_POINT_Size[idx]) };
            }
        }
    }

    return SHADOWFX_RETURN_CODE_SUCCESS;
}

void ShadowFX_OpaqueDesc::release()
{
    // explicitly release data
    m_sh_mask_cb_mem =  nullptr;
    m_srd_heap.heap = nullptr;
    m_srd_heap.num_slot = 0;
    m_sh_mask_rs = nullptr;
    for (int filter = 0; filter < SHADOWFX_FILTERING_COUNT; filter++)
    {
        for (int execution = 0; execution < SHADOWFX_EXECUTION_COUNT; execution++)
        {
            for (int textureFetch = 0; textureFetch < SHADOWFX_TEXTURE_FETCH_COUNT; textureFetch++)
            {
                for (int tapType = 0; tapType < SHADOWFX_TAP_TYPE_COUNT; tapType++)
                {
                    for (int normalOption = 0; normalOption < SHADOWFX_NORMAL_OPTION_COUNT; normalOption++)
                    {
                        for (int filterSize = 0; filterSize < SHADOWFX_FILTER_SIZE_COUNT; filterSize++)
                        {
                            // T2D
                            psoShadowT2D[filter][execution][textureFetch][tapType][normalOption][filterSize].pso = nullptr;

                            // T2DA
                            psoShadowT2DA[filter][execution][textureFetch][tapType][normalOption][filterSize].pso = nullptr;
                        }
                    }
                }
            }
        }
    }
    for (int execution = 0; execution < SHADOWFX_EXECUTION_COUNT; execution++)
    {

        for (int normalOption = 0; normalOption < SHADOWFX_NORMAL_OPTION_COUNT; normalOption++)
        {
            // T2D
            psoShadowPointDebugT2D[execution][normalOption].pso = nullptr;

            // T2DA
            psoShadowPointDebugT2DA[execution][normalOption].pso = nullptr;
        }
    }
}

SHADOWFX_RETURN_CODE ShadowFX_OpaqueDesc::render(const ShadowFX_Desc & desc)
{
    if(desc.m_InstanceID >= desc.m_MaxInstance ||
        desc.m_DepthSize.x == 0 ||
        desc.m_DepthSize.y == 0)
    {
        return SHADOWFX_RETURN_CODE_INVALID_ARGUMENT;
    }

    std::size_t inst_id = desc.m_InstanceID;
    auto cb_sz = align_to_page(sizeof(ShadowsData));
    auto cb_ptr = reinterpret_cast<ShadowsData*>(m_sh_mask_cb_ptr + inst_id * cb_sz);

    ID3D12GraphicsCommandList* cl = desc.m_CommandList;

    desc.m_pDevice->CreateShaderResourceView(desc.m_pDepth, &desc.m_DepthSRV, get_cpu_handle(m_srd_heap, inst_id * m_num_srd_heap_slot + 1));
    desc.m_pDevice->CreateShaderResourceView(desc.m_pShadow, &desc.m_ShadowSRV, get_cpu_handle(m_srd_heap, inst_id * m_num_srd_heap_slot + 3));
    if (desc.m_pNormal != nullptr)
    {
        desc.m_pDevice->CreateShaderResourceView(desc.m_pNormal, &desc.m_NormalSRV, get_cpu_handle(m_srd_heap, inst_id * m_num_srd_heap_slot + 2));
    }

    int filterSize = 0;

    switch (desc.m_FilterSize)
    {
    case SHADOWFX_FILTER_SIZE_7:  filterSize = 0; break;
    case SHADOWFX_FILTER_SIZE_9:  filterSize = 1; break;
    case SHADOWFX_FILTER_SIZE_11: filterSize = 2; break;
    case SHADOWFX_FILTER_SIZE_13: filterSize = 3; break;
    case SHADOWFX_FILTER_SIZE_15: filterSize = 4; break;
    }

    ID3D12PipelineState* pso = nullptr;

    if (desc.m_TextureType == SHADOWFX_TEXTURE_2D)
    {
        if (desc.m_Filtering != SHADOWFX_FILTERING_DEBUG_POINT)
        {
            pso = get(desc.m_pDevice, psoShadowT2D[desc.m_Filtering][desc.m_Execution][desc.m_TextureFetch][desc.m_TapType][desc.m_NormalOption][filterSize]);
        }
        else
        {
            pso = get(desc.m_pDevice, psoShadowPointDebugT2D[desc.m_Execution][desc.m_NormalOption]);
        }
    }
    else if(desc.m_TextureType == SHADOWFX_TEXTURE_2D_ARRAY)
    {
        if (desc.m_Filtering != SHADOWFX_FILTERING_DEBUG_POINT)
        {
            pso = get(desc.m_pDevice, psoShadowT2DA[desc.m_Filtering][desc.m_Execution][desc.m_TextureFetch][desc.m_TapType][desc.m_NormalOption][filterSize]);
        }
        else
        {
            pso = get(desc.m_pDevice, psoShadowPointDebugT2DA[desc.m_Execution][desc.m_NormalOption]);
        }
    }

    assert(pso != nullptr);

    // set viewport and scissor
    if (!desc.m_PreserveViewport)
    {
        set_viewport_scissor(cl, static_cast<size_t>(desc.m_DepthSize.x), static_cast<size_t>(desc.m_DepthSize.y));
    }

    // bind pso
    cl->SetGraphicsRootSignature(m_sh_mask_rs.Get());
    cl->SetPipelineState(pso);

    cb_ptr->m_ActiveLightCount = desc.m_ActiveLightCount;
    memcpy(&cb_ptr->m_Size, &desc.m_DepthSize, sizeof(cb_ptr->m_Size));
    memcpy(&cb_ptr->m_Viewer, &desc.m_Viewer, sizeof(cb_ptr->m_Viewer));
    cb_ptr->m_SizeInv.x = 1.0f / desc.m_DepthSize.x;
    cb_ptr->m_SizeInv.y = 1.0f / desc.m_DepthSize.y;

    for (uint i = 0; i < desc.m_ActiveLightCount; i++)
    {
        float2 shadowSizeInv ={1.0f / cb_ptr->m_Light[i].m_Size.x, 1.0f / cb_ptr->m_Light[i].m_Size.y};

        memcpy(&cb_ptr->m_Light[i].m_Camera, &desc.m_Light[i], sizeof(cb_ptr->m_Light[i].m_Camera));
        memcpy(&cb_ptr->m_Light[i].m_Size, &desc.m_ShadowSize[i], sizeof(cb_ptr->m_Light[i].m_Size));
        memcpy(&cb_ptr->m_Light[i].m_SizeInv, &shadowSizeInv, sizeof(shadowSizeInv));
        memcpy(&cb_ptr->m_Light[i].m_Region, &desc.m_ShadowRegion[i], sizeof(cb_ptr->m_Light[i].m_Region));
        memcpy(&cb_ptr->m_Light[i].m_SunArea, &desc.m_SunArea[i], sizeof(cb_ptr->m_Light[i].m_SunArea));
        memcpy(&cb_ptr->m_Light[i].m_DepthTestOffset, &desc.m_DepthTestOffset[i], sizeof(cb_ptr->m_Light[i].m_DepthTestOffset));
        memcpy(&cb_ptr->m_Light[i].m_NormalOffsetScale, &desc.m_NormalOffsetScale[i], sizeof(cb_ptr->m_Light[i].m_NormalOffsetScale));

        cb_ptr->m_Light[i].m_ArraySlice = desc.m_ArraySlice[i];
        cb_ptr->m_Light[i].m_Weight.x = desc.m_Weight[i];
    }

    // bind srd
    ID3D12DescriptorHeap* heaps[] = { m_srd_heap.heap.Get() };
    cl->SetDescriptorHeaps(1, heaps);
    cl->SetGraphicsRootDescriptorTable(0, get_gpu_handle(m_srd_heap, inst_id * m_num_srd_heap_slot + 0));

    // draw
    cl->IASetVertexBuffers(0, 0, nullptr);
    cl->IASetIndexBuffer(nullptr);
    cl->DrawInstanced(3, 1, 0, 0);

    return SHADOWFX_RETURN_CODE_SUCCESS;
}

}
