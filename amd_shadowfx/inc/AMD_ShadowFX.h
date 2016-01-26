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

#ifndef __AMD_SHADOWFX_H__
#define __AMD_SHADOWFX_H__

#    define AMD_SHADOWFX_VERSION_MAJOR            1
#    define AMD_SHADOWFX_VERSION_MINOR            3

#    ifdef AMD_SHADOWFX_COMPILE_STATIC_LIB
#        define AMD_SHADOWFX_DLL_API
#    else // AMD_COMPILE_STATIC
#        ifdef AMD_DLL_EXPORTS
#            define AMD_SHADOWFX_DLL_API __declspec(dllexport)
#        else // AMD_DLL_EXPORTS
#            define AMD_SHADOWFX_DLL_API __declspec(dllimport)
#        endif // AMD_DLL_EXPORTS
#    endif // AMD_COMPILE_STATIC

#    include "AMD_Types.h" 

#   if defined(DEBUG) || defined(_DEBUG)
#       define AMD_SHADOWFX_DEBUG                 1
#   endif

namespace AMD
{
// The Return codes
typedef enum _SHADOWFX_RETURN_CODE
{
    SHADOWFX_RETURN_CODE_SUCCESS,
    SHADOWFX_RETURN_CODE_FAIL,
    SHADOWFX_RETURN_CODE_INVALID_DEVICE,
    SHADOWFX_RETURN_CODE_INVALID_DEVICE_CONTEXT,
    SHADOWFX_RETURN_CODE_INVALID_ARGUMENT,
    SHADOWFX_RETURN_CODE_INVALID_POINTER,
    SHADOWFX_RETURN_CODE_D3D11_CALL_FAILED,

    SHADOWFX_RETURN_CODE_COUNT,
} SHADOWFX_RETURN_CODE;

typedef enum _SHADOWFX_IMPLEMENTATION
{
    SHADOWFX_IMPLEMENTATION_PS                   = 0,
    SHADOWFX_IMPLEMENTATION_CS                   = 1, // not available yet
    SHADOWFX_IMPLEMENTATION_COUNT                = 2,
} SHADOWFX_IMPLEMENTATION;

typedef enum _SHADOWFX_EXECUTION
{
    SHADOWFX_EXECUTION_UNION                     = 0,
    SHADOWFX_EXECUTION_CASCADE                   = 1,
    SHADOWFX_EXECUTION_CUBE                      = 2,

    SHADOWFX_EXECUTION_COUNT                     = 3,
} SHADOWFX_EXECUTION;


typedef enum _SHADOWFX_FILTERING
{
    SHADOWFX_FILTERING_UNIFORM                   = 0,
    SHADOWFX_FILTERING_CONTACT                   = 1,
    SHADOWFX_FILTERING_COUNT                     = 2,

    SHADOWFX_FILTERING_DEBUG_POINT               = 10,
    SHADOWFX_FILTERING_DEBUG_COUNT               = 1,
} SHADOWFX_FILTERING;

typedef enum _SHADOWFX_TEXTURE_FETCH
{
    SHADOWFX_TEXTURE_FETCH_GATHER4               = 0,
    SHADOWFX_TEXTURE_FETCH_PCF                   = 1,
    SHADOWFX_TEXTURE_FETCH_COUNT                 = 2,
} SHADOWFX_TEXTURE_FETCH;

typedef enum _SHADOWFX_TEXTURE_TYPE
{
    SHADOWFX_TEXTURE_2D                          = 0,
    SHADOWFX_TEXTURE_2D_ARRAY                    = 1,
    SHADOWFX_TEXTURE_TYPE_COUNT                  = 2,
} SHADOWFX_TEXTURE_TYPE;

// The TAP types
typedef enum _SHADOWFX_TAP_TYPE
{
    SHADOWFX_TAP_TYPE_FIXED                      = 0,
    SHADOWFX_TAP_TYPE_POISSON                    = 1,
    SHADOWFX_TAP_TYPE_COUNT                      = 2,
} SHADOWFX_TAP_TYPE;

typedef enum _SHADOWFX_NORMAL_OPTION
{
    SHADOWFX_NORMAL_OPTION_NONE                  = 0,
    SHADOWFX_NORMAL_OPTION_CALC_FROM_DEPTH       = 1,
    SHADOWFX_NORMAL_OPTION_READ_FROM_SRV         = 2,
    SHADOWFX_NORMAL_OPTION_COUNT                 = 3,
} SHADOWFX_NORMAL_OPTION;

typedef enum _SHADOWFX_FILTER_SIZE
{
    SHADOWFX_FILTER_SIZE_7                       = 7,
    SHADOWFX_FILTER_SIZE_9                       = 9,
    SHADOWFX_FILTER_SIZE_11                      = 11,
    SHADOWFX_FILTER_SIZE_13                      = 13,
    SHADOWFX_FILTER_SIZE_15                      = 15,
    SHADOWFX_FILTER_SIZE_COUNT                   = 5,
} SHADOWFX_FILTER_SIZE;

typedef enum _SHADOWFX_OUTPUT_CHANNEL
{
    SHADOWFX_OUTPUT_CHANNEL_R                    = 1,
    SHADOWFX_OUTPUT_CHANNEL_G                    = 2,
    SHADOWFX_OUTPUT_CHANNEL_B                    = 4,
    SHADOWFX_OUTPUT_CHANNEL_A                    = 8,
    SHADOWFX_OUTPUT_CHANNEL_COUNT                = 16
} SHADOWFX_OUTPUT_CHANNEL;

struct ShadowFX_OpaqueDesc;

struct ShadowFX_Desc
{
    /**
    FX Modules share a variety of trivial types such as vectors and
    camera structures. These types are declared inside an FX descriptor
    in order to avoid any collisions between different modules or app types.
    */
#pragma warning(push)
#pragma warning(disable : 4201)        // suppress nameless struct/union level 4 warnings
    AMD_DECLARE_BASIC_VECTOR_TYPE;
    AMD_DECLARE_CAMERA_TYPE;
#pragma warning(pop)

