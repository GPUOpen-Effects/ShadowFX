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

#include <d3d11.h>
#include <string>
#include <fstream>

#define _USE_MATH_DEFINES
#include <cmath>

#include "AMD_LIB.h"

#ifndef AMD_SHADOWFX_COMPILE_STATIC_LIB
#   define AMD_DLL_EXPORTS
#endif

#include "AMD_ShadowFX11_Opaque.h"
#include "AMD_ShadowFX_Precompiled.h"

#pragma warning( disable : 4100 ) // disable unreference formal parameter warnings for /W4 builds

namespace AMD
{
ShadowFX_OpaqueDesc::ShadowFX_OpaqueDesc(const ShadowFX_Desc & desc)
    : m_ssLinearClamp(NULL)
    , m_ssPointClamp(NULL)
    , m_scsPointBorder(NULL)
    , m_scsLinearBorder(NULL)
    , m_scsPointClamp(NULL)
    , m_scsLinearClamp(NULL)
    , m_vsFullscreen(NULL)
    , m_cbShadowsData(NULL)
    , m_rsNoCulling(NULL)
    , m_dssEqualToRef(NULL)
{
    for (int execution = 0; execution < SHADOWFX_EXECUTION_COUNT; execution++)
    {
        for (int filter = 0; filter < SHADOWFX_FILTERING_COUNT; filter++)
        {
            for (int textureFetch = 0; textureFetch < SHADOWFX_TEXTURE_FETCH_COUNT; textureFetch++)
            {
                for (int tapType = 0; tapType < SHADOWFX_TAP_TYPE_COUNT; tapType++)
                {
                    for (int normalOption = 0; normalOption < SHADOWFX_NORMAL_OPTION_COUNT; normalOption++)
                    {
                        for (int filterSize = 0; filterSize < SHADOWFX_FILTER_SIZE_COUNT; filterSize++)
                        {
                            m_psShadowT2D[filter][execution][textureFetch][tapType][normalOption][filterSize] = NULL;
                            m_psShadowT2DA[filter][execution][textureFetch][tapType][normalOption][filterSize] = NULL;
                        }
                    }
                }
            }
        }

        for (int normalOption = 0; normalOption < SHADOWFX_NORMAL_OPTION_COUNT; normalOption++)
        {
            m_psShadowPointDebugT2D[execution][normalOption] = NULL;
            m_psShadowPointDebugT2DA[execution][normalOption] = NULL;
        }

    }

    for (int i = 0; i < SHADOWFX_OUTPUT_CHANNEL_COUNT; i++)
    {
        m_bsOutputChannel[i] = NULL;
    }
}

ShadowFX_OpaqueDesc::~ShadowFX_OpaqueDesc()
{
    release();
}

SHADOWFX_RETURN_CODE ShadowFX_OpaqueDesc::cbInitialize(const ShadowFX_Desc & desc)
{
    HRESULT hr = S_OK;

    CD3D11_DEFAULT d3d11Default;

    D3D11_BUFFER_DESC b1dDesc;
    b1dDesc.Usage = D3D11_USAGE_DYNAMIC;
    b1dDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    b1dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    b1dDesc.MiscFlags = 0;
    b1dDesc.ByteWidth = sizeof(m_ShadowsData);
    hr = (desc.m_pDevice->CreateBuffer(&b1dDesc, NULL, &m_cbShadowsData));
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    CD3D11_SAMPLER_DESC ssDesc(d3d11Default);
    ssDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    hr = (desc.m_pDevice->CreateSamplerState(&ssDesc, &m_ssPointClamp));
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    ssDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    hr = (desc.m_pDevice->CreateSamplerState(&ssDesc, &m_ssLinearClamp));
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    ssDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    ssDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = (desc.m_pDevice->CreateSamplerState(&ssDesc, &m_scsPointClamp));
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    ssDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ssDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = (desc.m_pDevice->CreateSamplerState(&ssDesc, &m_scsLinearClamp));
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    ssDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
    ssDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
    ssDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
    ssDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    ssDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = (desc.m_pDevice->CreateSamplerState(&ssDesc, &m_scsPointBorder));
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    ssDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    ssDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    hr = (desc.m_pDevice->CreateSamplerState(&ssDesc, &m_scsLinearBorder));
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    CD3D11_BLEND_DESC bsDesc(d3d11Default);
    m_bsOutputChannel[0] = NULL;
    for (int blend = SHADOWFX_OUTPUT_CHANNEL_R; blend < SHADOWFX_OUTPUT_CHANNEL_COUNT; blend++)
    {
        bsDesc.RenderTarget[0].RenderTargetWriteMask = (UINT8)blend;
        hr = desc.m_pDevice->CreateBlendState(&bsDesc, &m_bsOutputChannel[blend]);
        if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;
    }

    CD3D11_RASTERIZER_DESC rsDesc(d3d11Default);
    rsDesc.CullMode = D3D11_CULL_NONE;
    hr = desc.m_pDevice->CreateRasterizerState(&rsDesc, &m_rsNoCulling);
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    CD3D11_DEPTH_STENCIL_DESC dssDesc(d3d11Default);
    dssDesc.DepthEnable = false;
    dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dssDesc.StencilEnable = true;
    dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    dssDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;

    hr = desc.m_pDevice->CreateDepthStencilState(&dssDesc, &m_dssEqualToRef);
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    return SHADOWFX_RETURN_CODE_SUCCESS;
}

SHADOWFX_RETURN_CODE ShadowFX_OpaqueDesc::createShaders(const ShadowFX_Desc & desc)
{
    HRESULT hr = S_OK;

    releaseShaders();

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

                            hr = desc.m_pDevice->CreatePixelShader(PS_SF_T2D_Data[idx], PS_SF_T2D_Size[idx], NULL, &m_psShadowT2D[filter][execution][textureFetch][tapType][normalOption][filterSize]);
                            if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

                            hr = desc.m_pDevice->CreatePixelShader(PS_SF_T2DA_Data[idx], PS_SF_T2DA_Size[idx], NULL, &m_psShadowT2DA[filter][execution][textureFetch][tapType][normalOption][filterSize]);
                            if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;
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
            hr = desc.m_pDevice->CreatePixelShader(PS_SF_T2D_POINT_Data[idx], PS_SF_T2D_POINT_Size[idx], NULL, &m_psShadowPointDebugT2D[execution][normalOption]);
            if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

            hr  = desc.m_pDevice->CreatePixelShader(PS_SF_T2DA_POINT_Data[idx], PS_SF_T2DA_POINT_Size[idx], NULL, &m_psShadowPointDebugT2DA[execution][normalOption]);
            if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;
        }
    }

    hr = desc.m_pDevice->CreateVertexShader(VS_FULLSCREEN_Data, sizeof(VS_FULLSCREEN_Data), NULL, &m_vsFullscreen);
    if (hr != S_OK) return SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED;

    return SHADOWFX_RETURN_CODE_SUCCESS;
}

