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

typedef enum SHADOWFX_CROSSFIRE_UI_MODE_t
{
    SHADOWFX_CROSSFIRE_UI_MODE_DISABLE_SUBMENU,
    SHADOWFX_CROSSFIRE_UI_MODE_ENABLE_SUBMENU,
    SHADOWFX_CROSSFIRE_UI_MODE_DISABLE_FULL_MENU,
    SHADOWFX_CROSSFIRE_UI_MODE_ENABLE_FULL_MENU,
    SHADOWFX_CROSSFIRE_UI_MODE_MORE_THAN_TWO_GPUS,

    SHADOWFX_CROSSFIRE_UI_MODE_COUNT,
} SHADOWFX_CROSSFIRE_UI_MODE;

//--------------------------------------------------------------------------------------
// Enable/disable the Crossfire UI
//--------------------------------------------------------------------------------------
void UpdateCrossfireUI(SHADOWFX_CROSSFIRE_UI_MODE uiMode)
{
    const bool enableTopLevelCheckBox = (uiMode != SHADOWFX_CROSSFIRE_UI_MODE_DISABLE_FULL_MENU);
    g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_ENABLE_CROSSFIRE_API)->SetEnabled(enableTopLevelCheckBox);

    if (uiMode == SHADOWFX_CROSSFIRE_UI_MODE_DISABLE_FULL_MENU)
    {
        g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_ENABLE_CROSSFIRE_API)->SetChecked(false);
    }

    const bool topLevelCheckBoxIsChecked = g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_ENABLE_CROSSFIRE_API)->GetChecked();
    const bool enableSubmenu = ((uiMode == SHADOWFX_CROSSFIRE_UI_MODE_ENABLE_SUBMENU) || (uiMode == SHADOWFX_CROSSFIRE_UI_MODE_ENABLE_FULL_MENU) || (uiMode == SHADOWFX_CROSSFIRE_UI_MODE_MORE_THAN_TWO_GPUS)) && topLevelCheckBoxIsChecked;

    if (uiMode == SHADOWFX_CROSSFIRE_UI_MODE_MORE_THAN_TWO_GPUS)
    {
        g_HUD.m_GUI.GetStatic(IDC_STATIC_TRANSFER_FLAG)->SetEnabled(false);
        g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_1STEP)->SetEnabled(false);
        g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP)->SetEnabled(false);
        g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP_BROADCAST)->SetChecked(true);
        g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP_BROADCAST)->SetEnabled(false);
    }
    else
    {
        g_HUD.m_GUI.GetStatic(IDC_STATIC_TRANSFER_FLAG)->SetEnabled(enableSubmenu);
        g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_1STEP)->SetEnabled(enableSubmenu);
        g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP)->SetEnabled(enableSubmenu);
        g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP_BROADCAST)->SetEnabled(enableSubmenu);
    }

    g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_ENABLE_2_STEP_GPU_TRANSFER)->SetEnabled(enableSubmenu);
    g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_DELAY_END_ALL_ACCESS)->SetEnabled(enableSubmenu);
}