    static const uint                            m_MaxLightCount = 6; // this has to be at least 6 to allow cube map shadow maps to work

    ID3D11Device*                                m_pDevice;
    ID3D11DeviceContext*                         m_pContext;

    bool                                         m_EnableCapture;

    Camera                                       m_Viewer;
    float2                                       m_DepthSize; // Viewer Depth Buffer Size

    Camera                                       m_Light[m_MaxLightCount];
    float2                                       m_ShadowSize[m_MaxLightCount];
    float4                                       m_ShadowRegion[m_MaxLightCount];
    float                                        m_SunArea[m_MaxLightCount];
    float                                        m_DepthTestOffset[m_MaxLightCount];
    float                                        m_NormalOffsetScale[m_MaxLightCount];
    uint                                         m_ArraySlice[m_MaxLightCount];
    uint                                         m_ActiveLightCount;

    SHADOWFX_EXECUTION                           m_Execution;
    SHADOWFX_IMPLEMENTATION                      m_Implementation;
    SHADOWFX_TEXTURE_TYPE                        m_TextureType;
    SHADOWFX_TEXTURE_FETCH                       m_TextureFetch;
    SHADOWFX_FILTERING                           m_Filtering;
    SHADOWFX_TAP_TYPE                            m_TapType;
    SHADOWFX_FILTER_SIZE                         m_FilterSize;
    SHADOWFX_NORMAL_OPTION                       m_NormalOption;

    ID3D11ShaderResourceView*                    m_pDepthSRV;  // input main viewer zbuffer
    ID3D11ShaderResourceView*                    m_pShadowSRV; // input shadow map
    ID3D11ShaderResourceView*                    m_pNormalSRV; // input main viewer normal data (for Deferred Renderers)

    ID3D11RenderTargetView*                      m_pOutputRTV; // output shadow mask 

    ID3D11DepthStencilState*                     m_pOutputDSS; // output dss can specify stencil test 
    ID3D11DepthStencilView*                      m_pOutputDSV; // output depth stencil view (used if dss != null) it should have stencil data
    unsigned int                                 m_ReferenceDSS; // stencil reference value (used if dss != null)

    ID3D11BlendState*                            m_pOutputBS;      // output bs can specify how to write to rtv
    unsigned int                                 m_OutputChannels; // output channels flags (not used if bs != null)

    AMD_SHADOWFX_DLL_API                         ShadowFX_Desc();

