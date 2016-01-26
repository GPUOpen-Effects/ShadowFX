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

// DXUT includes
#include "DXUT.h"
#include "DXUTmisc.h"
#include "DXUTgui.h"
#include "DXUTCamera.h"
#include "DXUTSettingsDlg.h"
#include "SDKmisc.h"
#include "SDKmesh.h"

// AMD includes
#include "AMD_LIB.h"
#include "AMD_SDK.h"
#include "amd_ags.h"

#include "AMD_ShadowFX.h"

#include <DirectXMath.h>
using namespace DirectX;

#pragma warning( disable : 4100 ) // disable unreferenced formal parameter warnings
#pragma warning( disable : 4127 ) // disable conditional expression is constant warnings
#pragma warning( disable : 4201 ) // disable nameless struct/union warnings
#pragma warning( disable : 4238 ) // disable class rvalue used as lvalue warnings
struct float2
{
    union
    {
        struct { float x, y; };
        float f[2];
    };

    float2(float ax = 0.0f, float ay = 0.0f) : x(ax), y(ay) {}
};

__declspec(align(16))
struct float4
{
    union
    {
        XMVECTOR     v;
        float        f[4];
        struct { float x, y, z, w; };
    };

    float4(float ax = 0, float ay = 0, float az = 0, float aw = 0)
    {
        x = ax; y = ay; z = az; w = aw;
    }

    float4(XMVECTOR av) { v = av; }
    float4(XMVECTOR xyz, float aw) { v = xyz; w = aw; }

    inline operator XMVECTOR() const { return v; }
    inline operator const float*() const { return f; }
#if !defined(_XM_NO_INTRINSICS_) && defined(_XM_SSE_INTRINSICS_)
    inline operator __m128i() const { return _mm_castps_si128(v); }
    inline operator __m128d() const { return _mm_castps_pd(v); }
#endif
};

#define float4x4  XMMATRIX

__declspec(align(16))
struct S_CAMERA_DATA
{
    float4x4    m_View;
    float4x4    m_Projection;
    float4x4    m_ViewInv;
    float4x4    m_ProjectionInv;
    float4x4    m_ViewProjection;
    float4x4    m_ViewProjectionInv;

    float2      m_BackBufferDim;
    float2      m_BackBufferDimRcp;
    float4      m_Color;

    float4      m_Position;
    float4      m_Direction;
    float4      m_Up;
    float       m_Fov;
    float       m_Aspect;
    float       m_zNear;
    float       m_zFar;

    float4      m_Parameter0;
    float4      m_Parameter1;
    float4      m_Parameter2;
    float4      m_Parameter3;
};

__declspec(align(16))
struct S_MODEL_DATA
{
    float4x4    m_World;
    float4x4    m_WorldViewProjection;
    float4x4    m_WorldViewProjectionLight;
    float4      m_Diffuse;
    float4      m_Ambient;
    float4      m_Parameter0;
};

__declspec(align(16))
struct S_UNIT_CUBE_TRANSFORM
{
    float4x4    m_Transform;
    float4x4    m_Inverse;
    float4x4    m_Forward;
    float4      m_Color;
};

AGSContext*                                      g_agsContext;

CDXUTDialogResourceManager                       g_DialogResourceManager;    // Manager for shared resources of dialogs
CD3DSettingsDlg                                  g_SettingsDlg;              // Device settings dialog
CDXUTTextHelper*                                 g_pTxtHelper = NULL;

#define CUBE_FACE_COUNT 6
CFirstPersonCamera                               g_LightCamera;                // A model viewing camera for the light
CFirstPersonCamera                               g_ViewerCamera;               // A first person viewing camera
CFirstPersonCamera*                              g_pCurrentCamera = &g_ViewerCamera;
int                                              g_CurrentLightCamera;
CFirstPersonCamera                               g_CubeCamera[CUBE_FACE_COUNT];
float4x4                                         g_LightOrtho[CUBE_FACE_COUNT];
S_CAMERA_DATA                                    g_ViewerData, g_LightData[CUBE_FACE_COUNT];
bool                                             g_bShowLightCamera = false;
bool                                             g_bShowShadowMapRegion = false;
bool                                             g_bShowShadowMask = false;

const float4                                     red(1.00f, 0.00f, 0.00f, 1.00f);
const float4                                     orange(1.00f, 0.50f, 0.00f, 1.00f);
const float4                                     yellow(1.00f, 1.00f, 0.00f, 1.00f);
const float4                                     bright_green(0.50f, 1.00f, 0.00f, 1.00f);
const float4                                     green(0.00f, 1.00f, 0.00f, 1.00f);
const float4                                     mint(0.00f, 1.00f, 0.50f, 1.00f);
const float4                                     cyan(0.00f, 1.00f, 1.00f, 1.00f);
const float4                                     deep_blue(0.00f, 0.50f, 1.00f, 1.00f);
const float4                                     blue(0.00f, 0.00f, 1.00f, 1.00f);
const float4                                     purple(0.50f, 0.00f, 1.00f, 1.00f);
const float4                                     magenta(1.00f, 0.00f, 1.00f, 1.00f);
const float4                                     white(1.00f, 1.00f, 1.00f, 1.00f);
const float4                                     grey(0.50f, 0.50f, 0.50f, 1.00f);
const float4                                     black(0.000f, 0.000f, 0.000f, 0.000f);
const float4                                     light_blue(0.176f, 0.196f, 0.667f, 0.000f);

const float4                                     g_Color[] =
{
  white, red, green, blue, magenta, cyan, yellow
};

// AMD helper classes defined here
static AMD::Magnify                              g_Magnify;
static AMD::MagnifyTool                          g_MagnifyTool;
static AMD::HUD                                  g_HUD;

// Global boolean for HUD rendering
bool                                             g_bRenderHUD = true;

ID3D11InputLayout*                               g_pSdkMeshIL = NULL;
ID3D11InputLayout*                               g_pSceneIL = NULL;

ID3D11SamplerState*                              g_pLinearWrapSS = NULL;

ID3D11BlendState*                                g_pOpaqueBS = NULL;
ID3D11BlendState*                                g_pShadowMaskChannelBS[4] = { 0,0,0,0 };

ID3D11VertexShader*                              g_pSceneVS = NULL;
ID3D11VertexShader*                              g_pShadowMapVS = NULL;

ID3D11PixelShader*                               g_pDepthPassScenePS = NULL;
ID3D11PixelShader*                               g_pDepthAndNormalPassScenePS = NULL;
ID3D11PixelShader*                               g_pShadowedScenePS = NULL;
ID3D11PixelShader*                               g_pShadowMapPS = NULL;

ID3D11VertexShader*                              g_pUnitCubeVS = NULL;
ID3D11VertexShader*                              g_pFullscreenVS = NULL;
ID3D11PixelShader*                               g_pUnitCubePS = NULL;
ID3D11PixelShader*                               g_pFullscreenPS = NULL;

// Constant Buffer
ID3D11Buffer*                                    g_pModelCB = NULL;
ID3D11Buffer*                                    g_pViewerCB = NULL;
ID3D11Buffer*                                    g_pLightCB = NULL;
ID3D11Buffer*                                    g_pUnitCubeCB = NULL;

ID3D11RasterizerState*                           g_pNoCullingSolidRS = NULL;
ID3D11RasterizerState*                           g_pBackCullingSolidRS = NULL;
ID3D11RasterizerState*                           g_pFrontCullingSolidRS = NULL;
ID3D11RasterizerState*                           g_pNoCullingWireframeRS = NULL;

ID3D11DepthStencilState*                         g_pDepthTestMarkStencilDSS = NULL;
ID3D11DepthStencilState*                         g_pStencilTestAndClearDSS = NULL;
ID3D11DepthStencilState*                         g_pDepthTestLessDSS = NULL;
ID3D11DepthStencilState*                         g_pDepthTestLessEqualDSS = NULL;

AMD::Texture2D                                   g_ShadowMask, g_AppDepth, g_AppNormal, g_LightColor, g_LightDepth;

AMD::Mesh                                        g_Tree, g_BoxPlane;
AMD::Mesh*                                       g_MeshArray[] = { &g_Tree, /*&g_Tree, &g_Tree, &g_Tree,*/  &g_BoxPlane, &g_BoxPlane  }; // TODO: rearrange this for a proper instanced rendering
XMMATRIX                                         g_MeshModelMatrix[AMD_ARRAY_SIZE(g_MeshArray)];
AMD::Texture2D                                   g_ShadowMap;

float                                            g_ShadowMapSize = 1024;
int                                              g_ShadowMapAtlasScaleW = CUBE_FACE_COUNT / 2, g_ShadowMapAtlasScaleH = CUBE_FACE_COUNT / g_ShadowMapAtlasScaleW;

int                                              g_agsGpuCount = 0;

#define  EXPERIMENTAL_DELAY_END_ALL_ACCESS    0
#define  EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY 0 // this is an experimental mode to reduce the amount of code needed to implement the CFX API

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1)
AMD::Texture2D                                   g_ShadowMapCopy; // in experimental mode, there is an extra resource used for shadow map transfer
AGSAfrTransferType                               g_ShadowMapCopyCfxTransfer = AGS_AFR_TRANSFER_2STEP_NO_BROADCAST; // this extra resource actually will be transfered around
AGSAfrTransferType                               g_ShadowMapCfxTransfer = AGS_AFR_TRANSFER_DISABLE; // while the original shadow map resource will be available to use by the current frame
#elif (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 0)
AGSAfrTransferType                               g_ShadowMapCfxTransfer = AGS_AFR_TRANSFER_2STEP_NO_BROADCAST; // if experimental is disabled the shadow map will be transferred directly
#endif

AMD::ShadowFX_Desc                               g_ShadowsDesc;
AMD::SHADOWFX_EXECUTION                          g_ShadowsExecution = AMD::SHADOWFX_EXECUTION_UNION;

// App variables
float                                            g_SunSize = 4.0f;
float                                            g_DepthTestOffset = 0.00000f;
float                                            g_NormalOffsetScale = 0.001f;
int                                              g_Height = 1080, g_Width = 1920;

//--------------------------------------------------------------------------------------
// Timing data
//--------------------------------------------------------------------------------------
float                                            g_ShadowRenderingTime = 0.0f;
float                                            g_ShadowFilteringTime = 0.0f;
float                                            g_DepthPrepassRenderingTime = 0.0f;
float                                            g_ShadowMapMasking = 0.0f;
float                                            g_SceneRendering = 0.0f;