//--------------------------------------------------------------------------------------
// Initialize the app
//--------------------------------------------------------------------------------------
void InitApplicationUI()
{
    CDXUTComboBox *pCombo;

    D3DCOLOR DlgColor = 0x88888888; // Semi-transparent background for the dialog

    g_SettingsDlg.Init(&g_DialogResourceManager);
    g_HUD.m_GUI.Init(&g_DialogResourceManager);

    g_HUD.m_GUI.SetBackgroundColors(DlgColor);
    g_HUD.m_GUI.SetCallback(OnGUIEvent);

    // Disable MSAA
    g_SettingsDlg.GetDialogControl()->GetControl(DXUTSETTINGSDLG_D3D11_MULTISAMPLE_COUNT)->SetEnabled(false);
    g_SettingsDlg.GetDialogControl()->GetControl(DXUTSETTINGSDLG_D3D11_MULTISAMPLE_QUALITY)->SetEnabled(false);

    int iY = AMD::HUD::iElementDelta;
    g_HUD.m_GUI.AddButton(IDC_TOGGLEFULLSCREEN, L"Toggle full screen", AMD::HUD::iElementOffset, iY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight);
    g_HUD.m_GUI.AddButton(IDC_TOGGLEREF, L"Toggle REF (F3)", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, VK_F3);
    g_HUD.m_GUI.AddButton(IDC_CHANGEDEVICE, L"Change device (F2)", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, VK_F2);
    iY += AMD::HUD::iGroupDelta;

    g_HUD.m_GUI.AddStatic(IDC_STATIC_SHADOW_MAP, L"Shadow Map Resource", AMD::HUD::iElementOffset, iY, 140, 24);
    g_HUD.m_GUI.AddRadioButton(IDC_RADIO_SHADOW_MAP_T2D, IDC_STATIC_SHADOW_MAP, L"Atlas", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth / 2, 24, true);
    g_HUD.m_GUI.AddRadioButton(IDC_RADIO_SHADOW_MAP_T2DA, IDC_STATIC_SHADOW_MAP, L"Array", AMD::HUD::iElementOffset + AMD::HUD::iElementWidth / 2, iY, AMD::HUD::iElementWidth / 2, 24, false);

    g_HUD.m_GUI.AddStatic(IDC_STATIC_SHADOWMAP_SIZE, L"Shadow Map Size:", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight);
    g_HUD.m_GUI.AddComboBox(IDC_COMBOBOX_SHADOWMAP_SIZE, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, true, &pCombo);
    if (pCombo)
    {
        pCombo->SetDropHeight(35);
        pCombo->AddItem(L"512x512", NULL);
        pCombo->AddItem(L"1024x1024", NULL);
        pCombo->AddItem(L"2048x2048", NULL);
        pCombo->AddItem(L"4096x4096", NULL);
        pCombo->SetSelectedByIndex(1);
    }

    g_HUD.m_GUI.AddCheckBox(IDC_CHECKBOX_ENABLE_CROSSFIRE_API, L"Enable Crossfire API", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 140, 24, g_EnableCrossfireApiTransfers);

    g_HUD.m_GUI.AddStatic(IDC_STATIC_TRANSFER_FLAG, L"CFX API Transfer Flag", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 140, 24);
    g_HUD.m_GUI.AddRadioButton(IDC_RADIO_TRANSFER_FLAG_1STEP, IDC_RADIO_BUTTON_GROUP_TRANSFER_FLAG,           L"1 STEP",           AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, 24, g_ResourceCfxTransferFlag == AGS_AFR_TRANSFER_1STEP_P2P);
    g_HUD.m_GUI.AddRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP, IDC_RADIO_BUTTON_GROUP_TRANSFER_FLAG,           L"2 STEP",           AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, 24, g_ResourceCfxTransferFlag == AGS_AFR_TRANSFER_2STEP_NO_BROADCAST);
    g_HUD.m_GUI.AddRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP_BROADCAST, IDC_RADIO_BUTTON_GROUP_TRANSFER_FLAG, L"2 STEP BROADCAST", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, 24, g_ResourceCfxTransferFlag == AGS_AFR_TRANSFER_2STEP_WITH_BROADCAST);

    g_HUD.m_GUI.AddCheckBox(IDC_CHECKBOX_ENABLE_2_STEP_GPU_TRANSFER, L"Enable 2 Step Transfer", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 140, 24, g_Enable2StepGpuTransfer);
    g_HUD.m_GUI.AddCheckBox(IDC_CHECKBOX_DELAY_END_ALL_ACCESS, L"Delay EndAllAccess()", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 140, 24, g_DelayEndAllAccess);

    g_HUD.m_GUI.AddCheckBox(IDC_CHECKBOX_ENABLE_1_FACE_UPDATE_PER_FRAME, L"Single face / frame", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 140, 24, false);


    // Add the magnify tool UI to our HUD
    g_MagnifyTool.InitApp(&g_HUD.m_GUI, iY += AMD::HUD::iElementDelta);

    g_ViewerCamera.SetRotateButtons(true, false, false);
    g_LightCamera.SetRotateButtons(true, false, false);
}