    /**
    All ShadowFX_Desc objects have a pointer to a single instance of a AOFX_OpaqueDesc.
    */
    ShadowFX_OpaqueDesc*                         m_pOpaque;

private:
    /**
    Copy constructor and assign operator are declared but lack implementation.
    This is done on purpose in order to avoid invoking them by accident.
    */
    AMD_SHADOWFX_DLL_API                         ShadowFX_Desc(const ShadowFX_Desc &);
    AMD_SHADOWFX_DLL_API                         ShadowFX_Desc & operator= (const ShadowFX_Desc &);
};

extern "C"
{
    /**
    Initialize internal data inside AOFX_Desc
    Calling this function requires setting up m_pDevice member
    */
    AMD_SHADOWFX_DLL_API SHADOWFX_RETURN_CODE ShadowFX_Initialize     (const ShadowFX_Desc & desc);

    /**
    Execute ShadowFX rendering for a given ShadowFX_Desc parameters descriptior
    Calling this function requires setting up:
    * m_pDeviceContext must be set to a valid immediate context
    * m_pDepthSRV must be set to a valid shader resource view pointing to a depth buffer resource
    * m_DepthSize must be set to the correct size of m_pDepthSRV resource
    * m_pShadowSRV must be set to a valid shader resource view pointing to a depth buffer resource
    * m_pOutputRTV must be set to a valid render target view pointint to a resource of m_InputSize size
    * m_Viewer - viewer camera parameters
    * m_ActiveLightCount must indicate a valid active light count (0, 6]
      for each active light application must specify values in:
        ** m_Light[] - light camera parameters
        ** m_ShadowSize[] - size of the shadow map
        ** m_ShadowRegion[] - dimensions of a shadow region in uv space [0, 1]x[0, 1] .xy - offset .zw - size
        ** m_SunArea[] - a scalar that controls the simulated light size (affects contact hardening algorithms)
        ** m_DepthTestOffset[] - a scalar that is used in the shader to slightly adjust z test
        ** m_NormalOffsetScale[] - a scalar that is used to displace tested position in world space before projecting to light space.
                                   It used when Normal option is set to either CALC_FROM_DEPTH or READ_FROM_SRV
        ** m_ArraySlice[] - texture array index. It is used when m_TextureType is equal to ARRAY
    Optionally application can change:
    * m_Execution - lights can be arranged either:
        ** as a union of shadow casters (each shadowed pixel is tested against each shadow caster)
        ** as cascades for a single directional light (each shadowed pixel is tested until it falls into a single cascade)
        ** as a cube map light (each shadowed pixel selects the correct shadow cube face and only uses that for filtering)                                        
    * m_Implementation - alternate between compute shader and pixel shader implementations (CS is under development)
    * m_TextureType - shadow map(s) can be stored in either a texture 2d (atlas) or a texture 2d array
    * m_TextureFetch - alternate between shader permutations that use Gather 4 or PCF instructions
    * m_Filtering - alternate between uniform and contact hardening shadows;
    * m_TapType - alternate between sampling all texels inside the filtering kernel or fetch poisson distributed samples (less samples) 
    * m_FilterSize - select filter size from 7x7 to 15x15
    * m_NormalOption - each visible pixel on the screen is first reprojected in World Space. At this point it can be displaced along the normal 
                       to help reduce incorrect self shadowing. Normal can either be calculated from depth buffer or fetched from SRV
    * m_pNormalSRV - set to a valid SRV with normal gbuffer layer to use normal option READ_FROM_SRV 
    
    * m_pOutputDSS - set to a valid depth stencil state to enable performance optimizations. 
                     For example this can be used to enable stencil testing to reduce the number of filtered pixels on the screen
    * m_pOutputDSV - set to a valid depth stencil view to use in conjunction with m_pOutputDSS 
    * m_ReferenceDSS - stencil reference value (used if m_pOutputDSS != NULL)
    * m_OutputChannels - set output write mask
    * m_pOutputBS - set the whole blend state (will override m_OutputChannels)
    
    */
    AMD_SHADOWFX_DLL_API SHADOWFX_RETURN_CODE ShadowFX_Render         (const ShadowFX_Desc & desc);
    
    /**
    Release all internal data used by AOFX_OpaqueDesc
    */
    AMD_SHADOWFX_DLL_API SHADOWFX_RETURN_CODE ShadowFX_Release        (const ShadowFX_Desc & desc);
}

}

#endif