void ShadowFX_OpaqueDesc::release()
{
    releaseShaders();

    AMD_SAFE_RELEASE(m_cbShadowsData);
    AMD_SAFE_RELEASE(m_ssLinearClamp);
    AMD_SAFE_RELEASE(m_ssPointClamp);
    AMD_SAFE_RELEASE(m_scsLinearClamp);
    AMD_SAFE_RELEASE(m_scsPointClamp);
    AMD_SAFE_RELEASE(m_scsPointBorder);
    AMD_SAFE_RELEASE(m_scsLinearBorder);
    AMD_SAFE_RELEASE(m_rsNoCulling);
    AMD_SAFE_RELEASE(m_dssEqualToRef);
    for (int i = 0; i < SHADOWFX_OUTPUT_CHANNEL_COUNT; i++)
        AMD_SAFE_RELEASE(m_bsOutputChannel[i]);
}

void    ShadowFX_OpaqueDesc::releaseShaders()
{
    AMD_SAFE_RELEASE(m_vsFullscreen);

    for (int execution = 0; execution < SHADOWFX_EXECUTION_COUNT; execution++)
    {
        for (int filter = 0; filter < SHADOWFX_FILTERING_COUNT; filter++)
        {
            for (int textureFetch = 0; textureFetch < SHADOWFX_TEXTURE_FETCH_COUNT; textureFetch++)
            {
                for (int tapType = 0; tapType < SHADOWFX_TAP_TYPE_COUNT; tapType++)
                {
                    for (int normalOption = 0; normalOption < SHADOWFX_NORMAL_OPTION_COUNT; normalOption++)
                    {
                        for (int filterSize = 0; filterSize < SHADOWFX_FILTER_SIZE_COUNT; filterSize++)
                        {
                            AMD_SAFE_RELEASE(m_psShadowT2D[filter][execution][textureFetch][tapType][normalOption][filterSize]);
                            AMD_SAFE_RELEASE(m_psShadowT2DA[filter][execution][textureFetch][tapType][normalOption][filterSize]);
                            //AMD_SAFE_RELEASE( m_psShadowTC[filter][textureFetch][tapType][normalOption][filterSize] );
                            //AMD_SAFE_RELEASE( m_csShadowT2D[filter][textureFetch][tapType][normalOption][filterSize] );
                            //AMD_SAFE_RELEASE( m_csShadowT2DA[filter][textureFetch][tapType][normalOption][filterSize] );
                            //AMD_SAFE_RELEASE( m_csShadowTC[filter][textureFetch][tapType][normalOption][filterSize] );
                        }
                    }
                }
            }
        }

        for (int normalOption = 0; normalOption < SHADOWFX_NORMAL_OPTION_COUNT; normalOption++)
        {
            AMD_SAFE_RELEASE(m_psShadowPointDebugT2D[execution][normalOption]);
            AMD_SAFE_RELEASE(m_psShadowPointDebugT2DA[execution][normalOption]);
        }
    }

    AMD_SAFE_RELEASE(m_vsFullscreen);
}