//--------------------------------------------------------------------------------------
// UI control IDs
//--------------------------------------------------------------------------------------
// Standard device control
enum
{
    IDC_TOGGLEFULLSCREEN = 1,
    IDC_TOGGLEREF,
    IDC_CHANGEDEVICE,

    // Sample UI
    IDC_STATIC_SHADOW_MAP,
    IDC_RADIO_SHADOW_MAP_T2D,
    IDC_RADIO_SHADOW_MAP_T2DA,

    IDC_STATIC_SHADOWFX_TYPE,
    IDC_RADIO_SHADOWFX_TYPE_UNION,
    IDC_RADIO_SHADOWFX_TYPE_CASCADE,
    IDC_RADIO_SHADOWFX_TYPE_CUBE,

    IDC_CHECKBOX_FILTERING,
    IDC_STATIC_SUN_SIZE,
    IDC_SLIDER_SUN_SIZE,
    IDC_CHECKBOX_DYNAMIC_FILTERING,

    IDC_STATIC_DEPTH_TEST_OFFSET,
    IDC_SLIDER_DEPTH_OFFSET_TEST,

    IDC_STATIC_NORMAL_OFFSET_SCALE,
    IDC_SLIDER_NORMAL_OFFSET_SCALE,

    IDC_STATIC_SHADOWFX_FILTER_SIZE,
    IDC_COMBOBOX_SHADOWFX_FILTER_SIZE,

    IDC_STATIC_SHADOWFX_FILTERING,
    IDC_RADIO_SHADOWFX_FILTERING_UNIFORM,
    IDC_RADIO_SHADOWFX_FILTERING_CONTACT,

    IDC_STATIC_SHADOWFX_TEXTURE_FETCH,
    IDC_RADIO_SHADOWFX_TEXTURE_FETCH,
    IDC_RADIO_SHADOWFX_TEXTURE_FETCH_PCF,
    IDC_RADIO_SHADOWFX_TEXTURE_FETCH_GATHER4,

    IDC_STATIC_SHADOWFX_TAP_TYPE,
    IDC_RADIO_SHADOWFX_TAP_TYPE_FIXED,
    IDC_RADIO_SHADOWFX_TAP_TYPE_POISSON,

    IDC_STATIC_SHADOWFX_NORMAL_OPTION,
    IDC_RADIO_SHADOWFX_NORMAL_OPTION_NONE,
    IDC_RADIO_SHADOWFX_NORMAL_OPTION_CALC,
    IDC_RADIO_SHADOWFX_NORMAL_OPTION_READ,

    IDC_STATIC_SHADOWMAP_SIZE,
    IDC_COMBOBOX_SHADOWMAP_SIZE,

    IDC_NUM_CONTROL_IDS
};

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
bool    CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext);
void    CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext);
void    CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext);
void    CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext);
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing, void* pUserContext);

HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext);
void    CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext);
void    CALLBACK OnD3D11DestroyDevice(void* pUserContext);
void    CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice, ID3D11DeviceContext* pd3dContext, double fTime, float fElapsedTime, void* pUserContext);

void             InitApplicationUI();
void             RenderText();

void             CreateShaders(ID3D11Device * pDevice);
void             InitializeCubeCamera(CFirstPersonCamera * pViewer, CFirstPersonCamera * pCubeCamera, S_CAMERA_DATA * pCubeCameraData);
void             InitializeCascadeCamera(CFirstPersonCamera * pViewer, CFirstPersonCamera * pCubeCamera, S_CAMERA_DATA * pCubeCameraData, float4x4 * ortho);

#include "ShadowFX_Sample_UI.inl"

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
    // Enable run-time memory check for debug builds.
#if defined(DEBUG) || defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    _CrtCheckMemory();

    // Disable gamma correction on this sample
    DXUTSetIsInGammaCorrectMode(false);

    DXUTSetCallbackDeviceChanging(ModifyDeviceSettings);
    DXUTSetCallbackMsgProc(MsgProc);
    DXUTSetCallbackKeyboard(OnKeyboard);
    DXUTSetCallbackFrameMove(OnFrameMove);

    DXUTSetCallbackD3D11DeviceCreated(OnD3D11CreateDevice);
    DXUTSetCallbackD3D11SwapChainResized(OnD3D11ResizedSwapChain);
    DXUTSetCallbackD3D11FrameRender(OnD3D11FrameRender);
    DXUTSetCallbackD3D11SwapChainReleasing(OnD3D11ReleasingSwapChain);
    DXUTSetCallbackD3D11DeviceDestroyed(OnD3D11DestroyDevice);

    InitApplicationUI();

    DXUTInit(true, true);                 // Use this line instead to try to create a hardware device
    DXUTSetCursorSettings(true, true);    // Show the cursor and clip it when in full screen
    DXUTCreateWindow(L"AMD ShadowFX v1.3");

    DXUTCreateDevice(D3D_FEATURE_LEVEL_11_0, true, g_Width, g_Height);
    DXUTMainLoop();

    return DXUTGetExitCode();
}

//--------------------------------------------------------------------------------------
// This callback function will be called once at the beginning of every frame. This is the
// best location for your application to handle updates to the scene, but is not
// intended to contain actual rendering calls, which should instead be placed in the
// OnFrameRender callback.
//--------------------------------------------------------------------------------------
void CALLBACK OnFrameMove(double fTime, float fElapsedTime, void* pUserContext)
{
    // Update the camera's position based on user input
    if (g_pCurrentCamera == &g_ViewerCamera ||
        g_pCurrentCamera == &g_LightCamera)
    {
        g_pCurrentCamera->FrameMove(fElapsedTime);
    }

    if ((g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_CUBE ||
         g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_UNION) 
         && g_bShowLightCamera) // if using light camera - need to update constant buffer values
        InitializeCubeCamera(&g_LightCamera, g_CubeCamera, g_LightData);

    if (g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_CASCADE)
        InitializeCascadeCamera(&g_ViewerCamera, g_CubeCamera, g_LightData, g_LightOrtho);
}


//--------------------------------------------------------------------------------------
// Before handling window messages, DXUT passes incoming windows
// messages to the application through this callback function. If the application sets
// *pbNoFurtherProcessing to TRUE, then DXUT will not process this message.
//--------------------------------------------------------------------------------------
LRESULT CALLBACK MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing,
    void* pUserContext)
{
    // Pass messages to dialog resource manager calls so GUI state is updated correctly
    *pbNoFurtherProcessing = g_DialogResourceManager.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;

    // Pass messages to settings dialog if its active
    if (g_SettingsDlg.IsActive())
    {
        g_SettingsDlg.MsgProc(hWnd, uMsg, wParam, lParam);
        return 0;
    }

    // Give the dialogs a chance to handle the message first
    *pbNoFurtherProcessing = g_HUD.m_GUI.MsgProc(hWnd, uMsg, wParam, lParam);
    if (*pbNoFurtherProcessing)
        return 0;

    // Pass all windows messages to camera so it can respond to user input
    g_pCurrentCamera->HandleMessages(hWnd, uMsg, wParam, lParam);

    return 0;
}

//--------------------------------------------------------------------------------------
// Create any D3D11 resources that aren't dependant on the back buffer
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11CreateDevice(ID3D11Device* pd3dDevice, const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    unsigned int extensionMask = 0;
    AGSReturnCode agsHr;
    AGSGPUInfo agsInfo;
    agsHr = agsInit(&g_agsContext, &agsInfo);
    agsHr = agsGetCrossfireGPUCount(g_agsContext, &g_agsGpuCount);
    agsHr = agsDriverExtensions_Init(g_agsContext, pd3dDevice, &extensionMask);

    if (g_agsGpuCount < 2 || 
        (extensionMask & AGS_EXTENSION_CROSSFIRE_API) == 0)
    {
        agsHr = agsDriverExtensions_DeInit(g_agsContext);
        agsHr = agsDeInit(g_agsContext);
        g_agsContext = NULL; 
    }
    else
    {
        agsHr = agsDriverExtensions_SetCrossfireMode(g_agsContext, AGS_CROSSFIRE_MODE_EXPLICIT_AFR);
    }

    ID3D11DeviceContext* pd3dContext = DXUTGetD3D11DeviceContext();
    V_RETURN(g_DialogResourceManager.OnD3D11CreateDevice(pd3dDevice, pd3dContext));
    V_RETURN(g_SettingsDlg.OnD3D11CreateDevice(pd3dDevice));
    g_pTxtHelper = new CDXUTTextHelper(pd3dDevice, pd3dContext, &g_DialogResourceManager, 15);
    // Hooks to various AMD helper classes
    g_MagnifyTool.OnCreateDevice(pd3dDevice);
    g_HUD.OnCreateDevice(pd3dDevice);

    g_ShadowsDesc.m_pDevice = pd3dDevice; // TODO: need to add an option to perform lazy shader compilation
    AMD::ShadowFX_Initialize(g_ShadowsDesc);

    CreateShaders(pd3dDevice);

    g_ShadowMap.Release();
    g_ShadowMap.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleW, (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleH, 1, 1, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, NULL, g_ShadowMapCfxTransfer);

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY==1)
    g_ShadowMapCopy.Release();
    g_ShadowMapCopy.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleW, (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleH, 1, 1, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, g_agsContext, g_ShadowMapCopyCfxTransfer);
