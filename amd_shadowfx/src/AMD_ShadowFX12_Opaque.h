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

#ifndef AMD_SHADOWFX_OPAQUE_H
#define AMD_SHADOWFX_OPAQUE_H

#include "AMD_ShadowFX.h"
#include <wrl/client.h>
#include <d3dx12.h>
#include <cstddef>
#include <cstdint>

#pragma warning( disable : 4996 ) // disable stdio deprecated message

namespace AMD
{


// structure holding a PSO object and a description of the states
struct shadowfx_pipeline_state_object
{
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pso{};
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
};

// structure holding a descriptor heap. The number of slots in the heap and the size of one descriptor
struct shadowfx_descriptor_heap
{
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap{};
    std::size_t num_slot = 0; // number of slots in the heap
    std::size_t slot_size = 0; // slot size
};

struct ShadowFX_OpaqueDesc
{
public:
#pragma warning(push)
#pragma warning(disable : 4201)        // suppress nameless struct/union level 4 warnings
    AMD_DECLARE_BASIC_VECTOR_TYPE;
    AMD_DECLARE_CAMERA_TYPE;
#pragma warning(pop)

    typedef struct ShadowsData_t
    {
        Camera                                   m_Viewer;
        float2                                   m_Size; // Viewer Depth Buffer Size
        float2                                   m_SizeInv; // Viewer Depth Buffer Size

        typedef struct LightData_t
        {
            Camera                               m_Camera;
            float2                               m_Size;
            float2                               m_SizeInv;
            float4                               m_Region;

            float4                               m_Weight;

            float                                m_SunArea;
            float                                m_DepthTestOffset;
            float                                m_NormalOffsetScale;
            uint                                 m_ArraySlice;
        } LightData;

        LightData                                m_Light[ShadowFX_Desc::m_MaxLightCount];
        uint                                     m_ActiveLightCount;
        uint                                     _pad[3];
    } ShadowsData;


    // shadow mask constant buffer memory
    Microsoft::WRL::ComPtr<ID3D12Resource> m_sh_mask_cb_mem{ nullptr };
    // command buffer instance
    std::size_t      m_num_cb_instance = 1;
    // cpu mapped command buffer pointer
    std::uint8_t*    m_sh_mask_cb_ptr = nullptr;

    // shader resource descriptor heap for shadow masking
    // layout
    // slot_0: const buffer
    // slot_1: view space depth srv
    // slot_2: normal_map_srv
    // slot_3: light space depth srv
    static std::size_t const m_num_srd_heap_slot = 4; // per instance
    shadowfx_descriptor_heap m_srd_heap;

    // root signature: descriptor table with b0, t0, t1, static sampler with s0, s1
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_sh_mask_rs{};

    shadowfx_pipeline_state_object  psoShadowT2D[SHADOWFX_FILTERING_COUNT][SHADOWFX_EXECUTION_COUNT][SHADOWFX_TEXTURE_FETCH_COUNT][SHADOWFX_TAP_TYPE_COUNT][SHADOWFX_NORMAL_OPTION_COUNT][SHADOWFX_FILTER_SIZE_COUNT];
    shadowfx_pipeline_state_object  psoShadowT2DA[SHADOWFX_FILTERING_COUNT][SHADOWFX_EXECUTION_COUNT][SHADOWFX_TEXTURE_FETCH_COUNT][SHADOWFX_TAP_TYPE_COUNT][SHADOWFX_NORMAL_OPTION_COUNT][SHADOWFX_FILTER_SIZE_COUNT];

    shadowfx_pipeline_state_object  psoShadowPointDebugT2D[SHADOWFX_EXECUTION_COUNT][SHADOWFX_NORMAL_OPTION_COUNT];
    shadowfx_pipeline_state_object  psoShadowPointDebugT2DA[SHADOWFX_EXECUTION_COUNT][SHADOWFX_NORMAL_OPTION_COUNT];

    ShadowFX_OpaqueDesc(const ShadowFX_Desc & desc);
    ~ShadowFX_OpaqueDesc();

    SHADOWFX_RETURN_CODE                         cbInitialize(const ShadowFX_Desc & desc);
    SHADOWFX_RETURN_CODE                         createPSO(const ShadowFX_Desc & desc);
    SHADOWFX_RETURN_CODE                         init(const ShadowFX_Desc & desc);

    SHADOWFX_RETURN_CODE                         render(const ShadowFX_Desc & desc);

    void                                         release();
};

#endif // AMD_SHADOWFX_OPAQUE_H

}