SHADOWFX_RETURN_CODE ShadowFX_OpaqueDesc::render(const ShadowFX_Desc & desc)
{
    if (desc.m_DepthSize.x == 0 ||
        desc.m_DepthSize.y == 0)
    {
        return SHADOWFX_RETURN_CODE_INVALID_ARGUMENT;
    }


    CD3D11_VIEWPORT FullscreenVP(0.0f, 0.0f, desc.m_DepthSize.x, desc.m_DepthSize.y);

    /*
      TODO: ADD MEMCMP CHECK THAT CONSTANT BUFFER VALUES ACTUALLY CHANGED
    */

    m_ShadowsData.m_ActiveLightCount = desc.m_ActiveLightCount;
    memcpy(&m_ShadowsData.m_Size, &desc.m_DepthSize, sizeof(m_ShadowsData.m_Size));
    memcpy(&m_ShadowsData.m_Viewer, &desc.m_Viewer, sizeof(m_ShadowsData.m_Viewer));
    m_ShadowsData.m_SizeInv.x = 1.0f / desc.m_DepthSize.x;
    m_ShadowsData.m_SizeInv.y = 1.0f / desc.m_DepthSize.y;

    for (uint i = 0; i < desc.m_ActiveLightCount; i++)
    {
        float2 shadowSizeInv ={1.0f / m_ShadowsData.m_Light[i].m_Size.x, 1.0f / m_ShadowsData.m_Light[i].m_Size.y};

        memcpy(&m_ShadowsData.m_Light[i].m_Camera, &desc.m_Light[i], sizeof(m_ShadowsData.m_Light[i].m_Camera));
        memcpy(&m_ShadowsData.m_Light[i].m_Size, &desc.m_ShadowSize[i], sizeof(m_ShadowsData.m_Light[i].m_Size));
        memcpy(&m_ShadowsData.m_Light[i].m_SizeInv, &shadowSizeInv, sizeof(shadowSizeInv));
        memcpy(&m_ShadowsData.m_Light[i].m_Region, &desc.m_ShadowRegion[i], sizeof(m_ShadowsData.m_Light[i].m_Region));
        memcpy(&m_ShadowsData.m_Light[i].m_SunArea, &desc.m_SunArea[i], sizeof(m_ShadowsData.m_Light[i].m_SunArea));
        memcpy(&m_ShadowsData.m_Light[i].m_DepthTestOffset, &desc.m_DepthTestOffset[i], sizeof(m_ShadowsData.m_Light[i].m_DepthTestOffset));
        memcpy(&m_ShadowsData.m_Light[i].m_NormalOffsetScale, &desc.m_NormalOffsetScale[i], sizeof(m_ShadowsData.m_Light[i].m_NormalOffsetScale));

        m_ShadowsData.m_Light[i].m_ArraySlice = desc.m_ArraySlice[i];
    }

    D3D11_MAPPED_SUBRESOURCE MappedResource;
    desc.m_pContext->Map(m_cbShadowsData, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    memcpy(MappedResource.pData, &m_ShadowsData, sizeof(m_ShadowsData));
    desc.m_pContext->Unmap(m_cbShadowsData, 0);

    int filterSize = 0;

    switch (desc.m_FilterSize)
    {
    case SHADOWFX_FILTER_SIZE_7:  filterSize = 0; break;
    case SHADOWFX_FILTER_SIZE_9:  filterSize = 1; break;
    case SHADOWFX_FILTER_SIZE_11: filterSize = 2; break;
    case SHADOWFX_FILTER_SIZE_13: filterSize = 3; break;
    case SHADOWFX_FILTER_SIZE_15: filterSize = 4; break;
    }

    ID3D11PixelShader* psSelect = NULL;

    switch (desc.m_TextureType)
    {
    case SHADOWFX_TEXTURE_2D:
    {
        if (desc.m_Filtering != SHADOWFX_FILTERING_DEBUG_POINT)
            psSelect = m_psShadowT2D[desc.m_Filtering][desc.m_Execution][desc.m_TextureFetch][desc.m_TapType][desc.m_NormalOption][filterSize];
        else
            psSelect = m_psShadowPointDebugT2D[desc.m_Execution][desc.m_NormalOption];
        break;
    }

    case SHADOWFX_TEXTURE_2D_ARRAY:
    {
        if (desc.m_Filtering != SHADOWFX_FILTERING_DEBUG_POINT)
            psSelect = m_psShadowT2DA[desc.m_Filtering][desc.m_Execution][desc.m_TextureFetch][desc.m_TapType][desc.m_NormalOption][filterSize];
        else
            psSelect = m_psShadowPointDebugT2DA[desc.m_Execution][desc.m_NormalOption];
        break;
    }
    }
    ID3D11Buffer* cb[] ={m_cbShadowsData};
    ID3D11SamplerState * ss[] ={m_ssPointClamp, m_ssLinearClamp, m_scsPointClamp, m_scsLinearClamp};
    ID3D11RenderTargetView* rtv[] ={desc.m_pOutputRTV};

    ID3D11BlendState * bsSelect = desc.m_pOutputBS != NULL ? desc.m_pOutputBS : m_bsOutputChannel[desc.m_OutputChannels];

    ID3D11ShaderResourceView* srv[] ={desc.m_pDepthSRV, desc.m_pNormalSRV, desc.m_pShadowSRV};

    HRESULT hr = AMD::RenderFullscreenPass(desc.m_pContext,
        FullscreenVP, m_vsFullscreen, psSelect,
        NULL, 0, cb, AMD_ARRAY_SIZE(cb),
        ss, AMD_ARRAY_SIZE(ss),
        srv, AMD_ARRAY_SIZE(srv),
        rtv, AMD_ARRAY_SIZE(rtv),
        NULL, 0, 0,
        desc.m_pOutputDSV,
        desc.m_pOutputDSS, desc.m_ReferenceDSS,
        bsSelect,
        m_rsNoCulling);

    return hr == S_OK ? SHADOWFX_RETURN_CODE_SUCCESS : SHADOWFX_RETURN_CODE_FAIL;
}
}