#endif

    g_LightDepth.Release();
    g_LightDepth.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)512, (unsigned int)512, 1, 1, 1,
        DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_USAGE_DEFAULT, false, 0, NULL, g_agsContext, AGS_AFR_TRANSFER_DEFAULT);

    g_LightColor.Release();
    g_LightColor.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)512, (unsigned int)512, 1, 1, 1,
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, g_agsContext, AGS_AFR_TRANSFER_DEFAULT);

    // Setup constant buffer
    D3D11_BUFFER_DESC b1dDesc;
    b1dDesc.Usage = D3D11_USAGE_DYNAMIC;
    b1dDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    b1dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    b1dDesc.MiscFlags = 0;
    b1dDesc.ByteWidth = sizeof(S_CAMERA_DATA);
    V_RETURN(pd3dDevice->CreateBuffer(&b1dDesc, NULL, &g_pViewerCB));
    DXUT_SetDebugName(g_pViewerCB, "g_pViewerCB");

    b1dDesc.Usage = D3D11_USAGE_DYNAMIC;
    b1dDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    b1dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    b1dDesc.MiscFlags = 0;
    b1dDesc.ByteWidth = sizeof(S_MODEL_DATA);
    V_RETURN(pd3dDevice->CreateBuffer(&b1dDesc, NULL, &g_pModelCB));
    DXUT_SetDebugName(g_pModelCB, "g_pModelCB");

    b1dDesc.Usage = D3D11_USAGE_DYNAMIC;
    b1dDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    b1dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    b1dDesc.MiscFlags = 0;
    b1dDesc.ByteWidth = sizeof(g_LightData);
    V_RETURN(pd3dDevice->CreateBuffer(&b1dDesc, NULL, &g_pLightCB));
    DXUT_SetDebugName(g_pLightCB, "g_pLightCB");

    b1dDesc.Usage = D3D11_USAGE_DYNAMIC;
    b1dDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    b1dDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    b1dDesc.MiscFlags = 0;
    b1dDesc.ByteWidth = sizeof(S_UNIT_CUBE_TRANSFORM);
    V_RETURN(pd3dDevice->CreateBuffer(&b1dDesc, NULL, &g_pUnitCubeCB));
    DXUT_SetDebugName(g_pUnitCubeCB, "g_pUnitCubeCB");

    // Load the meshes

    V_RETURN(g_Tree.Create(pd3dDevice, "..\\media\\CoconutTree\\", "coconut.sdkmesh", true));
    V_RETURN(g_BoxPlane.Create(pd3dDevice, "..\\media\\Plane\\", "plane.sdkmesh", true));

    g_MeshModelMatrix[0] = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixTranslation(5, 0, 0);
    /*
    g_MeshModelMatrix[1] = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixTranslation(0, 0, 5);
    g_MeshModelMatrix[2] = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixTranslation(-5, 0, 0);
    g_MeshModelMatrix[3] = XMMatrixScaling(0.01f, 0.01f, 0.01f) * XMMatrixTranslation(10, 0, 5);
    */
    g_MeshModelMatrix[1] = XMMatrixIdentity();
    g_MeshModelMatrix[2] = XMMatrixScaling(1.0f, 10.0f, 0.001f) * XMMatrixTranslation(0, 10, -2.5);

    g_ViewerData.m_Color = float4(1.0f, 1.0f, 1.0f, 1.0f);

    for (int light = 0; light < CUBE_FACE_COUNT; light++)
    {
        g_LightData[light].m_Color = g_Color[light];
        g_LightData[light].m_BackBufferDim = float2(g_ShadowMapSize, g_ShadowMapSize);
        g_LightData[light].m_BackBufferDimRcp = float2(1.0f / g_ShadowMapSize, 1.0f / g_ShadowMapSize);
    }

    // Setup the camera's view parameters
    float4 vecEye(19.193f, 3.425f, 1.794f, 1.0f);
    float4 vecLightEye(0.471f, 2.855f, 2.096f, 1.0f);
    float4 vecAt(18.193f, 3.425f, 1.794f, 1.0f);
    float4 vecUp(0.0f, 1.0f, 0.0f);

    g_ViewerCamera.SetViewParams(vecEye, vecAt, vecUp);
    g_ViewerCamera.FrameMove(0.00001f);

    g_LightCamera.SetViewParams(vecLightEye, vecAt, vecUp);
    g_LightCamera.FrameMove(0.00001f);
    g_LightCamera.SetProjParams(AMD_PI / 2.0f, 1.0f, 0.01f, 25.0f);

    if (g_ShadowsExecution != AMD::SHADOWFX_EXECUTION_CASCADE)
        InitializeCubeCamera(&g_LightCamera, g_CubeCamera, g_LightData);
    else
        InitializeCascadeCamera(&g_ViewerCamera, g_CubeCamera, g_LightData, g_LightOrtho);

    // Create sampler states for point, linear and point_cmp
    CD3D11_DEFAULT defaultDesc;
    // Point
    CD3D11_SAMPLER_DESC ssDesc(defaultDesc);
    ssDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    ssDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    ssDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    ssDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    V_RETURN(pd3dDevice->CreateSamplerState(&ssDesc, &g_pLinearWrapSS));
    DXUT_SetDebugName(g_pLinearWrapSS, "g_pLinearWrapSS");

    CD3D11_BLEND_DESC bsDesc(defaultDesc);
    pd3dDevice->CreateBlendState((const D3D11_BLEND_DESC*)&bsDesc, &g_pOpaqueBS);

    bsDesc.RenderTarget[0].BlendEnable = true;
    bsDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MIN;
    bsDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MIN;
    bsDesc.RenderTarget[0].RenderTargetWriteMask = 0x1;
    pd3dDevice->CreateBlendState((const D3D11_BLEND_DESC*)&bsDesc, &g_pShadowMaskChannelBS[0]);
    bsDesc.RenderTarget[0].RenderTargetWriteMask = 0x2;
    pd3dDevice->CreateBlendState((const D3D11_BLEND_DESC*)&bsDesc, &g_pShadowMaskChannelBS[1]);
    bsDesc.RenderTarget[0].RenderTargetWriteMask = 0x4;
    pd3dDevice->CreateBlendState((const D3D11_BLEND_DESC*)&bsDesc, &g_pShadowMaskChannelBS[2]);
    bsDesc.RenderTarget[0].RenderTargetWriteMask = 0x8;
    pd3dDevice->CreateBlendState((const D3D11_BLEND_DESC*)&bsDesc, &g_pShadowMaskChannelBS[3]);

    CD3D11_RASTERIZER_DESC rsDesc(defaultDesc);
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    pd3dDevice->CreateRasterizerState(&rsDesc, &g_pNoCullingSolidRS);
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    pd3dDevice->CreateRasterizerState(&rsDesc, &g_pBackCullingSolidRS);
    rsDesc.CullMode = D3D11_CULL_FRONT;
    rsDesc.FillMode = D3D11_FILL_SOLID;
    pd3dDevice->CreateRasterizerState(&rsDesc, &g_pFrontCullingSolidRS);
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.FillMode = D3D11_FILL_WIREFRAME;
    pd3dDevice->CreateRasterizerState(&rsDesc, &g_pNoCullingWireframeRS);

    // regular DSS
    CD3D11_DEPTH_STENCIL_DESC dssDesc(defaultDesc);
    pd3dDevice->CreateDepthStencilState(&dssDesc, &g_pDepthTestLessDSS);

    dssDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
    pd3dDevice->CreateDepthStencilState(&dssDesc, &g_pDepthTestLessEqualDSS);

    // DSS - no depth test and no depth write ; stencil test and clear stencil afterwards
    dssDesc = CD3D11_DEPTH_STENCIL_DESC(defaultDesc);
    dssDesc.DepthEnable = false;
    dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dssDesc.StencilWriteMask = 0xff;
    dssDesc.StencilEnable = true;
    dssDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    dssDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;
    dssDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_ZERO;
    dssDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_ZERO;
    pd3dDevice->CreateDepthStencilState(&dssDesc, &g_pStencilTestAndClearDSS);

    // DSS - depth test enabled but no depth write ; stencil write but no test
    dssDesc = CD3D11_DEPTH_STENCIL_DESC(defaultDesc);
    dssDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    dssDesc.StencilEnable = true;
    dssDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dssDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dssDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    dssDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    dssDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    dssDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    dssDesc.StencilWriteMask = 0xff;
    pd3dDevice->CreateDepthStencilState(&dssDesc, &g_pDepthTestMarkStencilDSS);

    TIMER_Init(pd3dDevice);

    return S_OK;
}

//--------------------------------------------------------------------------------------
// Resize
//--------------------------------------------------------------------------------------
HRESULT CALLBACK OnD3D11ResizedSwapChain(ID3D11Device* pd3dDevice, IDXGISwapChain* pSwapChain,
    const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc, void* pUserContext)
{
    HRESULT hr;

    g_Height = pBackBufferSurfaceDesc->Height;
    g_Width = pBackBufferSurfaceDesc->Width;

    g_ShadowMask.Release();
    g_ShadowMask.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)pBackBufferSurfaceDesc->Width,
        (unsigned int)pBackBufferSurfaceDesc->Height, 1, 1, 1,
        DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_R16G16B16A16_UNORM,
        DXGI_FORMAT_R16G16B16A16_UNORM, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, g_agsContext, AGS_AFR_TRANSFER_DEFAULT);

    g_AppDepth.Release();
    g_AppDepth.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)pBackBufferSurfaceDesc->Width,
        (unsigned int)pBackBufferSurfaceDesc->Height, 1, 1, 1,
        DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_R24_UNORM_X8_TYPELESS,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D24_UNORM_S8_UINT,
        D3D11_USAGE_DEFAULT, false, 0, NULL, g_agsContext, AGS_AFR_TRANSFER_DEFAULT);

    g_AppNormal.Release();
    g_AppNormal.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)pBackBufferSurfaceDesc->Width,
        (unsigned int)pBackBufferSurfaceDesc->Height, 1, 1, 1,
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UNORM,
        DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, g_agsContext, AGS_AFR_TRANSFER_DEFAULT);

    V_RETURN(g_DialogResourceManager.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));
    V_RETURN(g_SettingsDlg.OnD3D11ResizedSwapChain(pd3dDevice, pBackBufferSurfaceDesc));

    // Setup the camera's projection parameters
    FLOAT fAspectRatio = pBackBufferSurfaceDesc->Width / (FLOAT)pBackBufferSurfaceDesc->Height;
    g_ViewerCamera.SetProjParams(XM_PI / 4, fAspectRatio, 1.0f, 200.0f);
    g_ViewerData.m_BackBufferDim = float2((float)g_Width, (float)g_Height);
    g_ViewerData.m_BackBufferDimRcp = float2(1.0f / (float)g_Width, 1.0f / (float)g_Height);

    // Set the location and size of the AMD standard HUD
    g_HUD.m_GUI.SetLocation(pBackBufferSurfaceDesc->Width - AMD::HUD::iDialogWidth, 0);
    g_HUD.m_GUI.SetSize(AMD::HUD::iDialogWidth, pBackBufferSurfaceDesc->Height);
    // Magnify tool will capture from the color buffer
    g_MagnifyTool.OnResizedSwapChain(pd3dDevice, pSwapChain, pBackBufferSurfaceDesc, pUserContext, pBackBufferSurfaceDesc->Width - AMD::HUD::iDialogWidth, 0);
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    ID3D11Resource* pTempRTResource;
    DXUTGetD3D11RenderTargetView()->GetResource(&pTempRTResource);
    DXUTGetD3D11RenderTargetView()->GetDesc(&rtvDesc);
    g_MagnifyTool.SetSourceResources(pTempRTResource, rtvDesc.Format, g_Width, g_Height, DXUTGetDXGIBackBufferSurfaceDesc()->SampleDesc.Count);
    g_MagnifyTool.SetPixelRegion(128);
    g_MagnifyTool.SetScale(5);
    SAFE_RELEASE(pTempRTResource);

    // AMD HUD hook
    g_HUD.OnResizedSwapChain(pBackBufferSurfaceDesc);

    return S_OK;
}

