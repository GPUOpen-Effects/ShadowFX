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

#include "AMD_LIB.h"
#include "AMD_ShadowFX.h"

#pragma warning( disable : 4996 ) // disable stdio deprecated message

namespace AMD
{

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

            float                                m_SunArea;
            float                                m_DepthTestOffset;
            float                                m_NormalOffsetScale;
            uint                                 m_ArraySlice;
        } LightData;

        LightData                                m_Light[ShadowFX_Desc::m_MaxLightCount];
        uint                                     m_ActiveLightCount;
        uint                                     _pad[3];
    } ShadowsData;

    ShadowsData                                  m_ShadowsData;

    ID3D11Buffer*                                m_cbShadowsData;
    ID3D11VertexShader*                          m_vsFullscreen;

    ID3D11PixelShader*                           m_psShadowT2D[SHADOWFX_FILTERING_COUNT][SHADOWFX_EXECUTION_COUNT][SHADOWFX_TEXTURE_FETCH_COUNT][SHADOWFX_TAP_TYPE_COUNT][SHADOWFX_NORMAL_OPTION_COUNT][SHADOWFX_FILTER_SIZE_COUNT];
    ID3D11PixelShader*                           m_psShadowT2DA[SHADOWFX_FILTERING_COUNT][SHADOWFX_EXECUTION_COUNT][SHADOWFX_TEXTURE_FETCH_COUNT][SHADOWFX_TAP_TYPE_COUNT][SHADOWFX_NORMAL_OPTION_COUNT][SHADOWFX_FILTER_SIZE_COUNT];
    //ID3D11PixelShader*                         m_psShadowTC[SHADOWFX_FILTERING_COUNT][SHADOWFX_TEXTURE_FETCH_COUNT][SHADOWFX_TAP_TYPE_COUNT][SHADOWFX_NORMAL_OPTION_COUNT][SHADOWFX_FILTER_SIZE_COUNT];

    //ID3D11ComputeShader*                       m_csShadowT2D[SHADOWFX_FILTERING_COUNT][SHADOWFX_TEXTURE_FETCH_COUNT][SHADOWFX_TAP_TYPE_COUNT][SHADOWFX_NORMAL_OPTION_COUNT][SHADOWFX_FILTER_SIZE_COUNT];
    //ID3D11ComputeShader*                       m_csShadowT2DA[SHADOWFX_FILTERING_COUNT][SHADOWFX_TEXTURE_FETCH_COUNT][SHADOWFX_TAP_TYPE_COUNT][SHADOWFX_NORMAL_OPTION_COUNT][SHADOWFX_FILTER_SIZE_COUNT];
    //ID3D11ComputeShader*                       m_csShadowTC[SHADOWFX_FILTERING_COUNT][SHADOWFX_TEXTURE_FETCH_COUNT][SHADOWFX_TAP_TYPE_COUNT][SHADOWFX_NORMAL_OPTION_COUNT][SHADOWFX_FILTER_SIZE_COUNT];

    ID3D11PixelShader*                           m_psShadowPointDebugT2D[SHADOWFX_EXECUTION_COUNT][SHADOWFX_NORMAL_OPTION_COUNT];
    ID3D11PixelShader*                           m_psShadowPointDebugT2DA[SHADOWFX_EXECUTION_COUNT][SHADOWFX_NORMAL_OPTION_COUNT];
    //ID3D11PixelShader*                         m_psShadowPointDebugTC[SHADOWFX_NORMAL_OPTION_COUNT];

    ID3D11SamplerState*                          m_ssPointClamp;
    ID3D11SamplerState*                          m_ssLinearClamp;
    ID3D11SamplerState*                          m_scsPointBorder;
    ID3D11SamplerState*                          m_scsLinearBorder;
    ID3D11SamplerState*                          m_scsPointClamp;
    ID3D11SamplerState*                          m_scsLinearClamp;

    ID3D11RasterizerState*                       m_rsNoCulling;
    ID3D11DepthStencilState*                     m_dssEqualToRef;
    ID3D11BlendState*                            m_bsOutputChannel[SHADOWFX_OUTPUT_CHANNEL_COUNT];

    ShadowFX_OpaqueDesc(const ShadowFX_Desc & desc);
    ~ShadowFX_OpaqueDesc();

    SHADOWFX_RETURN_CODE                         cbInitialize(const ShadowFX_Desc & desc);
    SHADOWFX_RETURN_CODE                         createShaders(const ShadowFX_Desc & desc);

    SHADOWFX_RETURN_CODE                         render(const ShadowFX_Desc & desc);

    void                                         release();
    void                                         releaseShaders();
};

#endif // AMD_SHADOWFX_OPAQUE_H

}