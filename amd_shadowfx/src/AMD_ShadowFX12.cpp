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
#include <iostream>
#include <fstream>

#define _USE_MATH_DEFINES
#include <cmath>

#ifndef AMD_SHADOWFX_COMPILE_STATIC_LIB
#   define AMD_DLL_EXPORTS
#endif

#include "AMD_ShadowFX12_Opaque.h"

#pragma warning( disable : 4996 )// disable stdio deprecated

namespace AMD
{
    ShadowFX_Desc::ShadowFX_Desc()
        : m_EnableCapture(false)
        , m_TextureType(SHADOWFX_TEXTURE_2D)
        , m_Filtering(SHADOWFX_FILTERING_DEBUG_POINT)
        , m_TextureFetch(SHADOWFX_TEXTURE_FETCH_PCF)
        , m_TapType(SHADOWFX_TAP_TYPE_FIXED)
        , m_FilterSize(SHADOWFX_FILTER_SIZE_7)
        , m_Implementation(SHADOWFX_IMPLEMENTATION_PS)
        , m_Execution(SHADOWFX_EXECUTION_UNION)
        , m_NormalOption(SHADOWFX_NORMAL_OPTION_NONE)
        , m_pDevice(NULL)
        , m_pOpaque(NULL)
        , m_pDepth(NULL)
        , m_pShadow(NULL)
        , m_pNormal(NULL)
        , m_ActiveLightCount(0)
        , m_OutputFormat(DXGI_FORMAT_UNKNOWN)
        , m_pOutputDSS(NULL)
        , m_pOutputBS(NULL)
        , m_MaxInstance(1)
        , m_InstanceID(0)
        , m_PreserveViewport(false)
    {
        static ShadowFX_OpaqueDesc opaque(*this);
        m_pOpaque = &opaque;
    }

    SHADOWFX_RETURN_CODE AMD_SHADOWFX_DLL_API ShadowFX_GetVersion(uint* major, uint* minor, uint* patch)
    {
        if (major == NULL || minor == NULL || patch == NULL)
        {
            return SHADOWFX_RETURN_CODE_INVALID_POINTER;
        }

        *major = AMD_SHADOWFX_VERSION_MAJOR;
        *minor = AMD_SHADOWFX_VERSION_MINOR;
        *patch = AMD_SHADOWFX_VERSION_PATCH;

        return SHADOWFX_RETURN_CODE_SUCCESS;
    }

    SHADOWFX_RETURN_CODE AMD_SHADOWFX_DLL_API ShadowFX_Initialize(const ShadowFX_Desc & desc)
    {
        if (NULL == desc.m_pDevice)
        {
            return SHADOWFX_RETURN_CODE_INVALID_DEVICE;
        }

        return desc.m_pOpaque->init(desc);
    }

    SHADOWFX_RETURN_CODE AMD_SHADOWFX_DLL_API ShadowFX_Release(const ShadowFX_Desc & desc)
    {
        desc.m_pOpaque->release();
        return SHADOWFX_RETURN_CODE_SUCCESS;
    }

    SHADOWFX_RETURN_CODE AMD_SHADOWFX_DLL_API AMD::ShadowFX_Render(const ShadowFX_Desc & desc)
    {
        return desc.m_pOpaque->render(desc);;
    }

}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