void InitializeCubeCamera(CFirstPersonCamera * pViewer, CFirstPersonCamera * pCubeCamera, S_CAMERA_DATA * pCubeCameraData)
{
    if (pViewer == NULL || pCubeCamera == NULL || pCubeCameraData == NULL)
        return;

    S_CAMERA_DATA camera;

    float4 eye = pViewer->GetEyePt();
    float4 dir_x(1.0f, 0.0f, 0.0f);
    float4 dir_y(0.0f, 1.0f, 0.0f);
    float4 dir_z(0.0f, 0.0f, 1.0f);

    float aspect = 1.0f;
    float znear = 0.01f;
    float zfar = 25.0f;
    float fov = AMD_PI / 2.0f;

    float4 cube_look_at[6];
    float4 cube_up[6];

    cube_look_at[0] = eye + dir_x*(1.0f);
    cube_up[0] = dir_y*(1.0f);
    cube_look_at[1] = eye + dir_x*(-1.0f);
    cube_up[1] = dir_y*(1.0f);
    cube_look_at[2] = eye + dir_y*(1.0f);
    cube_up[2] = dir_z*(-1.0f);
    cube_look_at[3] = eye + dir_y*(-1.0f);
    cube_up[3] = dir_z*(1.0f);
    cube_look_at[4] = eye + dir_z*(1.0f);
    cube_up[4] = dir_y*(1.0f);
    cube_look_at[5] = eye + dir_z*(-1.0f);
    cube_up[5] = dir_y*(1.0f);

    for (int i = 0; i < 6; i++)
    {
        pCubeCamera[i].SetProjParams(fov, aspect, znear, zfar);

        pCubeCamera[i].SetViewParams(eye, cube_look_at[i], cube_up[i]);

        pCubeCamera[i].FrameMove(0.000001f);
    }
}

void InitializeCascadeCamera(CFirstPersonCamera * pViewer, CFirstPersonCamera * pCubeCamera, S_CAMERA_DATA * pCubeCameraData, float4x4 * ortho)
{
    if (pViewer == NULL || pCubeCamera == NULL || pCubeCameraData == NULL)
        return;

    S_CAMERA_DATA camera;

    float4 eye = pViewer->GetEyePt();
    float4 direction = pViewer->GetWorldAhead();
    float4 right = pViewer->GetWorldRight();
    float4 up = pViewer->GetWorldUp();
    float4 light(1.0f, 1.0f, 1.0f, 0.0f);

    float aspect = 1.0f;
    float znear = 0.01f;
    float zfar = 25.0f;
    float fov = AMD_PI / 2.0f;

    for (int i = 0; i < 6; i++)
    {
        pCubeCamera[i].SetProjParams(fov, aspect, znear, zfar);
        pCubeCamera[i].SetViewParams(eye + light * 2, eye, up);

        pCubeCamera[i].FrameMove(0.000001f);

        ortho[i] = XMMatrixOrthographicOffCenterLH(-5.0f*(i + 0.5f), 5.0f*(i + 0.5f), -5.0f*(i + 0.5f), 5.0f*(i + 0.5f), -5.0f*(i + 1.5f), 5.0f*(i + 1.5f)); // pLightCamera->GetProjMatrix(); // XMMatrixOrthographicOffCenterLH( -10.0, 10.0, -10.0, 10.0, -30.0, 30.0 );
    }
}