//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the
// application to modify the device settings. The supplied pDeviceSettings parameter
// contains the settings that the framework has selected for the new device, and the
// application can make any desired changes directly to this structure.  Note however that
// DXUT will not correct invalid device settings so care must be taken
// to return valid device settings, otherwise CreateDevice() will fail.
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings(DXUTDeviceSettings* pDeviceSettings, void* pUserContext)
{
    assert(pDeviceSettings->MinimumFeatureLevel == D3D_FEATURE_LEVEL_11_0);
    pDeviceSettings->d3d11.SyncInterval = 0;

    pDeviceSettings->d3d11.sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

    // For the first device created if it is a REF device, optionally display a warning dialog box
    static bool s_bFirstTime = true;
    if (s_bFirstTime)
    {
        s_bFirstTime = false;
        if (pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE)
        {
            DXUTDisplaySwitchingToREFWarning();
        }
    }

    // Disable MSAA
    pDeviceSettings->d3d11.sd.SampleDesc.Count = 1;

    return true;
}

//--------------------------------------------------------------------------------------
// Render stats
//--------------------------------------------------------------------------------------
void RenderText()
{
    g_pTxtHelper->Begin();

    g_pTxtHelper->SetInsertionPos(2, 0);

    g_pTxtHelper->SetForegroundColor(XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f));
    g_pTxtHelper->DrawTextLine(DXUTGetFrameStats(DXUTIsVsyncEnabled()));
    g_pTxtHelper->DrawTextLine(DXUTGetDeviceStats());
    WCHAR szTemp[256];
    if (g_agsGpuCount > 1)
    {
        swprintf_s(szTemp, L"Running on a system with %d GPUs\n", g_agsGpuCount);
        g_pTxtHelper->DrawTextLine(szTemp);
    }
    swprintf_s(szTemp, L"Effect cost in milliseconds (Depth Prepass Rendering = %.3f, Shadow Rendering = %.3f, Shadow Filtering = %.3f)", g_DepthPrepassRenderingTime, g_ShadowRenderingTime, g_ShadowFilteringTime);
    g_pTxtHelper->DrawTextLine(szTemp);
    swprintf_s(szTemp, L"Effect cost in milliseconds (Scene Rendering = %.3f, Shadow Map Masking = %.3f)", g_SceneRendering, g_ShadowMapMasking);
    g_pTxtHelper->DrawTextLine(szTemp);

    g_pTxtHelper->SetInsertionPos(10, DXUTGetDXGIBackBufferSurfaceDesc()->Height - 120);
    g_pTxtHelper->DrawTextLine(L"Switch to Camera Camera   : Press '9' \n"
                               L"Switch to Light Camera    : Press 'l' or 'L' \n"
                               L"Switch to Light Frustum   : Press {1 | 2 | 3 | 4 | 5 | 6} \n"
                               L"View Filtered Shadow (on / off) : Press 'm' or 'M' \n"
                               L"Toggle GUI                : F1\n");

    g_pTxtHelper->SetInsertionPos(DXUTGetDXGIBackBufferSurfaceDesc()->Width / 2 - 90, DXUTGetDXGIBackBufferSurfaceDesc()->Height - 40);
    g_pTxtHelper->DrawTextLine(L"\nCamera Control: W/S/A/D/Q/E");

    g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard(UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext)
{
    static int iCaptureNumber = 0;
#define VK_C (67)
#define VK_G (71)

    if (bKeyDown)
    {
        switch (nChar)
        {
        case VK_F1:
            g_bRenderHUD = !g_bRenderHUD;
            break;
        case 'L': case 'l':
            g_pCurrentCamera = &g_LightCamera;
            break;
        case '1': case '2': case '3':
        case '4': case '5': case '6':
            g_CurrentLightCamera = nChar - '1';
            g_pCurrentCamera = &g_CubeCamera[g_CurrentLightCamera];
            break;
        case '9': g_pCurrentCamera = &g_ViewerCamera;
            break;

        }
    }
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent(UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext)
{
    switch (nControlID)
    {

    case IDC_RADIO_TRANSFER_FLAG_1STEP:
    case IDC_RADIO_TRANSFER_FLAG_2STEP:
    case IDC_RADIO_TRANSFER_FLAG_2STEP_BROADCAST:

        if (g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_1STEP)->GetChecked())
            g_ResourceCfxTransferFlag = AGS_AFR_TRANSFER_1STEP_P2P;
        if (g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP)->GetChecked())
            g_ResourceCfxTransferFlag = AGS_AFR_TRANSFER_2STEP_NO_BROADCAST;
        if (g_HUD.m_GUI.GetRadioButton(IDC_RADIO_TRANSFER_FLAG_2STEP_BROADCAST)->GetChecked())
            g_ResourceCfxTransferFlag = AGS_AFR_TRANSFER_2STEP_WITH_BROADCAST;

        InitTransferredResources(DXUTGetD3D11Device());
        break;

    case IDC_CHECKBOX_ENABLE_CROSSFIRE_API:
        {
            const bool enableCrossfireUI = g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_ENABLE_CROSSFIRE_API)->GetChecked();
            SHADOWFX_CROSSFIRE_UI_MODE crossfireUIMode = enableCrossfireUI ? SHADOWFX_CROSSFIRE_UI_MODE_ENABLE_SUBMENU : SHADOWFX_CROSSFIRE_UI_MODE_DISABLE_SUBMENU;
            crossfireUIMode = (g_agsGpuCount > 2) ? SHADOWFX_CROSSFIRE_UI_MODE_MORE_THAN_TWO_GPUS : crossfireUIMode;
            UpdateCrossfireUI(crossfireUIMode);
        }
        // Fall through on purpose
    case IDC_CHECKBOX_ENABLE_2_STEP_GPU_TRANSFER:
    case IDC_CHECKBOX_DELAY_END_ALL_ACCESS:

        g_EnableCrossfireApiTransfers = g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_ENABLE_CROSSFIRE_API)->GetChecked();
        g_Enable2StepGpuTransfer = g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_ENABLE_2_STEP_GPU_TRANSFER)->GetChecked();
        g_DelayEndAllAccess = g_HUD.m_GUI.GetCheckBox(IDC_CHECKBOX_DELAY_END_ALL_ACCESS)->GetChecked();

        if (g_EnableCrossfireApiTransfers == false)
        {
            g_ShadowMapCfxFlag = g_ShadowMapTransferCfxFlag = AGS_AFR_TRANSFER_DEFAULT;
        }
        else
        {
            if (g_Enable2StepGpuTransfer == true)
            {
                g_ShadowMapCfxFlag = AGS_AFR_TRANSFER_DISABLE;
                g_ShadowMapTransferCfxFlag = g_ResourceCfxTransferFlag;
            }
            else
            {
                g_ShadowMapCfxFlag = g_ResourceCfxTransferFlag;
                g_ShadowMapTransferCfxFlag = AGS_AFR_TRANSFER_DISABLE;
            }
        }

        InitTransferredResources(DXUTGetD3D11Device());
        break;


    case IDC_RADIO_SHADOW_MAP_T2D:
    case IDC_RADIO_SHADOW_MAP_T2DA:

        if (g_HUD.m_GUI.GetRadioButton(IDC_RADIO_SHADOW_MAP_T2DA)->GetChecked())
        {
            g_ShadowTextureType = AMD::SHADOWFX_TEXTURE_2D_ARRAY;
        }

        if (g_HUD.m_GUI.GetRadioButton(IDC_RADIO_SHADOW_MAP_T2D)->GetChecked())
        {
            g_ShadowTextureType = AMD::SHADOWFX_TEXTURE_2D;
        }

        InitTransferredResources(DXUTGetD3D11Device());
        break;

    case IDC_COMBOBOX_SHADOWMAP_SIZE:
        g_ShadowMapSize = (float)(1 << (((CDXUTComboBox*)pControl)->GetSelectedIndex() + 9));

        for (int light = 0; light < CUBE_FACE_COUNT; light++)
        {
            g_LightData[light].m_BackBufferDim = float2(g_ShadowMapSize, g_ShadowMapSize);
            g_LightData[light].m_BackBufferDimRcp = float2(1.0f / g_ShadowMapSize, 1.0f / g_ShadowMapSize);
        }

        InitTransferredResources(DXUTGetD3D11Device());
        break;

    case IDC_TOGGLEFULLSCREEN:
        DXUTToggleFullScreen();
        break;

    case IDC_TOGGLEREF:
        DXUTToggleREF();
        break;

    case IDC_CHANGEDEVICE:
        g_SettingsDlg.SetActive(!g_SettingsDlg.IsActive());
        break;
    }
    // Call the MagnifyTool gui event handler
    g_MagnifyTool.OnGUIEvent(nEvent, nControlID, pControl, pUserContext);
}