void SetCameraConstantBufferData(ID3D11DeviceContext* pd3dContext,
    ID3D11Buffer*                                       pd3dCB,
    S_CAMERA_DATA*                                      pCameraData,
    CFirstPersonCamera*                                 pCamera,
    float4x4*                                           pProjection,
    unsigned int                                        nCount)
{
    D3D11_MAPPED_SUBRESOURCE MappedResource;

    if (pd3dContext == NULL) { OutputDebugString(AMD_FUNCTION_WIDE_NAME L" received a NULL D3D11 Context pointer \n");         return; }
    if (pd3dCB == NULL) { OutputDebugString(AMD_FUNCTION_WIDE_NAME L" received a NULL D3D11 Constant Buffer pointer \n"); return; }

    for (unsigned int i = 0; i < nCount; i++)
    {
        CFirstPersonCamera & camera = pCamera[i];
        S_CAMERA_DATA & cameraData = pCameraData[i];

        XMMATRIX  view = camera.GetViewMatrix();
        XMMATRIX  proj = pProjection != NULL ? pProjection[i] : camera.GetProjMatrix();
        XMMATRIX  viewproj = view * proj;
        XMMATRIX  view_inv = XMMatrixInverse(&XMMatrixDeterminant(view), view);
        XMMATRIX  proj_inv = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
        XMMATRIX  viewproj_inv = XMMatrixInverse(&XMMatrixDeterminant(viewproj), viewproj);

        cameraData.m_View = XMMatrixTranspose(view);
        cameraData.m_Projection = XMMatrixTranspose(proj);
        cameraData.m_ViewInv = XMMatrixTranspose(view_inv);
        cameraData.m_ProjectionInv = XMMatrixTranspose(proj_inv);
        cameraData.m_ViewProjection = XMMatrixTranspose(viewproj);
        cameraData.m_ViewProjectionInv = XMMatrixTranspose(viewproj_inv);

        cameraData.m_Position = camera.GetEyePt();
        cameraData.m_Direction = XMVector3Normalize(camera.GetLookAtPt() - camera.GetEyePt());
        cameraData.m_Up = camera.GetWorldUp();
        cameraData.m_Fov = camera.GetFOV();
        cameraData.m_Aspect = camera.GetAspect();
        cameraData.m_zNear = camera.GetNearClip();
        cameraData.m_zFar = camera.GetFarClip();
    }

    pd3dContext->Map(pd3dCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (MappedResource.pData)
        memcpy(MappedResource.pData, pCameraData, sizeof(S_CAMERA_DATA) * nCount);
    pd3dContext->Unmap(pd3dCB, 0);
}

void SetModelMatrices(ID3D11DeviceContext*  pd3dContext,
    ID3D11Buffer*                              pd3dCB,
    const XMMATRIX&                            world,
    const XMMATRIX&                            viewProj)
{
    D3D11_MAPPED_SUBRESOURCE MappedResource;

    S_MODEL_DATA modelData;
    modelData.m_World = XMMatrixTranspose(world);
    modelData.m_WorldViewProjection = XMMatrixTranspose(world * viewProj);
    modelData.m_Ambient = float4(0.1f, 0.1f, 0.1f, 1.0f);
    modelData.m_Diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f);

    pd3dContext->Map(pd3dCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (MappedResource.pData)
        memcpy(MappedResource.pData, &modelData, sizeof(modelData));
    pd3dContext->Unmap(pd3dCB, 0);
}

//--------------------------------------------------------------------------------------
// Render the scene (either for the main scene or the shadow map scene)
//--------------------------------------------------------------------------------------
void RenderScene(ID3D11DeviceContext*  pd3dContext,
    AMD::Mesh**                          pMesh,
    XMMATRIX*                            pModelMatrix,
    unsigned int                         nMeshCount,

    D3D11_VIEWPORT*                      pVP,           // ViewPort array
    unsigned int                         nVPCount,      // Viewport count

    D3D11_RECT*                          pSR,           // Scissor Rects array
    unsigned int                         nSRCount,      // Scissor rect count

    ID3D11RasterizerState*               pRS,           // Raster State

    ID3D11BlendState*                    pBS,           // Blend State
    const float*                         pFactorBS,     // Blend state factor

    ID3D11DepthStencilState*             pDSS,          // Depth Stencil State
    unsigned int                         dssRef,        // Depth stencil state reference value

    ID3D11InputLayout*                   pIL,           // Input Layout
    ID3D11VertexShader*                  pVS,           // Vertex Shader
    ID3D11HullShader*                    pHS,           // Hull Shader
    ID3D11DomainShader*                  pDS,           // Domain Shader
    ID3D11GeometryShader*                pGS,           // Geometry SHader
    ID3D11PixelShader*                   pPS,           // Pixel Shader

    ID3D11Buffer*                        pModelCB,
    unsigned int                         nModelCBSlot,

    ID3D11Buffer**                       ppCB,          // Constant Buffer array
    unsigned int                         nCBStart,      // First slot to attach constant buffer array
    unsigned int                         nCBCount,      // Number of constant buffers in the array

    ID3D11SamplerState**                 ppSS,          // Sampler State array
    unsigned int                         nSSStart,      // First slot to attach sampler state array
    unsigned int                         nSSCount,      // Number of sampler states in the array

    ID3D11ShaderResourceView**           ppSRV,         // Shader Resource View array
    unsigned int                         nSRVStart,     // First slot to attach sr views array
    unsigned int                         nSRVCount,     // Number of sr views in the array

    ID3D11RenderTargetView**             ppRTV,         // Render Target View array
    unsigned int                         nRTVCount,     // Number of rt views in the array
    ID3D11DepthStencilView*              pDSV,          // Depth Stencil View

    S_CAMERA_DATA*                       pViewerData,
    CFirstPersonCamera*                  pCamera = NULL)
{
    ID3D11RenderTargetView *   const pNullRTV[8] = { 0 };
    ID3D11ShaderResourceView * const pNullSRV[128] = { 0 };

    // Unbind anything that could be still bound on input or output
    // If this doesn't happen, DX Runtime will spam with warnings
    pd3dContext->OMSetRenderTargets(AMD_ARRAY_SIZE(pNullRTV), pNullRTV, NULL);
    pd3dContext->CSSetShaderResources(0, AMD_ARRAY_SIZE(pNullSRV), pNullSRV);
    pd3dContext->VSSetShaderResources(0, AMD_ARRAY_SIZE(pNullSRV), pNullSRV);
    pd3dContext->HSSetShaderResources(0, AMD_ARRAY_SIZE(pNullSRV), pNullSRV);
    pd3dContext->DSSetShaderResources(0, AMD_ARRAY_SIZE(pNullSRV), pNullSRV);
    pd3dContext->GSSetShaderResources(0, AMD_ARRAY_SIZE(pNullSRV), pNullSRV);
    pd3dContext->PSSetShaderResources(0, AMD_ARRAY_SIZE(pNullSRV), pNullSRV);

    pd3dContext->IASetInputLayout(pIL);

    pd3dContext->VSSetShader(pVS, NULL, 0);
    pd3dContext->HSSetShader(pHS, NULL, 0);
    pd3dContext->DSSetShader(pDS, NULL, 0);
    pd3dContext->GSSetShader(pGS, NULL, 0);
    pd3dContext->PSSetShader(pPS, NULL, 0);

    if (nSSCount)
    {
        pd3dContext->VSSetSamplers(nSSStart, nSSCount, ppSS);
        pd3dContext->HSSetSamplers(nSSStart, nSSCount, ppSS);
        pd3dContext->DSSetSamplers(nSSStart, nSSCount, ppSS);
        pd3dContext->GSSetSamplers(nSSStart, nSSCount, ppSS);
        pd3dContext->PSSetSamplers(nSSStart, nSSCount, ppSS);
    }

    if (nSRVCount)
    {
        pd3dContext->VSSetShaderResources(nSRVStart, nSRVCount, ppSRV);
        pd3dContext->HSSetShaderResources(nSRVStart, nSRVCount, ppSRV);
        pd3dContext->DSSetShaderResources(nSRVStart, nSRVCount, ppSRV);
        pd3dContext->GSSetShaderResources(nSRVStart, nSRVCount, ppSRV);
        pd3dContext->PSSetShaderResources(nSRVStart, nSRVCount, ppSRV);
    }

    if (nCBCount)
    {
        pd3dContext->VSSetConstantBuffers(nCBStart, nCBCount, ppCB);
        pd3dContext->HSSetConstantBuffers(nCBStart, nCBCount, ppCB);
        pd3dContext->DSSetConstantBuffers(nCBStart, nCBCount, ppCB);
        pd3dContext->GSSetConstantBuffers(nCBStart, nCBCount, ppCB);
        pd3dContext->PSSetConstantBuffers(nCBStart, nCBCount, ppCB);
    }

    pd3dContext->OMSetRenderTargets(nRTVCount, ppRTV, pDSV);
    pd3dContext->OMSetBlendState(pBS, pFactorBS, 0xf);
    pd3dContext->OMSetDepthStencilState(pDSS, dssRef);
    pd3dContext->RSSetState(pRS);
    pd3dContext->RSSetScissorRects(nSRCount, pSR);
    pd3dContext->RSSetViewports(nVPCount, pVP);

    // Setup the view matrices
    XMMATRIX view = pCamera != NULL ? pCamera->GetViewMatrix() : XMMatrixTranspose(pViewerData->m_View);
    XMMATRIX proj = pCamera != NULL ? pCamera->GetProjMatrix() : XMMatrixTranspose(pViewerData->m_Projection);
    XMMATRIX viewproj = view * proj;

    for (unsigned int mesh = 0; mesh < nMeshCount; mesh++)
    {
        SetModelMatrices(pd3dContext, pModelCB,
            pModelMatrix[mesh], viewproj);

        pMesh[mesh]->Render(pd3dContext);
    }
}


//--------------------------------------------------------------------------------------
// Render
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11FrameRender(ID3D11Device* pd3dDevice,
    ID3D11DeviceContext*                          pd3dContext,
    double                                        fTime,
    float                                         fElapsedTime,
    void*                                         pUserContext)
{
    D3D11_MAPPED_SUBRESOURCE MappedResource;

    D3D11_RECT*                pNullSR = NULL;
    ID3D11HullShader*          pNullHS = NULL;
    ID3D11DomainShader*        pNullDS = NULL;
    ID3D11GeometryShader*      pNullGS = NULL;
    ID3D11PixelShader*         pNullPS = NULL;
    ID3D11ShaderResourceView*  pNullSRV = NULL;
    ID3D11RenderTargetView*    pNullRTV = NULL;
    ID3D11DepthStencilView*    pNullDSV = NULL;
    ID3D11SamplerState*        pNullSS = NULL;
    CFirstPersonCamera*        pNullCamera = NULL;

    ID3D11RenderTargetView*    pOriginalRTV = NULL;
    ID3D11DepthStencilView*    pOriginalDSV = NULL;

    const  int                 showLightArea = 512;
    static int                 nCount = 0;
    static float               fTimeShadowMap = 0.0f;
    static float               fTimeShadowMapFiltering = 0.0f;
    static float               fTimeDepthPrepass = 0.0f;
    static float               fSceneRendering = 0.0f;
    static float               fShadowMapMasking = 0.0f;
    static bool                bCapture = false;

    static int                 shadowMapFrameDelay = 0;

    // if running on an MGPU PC, update shadow map once in two frames 
    static int                 maxShadowMapFrameDelay = g_agsGpuCount > 1 ? 2 : 1; 

    int shadowTextureType = g_HUD.m_GUI.GetRadioButton(IDC_RADIO_SHADOW_MAP_T2D)->GetChecked() ? AMD::SHADOWFX_TEXTURE_2D : AMD::SHADOWFX_TEXTURE_2D_ARRAY;

    TIMER_Reset();

    if (g_SettingsDlg.IsActive()) // If the settings dialog is being shown, then render it instead of rendering the app's scene
    {
        g_SettingsDlg.OnRender(fElapsedTime);
        return;
    }

    pd3dContext->OMGetRenderTargets(1, &pOriginalRTV, &pOriginalDSV); // Store the original render target and depth buffer so we can reset it at the end of the frame

    pd3dContext->ClearRenderTargetView(g_ShadowMask._rtv, black.f);
    pd3dContext->ClearRenderTargetView(pOriginalRTV, light_blue.f);
    pd3dContext->ClearRenderTargetView(g_LightColor._rtv, deep_blue.f);
    pd3dContext->ClearRenderTargetView(g_AppNormal._rtv, grey.f);
    pd3dContext->ClearDepthStencilView(pOriginalDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
    pd3dContext->ClearDepthStencilView(g_AppDepth._dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    pd3dContext->ClearDepthStencilView(g_LightDepth._dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    {
        ID3D11ShaderResourceView * pSRV[] = { NULL, g_ShadowMask._srv, NULL };
        ID3D11Buffer             * pCB[] = { g_pModelCB, g_pViewerCB, g_pLightCB };
        ID3D11SamplerState       * pSS[] = { g_pLinearWrapSS };

        SetCameraConstantBufferData(pd3dContext, g_pViewerCB, &g_ViewerData, &g_ViewerCamera, NULL, 1);

        TIMER_Begin(0, L"Depth Prepass Rendering");
        {
            ID3D11RenderTargetView* pRTV[] = { g_AppNormal._rtv };

            RenderScene(pd3dContext,
                g_MeshArray, g_MeshModelMatrix, AMD_ARRAY_SIZE(g_MeshArray),
                &CD3D11_VIEWPORT(0.0f, 0.0f, (float)g_Width, (float)g_Height), 1,
                pNullSR, 0,
                g_pBackCullingSolidRS, g_pOpaqueBS, white.f,
                g_pDepthTestLessDSS, 0, g_pSceneIL,
                g_pSceneVS, pNullHS, pNullDS, pNullGS, g_pDepthAndNormalPassScenePS,
                g_pModelCB, 0, pCB, 0, AMD_ARRAY_SIZE(pCB),
                pSS, 0, AMD_ARRAY_SIZE(pSS), &pNullSRV, 1, 0,
                pRTV, AMD_ARRAY_SIZE(pRTV), g_AppDepth._dsv,
                &g_ViewerData, pNullCamera);

        }
        TIMER_End();

        if (g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_CASCADE)
            SetCameraConstantBufferData(pd3dContext, g_pLightCB, g_LightData, g_CubeCamera, g_LightOrtho, CUBE_FACE_COUNT);
        else
            SetCameraConstantBufferData(pd3dContext, g_pLightCB, g_LightData, g_CubeCamera, NULL, CUBE_FACE_COUNT);

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 0) // if experimental code is diables, begin access the original shadow map
        agsDriverExtensions_NotifyResourceBeginAllAccess(g_agsContext, g_ShadowMap._t2d);
#endif

        if (shadowMapFrameDelay % maxShadowMapFrameDelay == 0)
        {
            if (shadowTextureType == AMD::SHADOWFX_TEXTURE_2D)
            {
                pd3dContext->ClearDepthStencilView(g_ShadowMap._dsv, D3D11_CLEAR_DEPTH, 1.0, 0); // there is only 1 shadow map, so clear it every frame

                TIMER_Begin(0, L"Shadow Map Rendering"); // Render shadow map into separate texture array slices
                {
                    for (int light = 0; light < CUBE_FACE_COUNT; light++)
                    {
                        int lightIndexX = light % g_ShadowMapAtlasScaleW;
                        int lightIndexY = light / g_ShadowMapAtlasScaleW;

                        RenderScene(pd3dContext,
                            g_MeshArray, g_MeshModelMatrix, AMD_ARRAY_SIZE(g_MeshArray),
                            &CD3D11_VIEWPORT(g_ShadowMapSize*lightIndexX, g_ShadowMapSize*lightIndexY, g_ShadowMapSize, g_ShadowMapSize), 1,
                            pNullSR, 0,
                            g_pFrontCullingSolidRS, g_pOpaqueBS, white.f,
                            g_pDepthTestLessDSS, 0, g_pSceneIL,
                            g_pSceneVS, pNullHS, pNullDS, pNullGS, g_pDepthPassScenePS,
                            g_pModelCB, 0, pCB, 0, AMD_ARRAY_SIZE(pCB),
                            pSS, 0, AMD_ARRAY_SIZE(pSS), &pNullSRV, 1, 0,
                            &pNullRTV, 0, g_ShadowMap._dsv,
                            &g_LightData[light], pNullCamera);

                    }
                }
                TIMER_End();
            }

            if (shadowTextureType == AMD::SHADOWFX_TEXTURE_2D_ARRAY)
            {
                TIMER_Begin(0, L"Shadow Map Rendering"); // Render shadow map into separate texture array slices
                {
                    for (int light = 0; light < CUBE_FACE_COUNT; light++)
                    {
                        pd3dContext->ClearDepthStencilView(g_ShadowMap._dsv_cube[light], D3D11_CLEAR_DEPTH, 1.0, 0); // there is only 1 shadow map, so clear it every frame

                        RenderScene(pd3dContext,
                            g_MeshArray, g_MeshModelMatrix, AMD_ARRAY_SIZE(g_MeshArray),
                            &CD3D11_VIEWPORT(0.0f, 0.0f, g_ShadowMapSize, g_ShadowMapSize), 1,
                            pNullSR, 0,
                            g_pFrontCullingSolidRS, g_pOpaqueBS, white.f,
                            g_pDepthTestLessDSS, 0, g_pSceneIL,
                            g_pSceneVS, pNullHS, pNullDS, pNullGS, g_pDepthPassScenePS,
                            g_pModelCB, 0, pCB, 0, AMD_ARRAY_SIZE(pCB),
                            pSS, 0, AMD_ARRAY_SIZE(pSS), &pNullSRV, 1, 0,
                            &pNullRTV, 0, g_ShadowMap._dsv_cube[light],
                            &g_LightData[light], pNullCamera);

                    }
                }
                TIMER_End();
            }

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1) // if experimental is enabled, then only shadow map Copy needs to notify access 
            agsDriverExtensions_NotifyResourceBeginAllAccess(g_agsContext, g_ShadowMapCopy._t2d);
            pd3dContext->CopyResource(g_ShadowMapCopy._t2d, g_ShadowMap._t2d); // update the shadow map copy from current frame 
            agsDriverExtensions_NotifyResourceEndWrites(g_agsContext, g_ShadowMapCopy._t2d, NULL, 0, 0); // intiate transfer
# if (EXPERIMENTAL_DELAY_END_ALL_ACCESS == 0)
            agsDriverExtensions_NotifyResourceEndAllAccess(g_agsContext, g_ShadowMapCopy._t2d); // we won't be accessing this resource again in the frame!
# endif

#elif (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 0)
            agsDriverExtensions_NotifyResourceEndWrites(g_agsContext, g_ShadowMap._t2d, NULL, 0, 0); // if experimental is disabled - we can only tell the driver that shadow map is done updating
#endif
        }
        else
        {
#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1) // if experimental is enabled, then only shadow map Copy needs to notify access 
            agsDriverExtensions_NotifyResourceBeginAllAccess(g_agsContext, g_ShadowMapCopy._t2d); // 
            pd3dContext->CopyResource(g_ShadowMap._t2d, g_ShadowMapCopy._t2d);
# if (EXPERIMENTAL_DELAY_END_ALL_ACCESS == 0)
            agsDriverExtensions_NotifyResourceEndAllAccess(g_agsContext, g_ShadowMapCopy._t2d); // we won't be accessing this resource again in the frame!
# endif
#endif
        }

        shadowMapFrameDelay++;

        TIMER_Begin(0, L"Shadow Map Masking");
        {
            for (int light = 0; light < CUBE_FACE_COUNT; light++)
            {
                pd3dContext->Map(g_pUnitCubeCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
                S_UNIT_CUBE_TRANSFORM* pUnitCubeCB = (S_UNIT_CUBE_TRANSFORM*)MappedResource.pData;
                {
                    pUnitCubeCB->m_Transform = g_ViewerData.m_ViewProjection  * g_LightData[light].m_ViewProjectionInv;
                    pUnitCubeCB->m_Inverse = g_LightData[light].m_ViewProjectionInv;
                    pUnitCubeCB->m_Forward = g_ViewerData.m_ViewProjection;
                    pUnitCubeCB->m_Color = white;
                }
                pd3dContext->Unmap(g_pUnitCubeCB, 0);

                AMD::RenderUnitCube(pd3dContext,
                    CD3D11_VIEWPORT(0.0f, 0.0f, (float)g_Width, (float)g_Height),
                    pNullSR, 0,
                    g_pNoCullingSolidRS,
                    g_pOpaqueBS, white.f,
                    g_pDepthTestMarkStencilDSS, 1,
                    g_pUnitCubeVS, pNullHS, pNullDS, pNullGS, pNullPS,
                    &g_pUnitCubeCB, 0, 1,
                    &pNullSS, 0, 0,
                    &pNullSRV, 0, 0,
                    &pNullRTV, 0, g_AppDepth._dsv);
            }
        }
        TIMER_End();

        g_ShadowsDesc.m_Execution = g_ShadowsExecution;

        TIMER_Begin(0, L"Shadow Map Filtering");
        {
            g_ShadowsDesc.m_NormalOption = AMD::SHADOWFX_NORMAL_OPTION_NONE;
            if (g_HUD.m_GUI.GetRadioButton(IDC_RADIO_SHADOWFX_NORMAL_OPTION_CALC)->GetChecked())
                g_ShadowsDesc.m_NormalOption = AMD::SHADOWFX_NORMAL_OPTION_CALC_FROM_DEPTH;
            if (g_HUD.m_GUI.GetRadioButton(IDC_RADIO_SHADOWFX_NORMAL_OPTION_READ)->GetChecked())
                g_ShadowsDesc.m_NormalOption = AMD::SHADOWFX_NORMAL_OPTION_READ_FROM_SRV;

            if (g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_FILTERING)->GetChecked())
            {
                g_ShadowsDesc.m_TapType = g_HUD.m_GUI.GetRadioButton(IDC_RADIO_SHADOWFX_TAP_TYPE_FIXED)->GetChecked() ? AMD::SHADOWFX_TAP_TYPE_FIXED : AMD::SHADOWFX_TAP_TYPE_POISSON;
                g_ShadowsDesc.m_Filtering = g_HUD.m_GUI.GetRadioButton(IDC_RADIO_SHADOWFX_FILTERING_UNIFORM)->GetChecked() ? AMD::SHADOWFX_FILTERING_UNIFORM : AMD::SHADOWFX_FILTERING_CONTACT;
                g_ShadowsDesc.m_TextureFetch = g_HUD.m_GUI.GetRadioButton(IDC_RADIO_SHADOWFX_TEXTURE_FETCH_GATHER4)->GetChecked() ? AMD::SHADOWFX_TEXTURE_FETCH_GATHER4 : AMD::SHADOWFX_TEXTURE_FETCH_PCF;

                g_ShadowsDesc.m_TextureType = (AMD::SHADOWFX_TEXTURE_TYPE) shadowTextureType;

                switch (g_HUD.m_GUI.GetComboBox(IDC_COMBOBOX_SHADOWFX_FILTER_SIZE)->GetSelectedIndex())
                {
                case 0: g_ShadowsDesc.m_FilterSize = AMD::SHADOWFX_FILTER_SIZE_7;  break;
                case 1: g_ShadowsDesc.m_FilterSize = AMD::SHADOWFX_FILTER_SIZE_9;  break;
                case 2: g_ShadowsDesc.m_FilterSize = AMD::SHADOWFX_FILTER_SIZE_11; break;
                case 3: g_ShadowsDesc.m_FilterSize = AMD::SHADOWFX_FILTER_SIZE_13; break;
                case 4: g_ShadowsDesc.m_FilterSize = AMD::SHADOWFX_FILTER_SIZE_15; break;
                }
            }
            else
            {
                g_ShadowsDesc.m_Filtering = AMD::SHADOWFX_FILTERING_DEBUG_POINT;
            }

            float2 backbufferDim((float)g_Width, (float)g_Height);
            float2 shadowAtlasRegionDim(g_ShadowMapSize, g_ShadowMapSize);

            g_ShadowsDesc.m_ActiveLightCount = CUBE_FACE_COUNT;

            memcpy(&g_ShadowsDesc.m_Viewer.m_View, &g_ViewerData.m_View, sizeof(g_ShadowsDesc.m_Viewer.m_View));
            memcpy(&g_ShadowsDesc.m_Viewer.m_Projection, &g_ViewerData.m_Projection, sizeof(g_ShadowsDesc.m_Viewer.m_Projection));
            memcpy(&g_ShadowsDesc.m_Viewer.m_ViewProjection, &g_ViewerData.m_ViewProjection, sizeof(g_ShadowsDesc.m_Viewer.m_ViewProjection));
            memcpy(&g_ShadowsDesc.m_Viewer.m_View_Inv, &g_ViewerData.m_ViewInv, sizeof(g_ShadowsDesc.m_Viewer.m_View_Inv));
            memcpy(&g_ShadowsDesc.m_Viewer.m_Projection_Inv, &g_ViewerData.m_ProjectionInv, sizeof(g_ShadowsDesc.m_Viewer.m_Projection_Inv));
            memcpy(&g_ShadowsDesc.m_Viewer.m_ViewProjection_Inv, &g_ViewerData.m_ViewProjectionInv, sizeof(g_ShadowsDesc.m_Viewer.m_ViewProjection_Inv));
            memcpy(&g_ShadowsDesc.m_Viewer.m_Position, &g_ViewerData.m_Position, sizeof(g_ShadowsDesc.m_Viewer.m_Position));
            memcpy(&g_ShadowsDesc.m_Viewer.m_Direction, &g_ViewerData.m_Direction, sizeof(g_ShadowsDesc.m_Viewer.m_Direction));
            memcpy(&g_ShadowsDesc.m_Viewer.m_Up, &g_ViewerData.m_Up, sizeof(g_ShadowsDesc.m_Viewer.m_Up));
            memcpy(&g_ShadowsDesc.m_Viewer.m_Color, &g_ViewerData.m_Color, sizeof(g_ShadowsDesc.m_Viewer.m_Color));
            memcpy(&g_ShadowsDesc.m_DepthSize, &backbufferDim, sizeof(g_ShadowsDesc.m_DepthSize));

            for (int i = 0; i < CUBE_FACE_COUNT; i++)
            {
                int lightIndexX = i % g_ShadowMapAtlasScaleW;
                int lightIndexY = i / g_ShadowMapAtlasScaleW;

                float4 shadowRegion(0.0f, 0.0f, 0.0f, 0.0f);

                if (shadowTextureType == AMD::SHADOWFX_TEXTURE_2D)
                {
                    shadowRegion.x = 1.0f * lightIndexX / g_ShadowMapAtlasScaleW;
                    shadowRegion.z = 1.0f * (lightIndexX + 1.0f) / g_ShadowMapAtlasScaleW;
                    shadowRegion.y = 1.0f * lightIndexY / g_ShadowMapAtlasScaleH;
                    shadowRegion.w = 1.0f * (lightIndexY + 1.0f) / g_ShadowMapAtlasScaleH;

                    g_ShadowsDesc.m_ArraySlice[i] = 0;
                }

                if (shadowTextureType == AMD::SHADOWFX_TEXTURE_2D_ARRAY)
                {
                    shadowRegion.x = 0.0f;
                    shadowRegion.z = 1.0f;
                    shadowRegion.y = 0.0f;
                    shadowRegion.w = 1.0f;

                    g_ShadowsDesc.m_ArraySlice[i] = i;
                }

                memcpy(&g_ShadowsDesc.m_ShadowSize[i], &shadowAtlasRegionDim, sizeof(g_ShadowsDesc.m_ShadowSize[i]));
                memcpy(&g_ShadowsDesc.m_ShadowRegion[i], &shadowRegion, sizeof(g_ShadowsDesc.m_ShadowRegion[i]));
                memcpy(&g_ShadowsDesc.m_Light[i].m_View, &g_LightData[i].m_View, sizeof(g_ShadowsDesc.m_Light[i].m_View));
                memcpy(&g_ShadowsDesc.m_Light[i].m_Projection, &g_LightData[i].m_Projection, sizeof(g_ShadowsDesc.m_Light[i].m_Projection));
                memcpy(&g_ShadowsDesc.m_Light[i].m_ViewProjection, &g_LightData[i].m_ViewProjection, sizeof(g_ShadowsDesc.m_Light[i].m_ViewProjection));
                memcpy(&g_ShadowsDesc.m_Light[i].m_View_Inv, &g_LightData[i].m_ViewInv, sizeof(g_ShadowsDesc.m_Light[i].m_View_Inv));
                memcpy(&g_ShadowsDesc.m_Light[i].m_Projection_Inv, &g_LightData[i].m_ProjectionInv, sizeof(g_ShadowsDesc.m_Light[i].m_Projection_Inv));
                memcpy(&g_ShadowsDesc.m_Light[i].m_ViewProjection_Inv, &g_LightData[i].m_ViewProjectionInv, sizeof(g_ShadowsDesc.m_Light[i].m_ViewProjection_Inv));

                memcpy(&g_ShadowsDesc.m_Light[i].m_Position, &g_LightData[i].m_Position, sizeof(g_ShadowsDesc.m_Light[i].m_Position));
                memcpy(&g_ShadowsDesc.m_Light[i].m_Up, &g_LightData[i].m_Up, sizeof(g_ShadowsDesc.m_Light[i].m_Up));
                memcpy(&g_ShadowsDesc.m_Light[i].m_Direction, &g_LightData[i].m_Direction, sizeof(g_ShadowsDesc.m_Light[i].m_Direction));

                g_ShadowsDesc.m_Light[i].m_Aspect = g_LightCamera.GetAspect();
                g_ShadowsDesc.m_Light[i].m_Fov = g_LightCamera.GetFOV();
                g_ShadowsDesc.m_Light[i].m_FarPlane = g_LightCamera.GetFarClip();
                g_ShadowsDesc.m_Light[i].m_NearPlane = g_LightCamera.GetNearClip();

                g_ShadowsDesc.m_SunArea[i] = g_SunSize * g_SunSize; // for the filtering we actually need squared size 
                g_ShadowsDesc.m_DepthTestOffset[i] = g_DepthTestOffset;
                g_ShadowsDesc.m_NormalOffsetScale[i] = g_NormalOffsetScale;
            }

            g_ShadowsDesc.m_pContext = pd3dContext;
            g_ShadowsDesc.m_pDevice = pd3dDevice;
            g_ShadowsDesc.m_pDepthSRV = g_AppDepth._srv;
            g_ShadowsDesc.m_pNormalSRV = g_AppNormal._srv;
            g_ShadowsDesc.m_pOutputRTV = g_ShadowMask._rtv;
            g_ShadowsDesc.m_OutputChannels = 1;
            g_ShadowsDesc.m_ReferenceDSS = 0;
            g_ShadowsDesc.m_pOutputDSS = NULL;
            g_ShadowsDesc.m_pOutputDSV = NULL;
            g_ShadowsDesc.m_EnableCapture = false;

            if (shadowTextureType == AMD::SHADOWFX_TEXTURE_2D || shadowTextureType == AMD::SHADOWFX_TEXTURE_2D_ARRAY)
            {
                g_ShadowsDesc.m_pShadowSRV = g_ShadowMap._srv;
            }

            g_ShadowsDesc.m_TextureType = (AMD::SHADOWFX_TEXTURE_TYPE) shadowTextureType;

            AMD::ShadowFX_Render(g_ShadowsDesc);

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 0 && EXPERIMENTAL_DELAY_END_ALL_ACCESS == 0)
            agsDriverExtensions_NotifyResourceEndAllAccess(g_agsContext, g_ShadowMap._t2d); // this is really the last point where we need the shadow map
#endif
        }
        TIMER_End();

        bCapture = false;

        TIMER_Begin(0, L"Scene Rendering");
        RenderScene(pd3dContext,
            g_MeshArray, g_MeshModelMatrix, AMD_ARRAY_SIZE(g_MeshArray),
            &CD3D11_VIEWPORT(0.0f, 0.0f, (float)g_Width, (float)g_Height), 1,
            pNullSR, 0,
            g_pBackCullingSolidRS, g_pOpaqueBS, white.f,
            g_pDepthTestLessEqualDSS, 0, g_pSceneIL,
            g_pSceneVS, pNullHS, pNullDS, pNullGS, g_pShadowedScenePS,
            g_pModelCB, 0, pCB, 0, AMD_ARRAY_SIZE(pCB),
            pSS, 0, AMD_ARRAY_SIZE(pSS), pSRV, 0, AMD_ARRAY_SIZE(pSRV),
            &pOriginalRTV, 1, g_AppDepth._dsv,
            &g_ViewerData, pNullCamera);

        for (int light = 0; light < CUBE_FACE_COUNT; light++)
        {
            pd3dContext->Map(g_pUnitCubeCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
            S_UNIT_CUBE_TRANSFORM* pUnitCubeCB = (S_UNIT_CUBE_TRANSFORM*)MappedResource.pData;
            {
                pUnitCubeCB->m_Transform = g_ViewerData.m_ViewProjection * g_LightData[light].m_ViewProjectionInv;
                pUnitCubeCB->m_Inverse = g_LightData[light].m_ViewProjectionInv;
                pUnitCubeCB->m_Forward = g_ViewerData.m_ViewProjection;
                pUnitCubeCB->m_Color = g_LightData[light].m_Color;
            }
            pd3dContext->Unmap(g_pUnitCubeCB, 0);

            AMD::RenderUnitCube(pd3dContext,
                CD3D11_VIEWPORT(0.0f, 0.0f, (float)g_Width, (float)g_Height),
                pNullSR, 0,
                g_pNoCullingWireframeRS,
                g_pOpaqueBS, white.f,
                g_pDepthTestLessDSS, 1,
                g_pUnitCubeVS, pNullHS, pNullDS, pNullGS, g_pUnitCubePS,
                &g_pUnitCubeCB, 0, 1,
                &pNullSS, 0, 0,
                &pNullSRV, 0, 0,
                &pOriginalRTV, 1, g_AppDepth._dsv);
        }

        if (g_bShowLightCamera)
        {
            RenderScene(pd3dContext,
                g_MeshArray, g_MeshModelMatrix, AMD_ARRAY_SIZE(g_MeshArray),
                &CD3D11_VIEWPORT(0.0f, 0.0f, (float)showLightArea, (float)showLightArea), 1,
                pNullSR, 0,
                g_pBackCullingSolidRS, g_pOpaqueBS, white.f,
                g_pDepthTestLessDSS, 0, g_pSceneIL,
                g_pSceneVS, pNullHS, pNullDS, pNullGS, g_pShadowMapPS,
                g_pModelCB, 0, pCB, 0, AMD_ARRAY_SIZE(pCB),
                pSS, 0, AMD_ARRAY_SIZE(pSS), pSRV, 1, AMD_ARRAY_SIZE(pSRV),
                &g_LightColor._rtv, 1, g_LightDepth._dsv,
                NULL,
                g_pCurrentCamera);
        }

        if (g_bShowShadowMapRegion)
        {
            RenderScene(pd3dContext,
                g_MeshArray, g_MeshModelMatrix, AMD_ARRAY_SIZE(g_MeshArray),
                &CD3D11_VIEWPORT(0.0f, 0.0f, (float)showLightArea, (float)showLightArea), 1,
                pNullSR, 0,
                g_pBackCullingSolidRS, g_pOpaqueBS, white.f,
                g_pDepthTestLessDSS, 0, g_pSceneIL,
                g_pSceneVS, pNullHS, pNullDS, pNullGS, g_pShadowMapPS,
                g_pModelCB, 0, pCB, 0, AMD_ARRAY_SIZE(pCB),
                pSS, 0, AMD_ARRAY_SIZE(pSS), pSRV, 1, AMD_ARRAY_SIZE(pSRV),
                &g_LightColor._rtv, 1, g_LightDepth._dsv,
                &g_LightData[g_CurrentLightCamera], pNullCamera);
        }

        if (g_bShowLightCamera || g_bShowShadowMapRegion)
            AMD::RenderFullscreenPass(pd3dContext,
                CD3D11_VIEWPORT(0.0f, 64.0f, 512.0f, 512.0f),
                g_pFullscreenVS, g_pFullscreenPS,
                pNullSR, 0, NULL, 0, &g_pLinearWrapSS, 1, &g_LightColor._srv, 1,
                &pOriginalRTV, 1, NULL, 0, 0, pNullDSV, NULL, 0, NULL, NULL);

        if (g_bShowShadowMask)
            AMD::RenderFullscreenPass(pd3dContext,
                CD3D11_VIEWPORT(0.0f, 0.0f, (float)g_Width, (float)g_Height),
                g_pFullscreenVS, g_pFullscreenPS,
                pNullSR, 0, NULL, 0, &g_pLinearWrapSS, 1, &g_ShadowMask._srv, 1,
                &pOriginalRTV, 1, NULL, 0, 0, pNullDSV, NULL, 0, NULL, NULL);
        TIMER_End();
    }

    // for experimental delay of end all access - this is roughly the end of the frame, let the driver know that we are done with corresponding resource
#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1 && EXPERIMENTAL_DELAY_END_ALL_ACCESS == 1)
    agsDriverExtensions_NotifyResourceEndAllAccess(g_agsContext, g_ShadowMapCopy._t2d);
#elif (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 0 && EXPERIMENTAL_DELAY_END_ALL_ACCESS == 1)
    agsDriverExtensions_NotifyResourceEndAllAccess(g_agsContext, g_ShadowMap._t2d);
#endif

    pd3dContext->RSSetViewports(1, &CD3D11_VIEWPORT(0.0f, 0.0f, (float)g_Width, (float)g_Height));

    pd3dContext->OMSetRenderTargets(1, &pOriginalRTV, pOriginalDSV);
    SAFE_RELEASE(pOriginalRTV);
    SAFE_RELEASE(pOriginalDSV);

    // Render GUI
    if (g_bRenderHUD)
    {
        DXUT_BeginPerfEvent(DXUT_PERFEVENTCOLOR, L"HUD / Stats");

        {
            // Render the HUD
            if (g_bRenderHUD)
            {
                g_MagnifyTool.Render();
                g_HUD.OnRender(fElapsedTime);
            }
            RenderText();
        }

        DXUT_EndPerfEvent();
    }

    fTimeShadowMap += (float)TIMER_GetTime(Gpu, L"Shadow Map Rendering") * 1000.0f;
    fTimeShadowMapFiltering += (float)TIMER_GetTime(Gpu, L"Shadow Map Filtering") * 1000.0f;
    fTimeDepthPrepass += (float)TIMER_GetTime(Gpu, L"Depth Prepass Rendering") * 1000.0f;
    fShadowMapMasking += (float)TIMER_GetTime(Gpu, L"Shadow Map Masking") * 1000.0f;
    fSceneRendering += (float)TIMER_GetTime(Gpu, L"Scene Rendering") * 1000.0f;

    if (nCount++ == 100)
    {
        g_ShadowRenderingTime = fTimeShadowMap / (float)nCount;
        g_ShadowFilteringTime = fTimeShadowMapFiltering / (float)nCount;
        g_DepthPrepassRenderingTime = fTimeDepthPrepass / (float)nCount;

        g_ShadowMapMasking = fShadowMapMasking / (float)nCount;
        g_SceneRendering = fSceneRendering / (float)nCount;

        fShadowMapMasking = fSceneRendering = fTimeShadowMap = fTimeShadowMapFiltering = fTimeDepthPrepass = 0.0f;
        nCount = 0;
    }
}

void CreateShaders(ID3D11Device * pDevice)
{
    ID3DBlob *code_blob = NULL;

    const D3D11_INPUT_ELEMENT_DESC SceneLayout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    if (AMD::CompileShaderFromFile(L"..\\src\\Shaders\\ShadowFX_Sample.hlsl", "VS_RenderScene", "vs_5_0", &code_blob, NULL) == S_OK)
    {
        pDevice->CreateVertexShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &g_pSceneVS);
        pDevice->CreateInputLayout(SceneLayout, AMD_ARRAY_SIZE(SceneLayout), code_blob->GetBufferPointer(), code_blob->GetBufferSize(), &g_pSceneIL);
        SAFE_RELEASE(code_blob);
    }

    if (AMD::CompileShaderFromFile(L"..\\src\\Shaders\\ShadowFX_Sample.hlsl", "PS_RenderShadowMap", "ps_5_0", &code_blob, NULL) == S_OK)
    {
        pDevice->CreatePixelShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &g_pShadowMapPS);
        SAFE_RELEASE(code_blob);
    }

    if (AMD::CompileShaderFromFile(L"..\\src\\Shaders\\ShadowFX_Sample.hlsl", "VS_RenderShadowMap", "vs_5_0", &code_blob, NULL) == S_OK)
    {
        pDevice->CreateVertexShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &g_pShadowMapVS);
        SAFE_RELEASE(code_blob);
    }

    if (AMD::CompileShaderFromFile(L"..\\src\\Shaders\\ShadowFX_Sample.hlsl", "PS_RenderShadowedScene", "ps_5_0", &code_blob, NULL) == S_OK)
    {
        pDevice->CreatePixelShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &g_pShadowedScenePS);
        SAFE_RELEASE(code_blob);
    }

    if (AMD::CompileShaderFromFile(L"..\\src\\Shaders\\ShadowFX_Sample.hlsl", "PS_DepthPassScene", "ps_5_0", &code_blob, NULL) == S_OK)
    {
        pDevice->CreatePixelShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &g_pDepthPassScenePS);
        SAFE_RELEASE(code_blob);
    }

    if (AMD::CompileShaderFromFile(L"..\\src\\Shaders\\ShadowFX_Sample.hlsl", "PS_DepthAndNormalPassScene", "ps_5_0", &code_blob, NULL) == S_OK)
    {
        pDevice->CreatePixelShader(code_blob->GetBufferPointer(), code_blob->GetBufferSize(), NULL, &g_pDepthAndNormalPassScenePS);
        SAFE_RELEASE(code_blob);
    }

    //g_ShaderCache.AddShader((ID3D11DeviceChild**)&g_pSceneVS, AMD::ShaderCache::SHADER_TYPE_VERTEX,
        //    L"vs_5_0", L"VS_RenderScene", L"ShadowFX_Sample.hlsl",
        //    0, NULL, &g_pSceneIL, (D3D11_INPUT_ELEMENT_DESC*)SceneLayout, ARRAYSIZE(SceneLayout));
        //DXUT_SetDebugName(g_pSceneVS, "g_pSceneVS");

        //g_ShaderCache.AddShader((ID3D11DeviceChild**)&g_pShadowMapPS, AMD::ShaderCache::SHADER_TYPE_PIXEL,
        //    L"ps_5_0", L"PS_RenderShadowMap", L"ShadowFX_Sample.hlsl", 0, NULL, NULL, NULL, 0);
        //DXUT_SetDebugName(g_pShadowMapPS, "g_pShadowMapPS");

        //g_ShaderCache.AddShader((ID3D11DeviceChild**)&g_pShadowMapVS, AMD::ShaderCache::SHADER_TYPE_VERTEX,
        //    L"vs_5_0", L"VS_RenderShadowMap", L"ShadowFX_Sample.hlsl", 0, NULL, NULL, NULL, 0);
        //DXUT_SetDebugName(g_pShadowMapVS, "g_pShadowMapVS");

        //g_ShaderCache.AddShader((ID3D11DeviceChild**)&g_pShadowedScenePS, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0",
        //    L"PS_RenderShadowedScene", L"ShadowFX_Sample.hlsl", 0, NULL, NULL, NULL, 0);
        //DXUT_SetDebugName(g_pShadowedScenePS, "g_pShadowedScenePS");

        //g_ShaderCache.AddShader((ID3D11DeviceChild**)&g_pDepthPassScenePS, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0",
        //    L"PS_DepthPassScene", L"ShadowFX_Sample.hlsl", 0, NULL, NULL, NULL, 0);
        //DXUT_SetDebugName(g_pDepthPassScenePS, "g_pDepthPassScenePS");

        //g_ShaderCache.AddShader((ID3D11DeviceChild**)&g_pDepthAndNormalPassScenePS, AMD::ShaderCache::SHADER_TYPE_PIXEL, L"ps_5_0",
        //    L"PS_DepthAndNormalPassScene", L"ShadowFX_Sample.hlsl", 0, NULL, NULL, NULL, 0);
        //DXUT_SetDebugName(g_pDepthAndNormalPassScenePS, "g_pDepthAndNormalPassScenePS");


    AMD::CreateClipSpaceCube(&g_pUnitCubeVS, pDevice);
    AMD::CreateFullscreenPass(&g_pFullscreenVS, pDevice);
    AMD::CreateUnitCube(&g_pUnitCubePS, pDevice);
    AMD::CreateFullscreenPass(&g_pFullscreenPS, pDevice);
}


//--------------------------------------------------------------------------------------
// Release D3D11 resources created in OnD3D11CreateDevice
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11DestroyDevice(void* pUserContext)
{
    g_DialogResourceManager.OnD3D11DestroyDevice();
    g_SettingsDlg.OnD3D11DestroyDevice();
    DXUTGetGlobalResourceCache().OnDestroyDevice();
    SAFE_DELETE(g_pTxtHelper);

    g_MagnifyTool.OnDestroyDevice();
    g_HUD.OnDestroyDevice();

    agsDriverExtensions_DeInit(g_agsContext);
    agsDeInit(g_agsContext);

    TIMER_Destroy();

    g_Tree.Release();
    g_BoxPlane.Release();

    SAFE_RELEASE(g_pShadowMapVS);
    SAFE_RELEASE(g_pSceneVS);
    SAFE_RELEASE(g_pShadowMapPS);
    SAFE_RELEASE(g_pShadowedScenePS);
    SAFE_RELEASE(g_pDepthPassScenePS);
    SAFE_RELEASE(g_pDepthAndNormalPassScenePS);


    SAFE_RELEASE(g_pFullscreenVS);
    SAFE_RELEASE(g_pFullscreenPS);

    SAFE_RELEASE(g_pUnitCubeVS);
    SAFE_RELEASE(g_pUnitCubePS);
    SAFE_RELEASE(g_pUnitCubeCB);

    SAFE_RELEASE(g_pSceneIL);

    SAFE_RELEASE(g_pLinearWrapSS);

    SAFE_RELEASE(g_pNoCullingSolidRS);
    SAFE_RELEASE(g_pBackCullingSolidRS);
    SAFE_RELEASE(g_pFrontCullingSolidRS);
    SAFE_RELEASE(g_pNoCullingWireframeRS);

    SAFE_RELEASE(g_pDepthTestLessDSS);
    SAFE_RELEASE(g_pDepthTestLessEqualDSS);
    SAFE_RELEASE(g_pStencilTestAndClearDSS);
    SAFE_RELEASE(g_pDepthTestMarkStencilDSS);

    SAFE_RELEASE(g_pOpaqueBS);
    SAFE_RELEASE(g_pShadowMaskChannelBS[0]);
    SAFE_RELEASE(g_pShadowMaskChannelBS[1]);
    SAFE_RELEASE(g_pShadowMaskChannelBS[2]);
    SAFE_RELEASE(g_pShadowMaskChannelBS[3]);

    SAFE_RELEASE(g_pViewerCB);
    SAFE_RELEASE(g_pModelCB);
    SAFE_RELEASE(g_pLightCB);

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1)
    g_ShadowMapCopy.Release();
#endif

    g_ShadowMap.Release();
    g_ShadowMask.Release();
    g_AppDepth.Release();
    g_AppNormal.Release();
    g_LightDepth.Release();
    g_LightColor.Release();

    AMD::ShadowFX_Release(g_ShadowsDesc);
}


//--------------------------------------------------------------------------------------
// Release swap chain and backbuffer associated resources
//--------------------------------------------------------------------------------------
void CALLBACK OnD3D11ReleasingSwapChain(void* pUserContext)
{
    g_DialogResourceManager.OnD3D11ReleasingSwapChain();

    g_ShadowMask.Release();
    g_AppDepth.Release();
    g_AppNormal.Release();
}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
