
//--------------------------------------------------------------------------------------
// Initialize the app
//--------------------------------------------------------------------------------------
void InitApplicationUI()
{
  CDXUTComboBox *pCombo;

  D3DCOLOR DlgColor = 0x88888888; // Semi-transparent background for the dialog

  g_SettingsDlg.Init( &g_DialogResourceManager );
  g_HUD.m_GUI.Init( &g_DialogResourceManager );

  g_HUD.m_GUI.SetBackgroundColors( DlgColor );
  g_HUD.m_GUI.SetCallback( OnGUIEvent );

  // Disable MSAA
  g_SettingsDlg.GetDialogControl()->GetControl(DXUTSETTINGSDLG_D3D11_MULTISAMPLE_COUNT)->SetEnabled(false);
  g_SettingsDlg.GetDialogControl()->GetControl(DXUTSETTINGSDLG_D3D11_MULTISAMPLE_QUALITY)->SetEnabled(false);

  int iY = AMD::HUD::iElementDelta;
  g_HUD.m_GUI.AddButton( IDC_TOGGLEFULLSCREEN, L"Toggle full screen", AMD::HUD::iElementOffset, iY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
  g_HUD.m_GUI.AddButton( IDC_TOGGLEREF, L"Toggle REF (F3)", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, VK_F3 );
  g_HUD.m_GUI.AddButton( IDC_CHANGEDEVICE, L"Change device (F2)", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, VK_F2 );
  iY += AMD::HUD::iGroupDelta;

  g_HUD.m_GUI.AddStatic( IDC_STATIC_SHADOW_MAP, L"Shadow Map Resource", AMD::HUD::iElementOffset, iY, 140, 24);
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOW_MAP_T2D, IDC_STATIC_SHADOW_MAP, L"Atlas", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth / 2, 24, true );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOW_MAP_T2DA, IDC_STATIC_SHADOW_MAP, L"Array", AMD::HUD::iElementOffset + AMD::HUD::iElementWidth / 2, iY, AMD::HUD::iElementWidth / 2, 24, false );

  g_HUD.m_GUI.AddStatic( IDC_STATIC_SHADOWMAP_SIZE, L"Shadow Map Size:", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
  g_HUD.m_GUI.AddComboBox( IDC_COMBOBOX_SHADOWMAP_SIZE, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, true, &pCombo );
  if( pCombo )
  {
    pCombo->SetDropHeight( 35 );
    pCombo->AddItem( L"512x512", NULL );
    pCombo->AddItem( L"1024x1024", NULL );
    pCombo->AddItem( L"2048x2048", NULL );
    pCombo->AddItem( L"4096x4096", NULL );
    pCombo->SetSelectedByIndex( 1 );
  }

  g_HUD.m_GUI.AddStatic( IDC_STATIC_SHADOWFX_TYPE, L"Shadow Type", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 140, 24);
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_TYPE_UNION, IDC_STATIC_SHADOWFX_TYPE, L"Union", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, 24, g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_UNION );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_TYPE_CASCADE, IDC_STATIC_SHADOWFX_TYPE, L"Cascade", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, 24, g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_CASCADE );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_TYPE_CUBE, IDC_STATIC_SHADOWFX_TYPE, L"Cube", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, 24, g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_CUBE );

  g_HUD.m_GUI.AddCheckBox( IDC_CHECKBOX_FILTERING, L"Enable Shadow Filtering", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 140, 24, true );
  g_HUD.m_GUI.AddStatic( IDC_STATIC_SHADOWFX_FILTERING, L"Shadow Filtering Mode:", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_FILTERING_UNIFORM, IDC_STATIC_SHADOWFX_FILTERING, L"Uniform Filtering", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, true );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_FILTERING_CONTACT, IDC_STATIC_SHADOWFX_FILTERING, L"Contact Filtering", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, false );
 
  g_HUD.m_GUI.AddStatic( IDC_STATIC_SHADOWFX_NORMAL_OPTION, L"Shadow Normal Option:", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_NORMAL_OPTION_NONE, IDC_STATIC_SHADOWFX_NORMAL_OPTION, L"None", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, true );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_NORMAL_OPTION_CALC, IDC_STATIC_SHADOWFX_NORMAL_OPTION, L"Calc from Depth", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, false );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_NORMAL_OPTION_READ, IDC_STATIC_SHADOWFX_NORMAL_OPTION, L"Read from SRV", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, false );

  g_HUD.m_GUI.AddStatic( IDC_STATIC_SHADOWFX_TAP_TYPE, L"Shadow Tap Type:", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_TAP_TYPE_FIXED, IDC_STATIC_SHADOWFX_TAP_TYPE, L"Fixed", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, true );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_TAP_TYPE_POISSON, IDC_STATIC_SHADOWFX_TAP_TYPE, L"Poisson", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, false );

  g_HUD.m_GUI.AddStatic( IDC_RADIO_SHADOWFX_TEXTURE_FETCH, L"Shadow Texture Fetch:", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_TEXTURE_FETCH_GATHER4, IDC_RADIO_SHADOWFX_TEXTURE_FETCH, L"Gather4", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth / 2, AMD::HUD::iElementHeight, true );
  g_HUD.m_GUI.AddRadioButton( IDC_RADIO_SHADOWFX_TEXTURE_FETCH_PCF, IDC_RADIO_SHADOWFX_TEXTURE_FETCH, L"PCF", AMD::HUD::iElementOffset + AMD::HUD::iElementWidth / 2, iY, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, false );
  
  WCHAR szTemp[256];
  swprintf_s( szTemp, L"Sun Width : %.2f", g_SunSize );
  g_HUD.m_GUI.AddStatic( IDC_STATIC_SUN_SIZE, szTemp, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 108, 24 );
  g_HUD.m_GUI.AddSlider( IDC_SLIDER_SUN_SIZE, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, 50, (int)(g_SunSize), false );

  swprintf_s( szTemp, L"Depth Test Offset: %.5f", g_DepthTestOffset );
  g_HUD.m_GUI.AddStatic( IDC_STATIC_DEPTH_TEST_OFFSET, szTemp, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 108, 24 );
  g_HUD.m_GUI.AddSlider( IDC_SLIDER_DEPTH_OFFSET_TEST, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, 100, (int)(g_DepthTestOffset) * 1000 * 100, false );

  swprintf_s( szTemp, L"Normal Offset Scale: %.3f", g_NormalOffsetScale );
  g_HUD.m_GUI.AddStatic( IDC_STATIC_NORMAL_OFFSET_SCALE, szTemp, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, 108, 24 );
  g_HUD.m_GUI.AddSlider( IDC_SLIDER_NORMAL_OFFSET_SCALE, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, 10, (int)(g_NormalOffsetScale) * 1000, false );

  g_HUD.m_GUI.AddStatic( IDC_STATIC_SHADOWFX_FILTER_SIZE, L"Kernel Size:", AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight );
  g_HUD.m_GUI.AddComboBox( IDC_COMBOBOX_SHADOWFX_FILTER_SIZE, AMD::HUD::iElementOffset, iY += AMD::HUD::iElementDelta, AMD::HUD::iElementWidth, AMD::HUD::iElementHeight, 0, true, &pCombo );
  if( pCombo )
  {
    pCombo->SetDropHeight( 35 );
    pCombo->AddItem( L"7x7", NULL );
    pCombo->AddItem( L"9x9", NULL );
    pCombo->AddItem( L"11x11", NULL );
    pCombo->AddItem( L"13x13", NULL );
    pCombo->AddItem( L"15x15", NULL );
    pCombo->SetSelectedByIndex( 0 );
  }
  iY += AMD::HUD::iGroupDelta;

  // Add the magnify tool UI to our HUD
  g_MagnifyTool.InitApp( &g_HUD.m_GUI, iY += AMD::HUD::iElementDelta );

  g_ViewerCamera.SetRotateButtons( true, false, false );
  g_LightCamera.SetRotateButtons( true, false, false );
}

//--------------------------------------------------------------------------------------
// This callback function is called immediately before a device is created to allow the
// application to modify the device settings. The supplied pDeviceSettings parameter
// contains the settings that the framework has selected for the new device, and the
// application can make any desired changes directly to this structure.  Note however that
// DXUT will not correct invalid device settings so care must be taken
// to return valid device settings, otherwise CreateDevice() will fail.
//--------------------------------------------------------------------------------------
bool CALLBACK ModifyDeviceSettings( DXUTDeviceSettings* pDeviceSettings, void* pUserContext )
{
  assert( pDeviceSettings->MinimumFeatureLevel == D3D_FEATURE_LEVEL_11_0 );
  pDeviceSettings->d3d11.SyncInterval = 0;

  pDeviceSettings->d3d11.sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

  // For the first device created if it is a REF device, optionally display a warning dialog box
  static bool s_bFirstTime = true;
  if( s_bFirstTime )
  {
    s_bFirstTime = false;
    if( pDeviceSettings->d3d11.DriverType == D3D_DRIVER_TYPE_REFERENCE )
    {
      DXUTDisplaySwitchingToREFWarning( );
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

  g_pTxtHelper->SetInsertionPos( 2, 0 );
  if (g_bShowShadowMask)
      g_pTxtHelper->SetForegroundColor( XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ) );
  else
      g_pTxtHelper->SetForegroundColor( XMFLOAT4( 1.0f, 0.0f, 0.0f, 1.0f ) );
  g_pTxtHelper->DrawTextLine( DXUTGetFrameStats( DXUTIsVsyncEnabled() ) );
  g_pTxtHelper->DrawTextLine( DXUTGetDeviceStats() );
  WCHAR szTemp[256];
  swprintf_s( szTemp, L"Effect cost in milliseconds (Depth Prepass Rendering = %.3f, Shadow Rendering = %.3f, Shadow Filtering = %.3f)", g_DepthPrepassRenderingTime, g_ShadowRenderingTime, g_ShadowFilteringTime);
  g_pTxtHelper->DrawTextLine( szTemp );
  swprintf_s( szTemp, L"Effect cost in milliseconds (Scene Rendering = %.3f, Shadow Map Masking = %.3f)", g_SceneRendering, g_ShadowMapMasking);
  g_pTxtHelper->DrawTextLine( szTemp );

  g_pTxtHelper->SetInsertionPos( 10, DXUTGetDXGIBackBufferSurfaceDesc()->Height - 120 );
  g_pTxtHelper->DrawTextLine(L"Switch to Camera Camera   : Press '9' \n"
                             L"Switch to Light Camera    : Press 'l' or 'L' \n"
                             L"Switch to Light Frustum   : Press {1 | 2 | 3 | 4 | 5 | 6} \n"
                             L"View Filtered Shadow (on / off) : Press 'm' or 'M' \n"
                             L"Toggle GUI                : F1\n");

  g_pTxtHelper->SetInsertionPos( DXUTGetDXGIBackBufferSurfaceDesc()->Width / 2 - 90,   DXUTGetDXGIBackBufferSurfaceDesc()->Height - 40 );
  g_pTxtHelper->DrawTextLine( L"\nCamera Control: W/S/A/D/Q/E" );

  g_pTxtHelper->End();
}

//--------------------------------------------------------------------------------------
// Handle key presses
//--------------------------------------------------------------------------------------
void CALLBACK OnKeyboard( UINT nChar, bool bKeyDown, bool bAltDown, void* pUserContext )
{
  static int iCaptureNumber = 0;
#define VK_C (67)
#define VK_G (71)

  if( bKeyDown )
  {
    switch( nChar )
    {
    case VK_F1:
      g_bRenderHUD = !g_bRenderHUD;
      break;
    case 'L' : case 'l' : 
      g_pCurrentCamera = &g_LightCamera; 
      g_bShowLightCamera = true; 
      g_bShowShadowMapRegion = false;
      break;
    case '1': case '2': case '3': 
    case '4': case '5': case '6': 
      g_CurrentLightCamera = nChar - '1';
      g_pCurrentCamera = &g_CubeCamera[g_CurrentLightCamera];
      g_bShowLightCamera = false;
      g_bShowShadowMapRegion = true;
      break;
    case '9': g_pCurrentCamera = &g_ViewerCamera;
      g_bShowLightCamera = false;
      g_bShowShadowMapRegion = false;
      break;

    case 'm': case 'M' : 
      g_bShowShadowMask = !g_bShowShadowMask;
      break;
    }
  }
}

//--------------------------------------------------------------------------------------
// Handles the GUI events
//--------------------------------------------------------------------------------------
void CALLBACK OnGUIEvent( UINT nEvent, int nControlID, CDXUTControl* pControl, void* pUserContext )
{
  WCHAR szTemp[256];

  switch( nControlID )
  {
  case IDC_CHECKBOX_FILTERING:
    if (g_HUD.m_GUI.GetCheckBox( IDC_CHECKBOX_FILTERING )->GetChecked())
    {
      g_HUD.m_GUI.GetStatic( IDC_STATIC_SUN_SIZE )->SetEnabled(TRUE);
      g_HUD.m_GUI.GetSlider( IDC_SLIDER_SUN_SIZE )->SetEnabled(TRUE);
      g_HUD.m_GUI.GetStatic( IDC_STATIC_SHADOWFX_FILTER_SIZE )->SetEnabled(TRUE);
      g_HUD.m_GUI.GetComboBox( IDC_COMBOBOX_SHADOWFX_FILTER_SIZE )->SetEnabled(TRUE);
    }
    else
    {
      g_HUD.m_GUI.GetStatic( IDC_STATIC_SUN_SIZE )->SetEnabled(FALSE);
      g_HUD.m_GUI.GetSlider( IDC_SLIDER_SUN_SIZE )->SetEnabled(FALSE);
      g_HUD.m_GUI.GetStatic(IDC_STATIC_SHADOWFX_FILTER_SIZE)->SetEnabled(FALSE);
      g_HUD.m_GUI.GetComboBox( IDC_COMBOBOX_SHADOWFX_FILTER_SIZE )->SetEnabled(FALSE);
    }
    break;

  case IDC_RADIO_SHADOWFX_TYPE_UNION:
  case IDC_RADIO_SHADOWFX_TYPE_CASCADE:
  case IDC_RADIO_SHADOWFX_TYPE_CUBE:
    
    g_ShadowsExecution = AMD::SHADOWFX_EXECUTION_UNION;
    g_ShadowsExecution = g_HUD.m_GUI.GetRadioButton( IDC_RADIO_SHADOWFX_TYPE_CASCADE )->GetChecked() ? AMD::SHADOWFX_EXECUTION_CASCADE : g_ShadowsExecution;
    g_ShadowsExecution = g_HUD.m_GUI.GetRadioButton( IDC_RADIO_SHADOWFX_TYPE_CUBE )->GetChecked() ? AMD::SHADOWFX_EXECUTION_CUBE : g_ShadowsExecution;

    if ((g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_CUBE ||
         g_ShadowsExecution == AMD::SHADOWFX_EXECUTION_UNION)) // if using light camera - need to update constant buffer values
        InitializeCubeCamera(&g_LightCamera, g_CubeCamera, g_LightData);

  break;

  case IDC_RADIO_SHADOW_MAP_T2D:
  case IDC_RADIO_SHADOW_MAP_T2DA:
    
    if (g_HUD.m_GUI.GetRadioButton( IDC_RADIO_SHADOW_MAP_T2DA )->GetChecked())
    {
      g_ShadowMap.Release();
      g_ShadowMap.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize, (unsigned int)g_ShadowMapSize, 1, 6, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, true, 0, NULL, NULL, g_ShadowMapCfxTransfer);

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1)
      g_ShadowMapCopy.Release();
      g_ShadowMapCopy.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize, (unsigned int)g_ShadowMapSize, 1, 6, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, true, 0, NULL, g_agsContext, g_ShadowMapCopyCfxTransfer);
#endif
    }

    if (g_HUD.m_GUI.GetRadioButton( IDC_RADIO_SHADOW_MAP_T2D )->GetChecked())
    {
      g_ShadowMap.Release();
      g_ShadowMap.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleW, (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleH, 1, 1, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, NULL, g_ShadowMapCfxTransfer);

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1)
      g_ShadowMapCopy.Release();
      g_ShadowMapCopy.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleW, (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleH, 1, 1, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, g_agsContext, g_ShadowMapCopyCfxTransfer);
#endif
    }

  break;

  case IDC_TOGGLEFULLSCREEN:
    DXUTToggleFullScreen();
    break;

  case IDC_TOGGLEREF:
    DXUTToggleREF();
    break;

  case IDC_CHANGEDEVICE:
    g_SettingsDlg.SetActive( !g_SettingsDlg.IsActive() );
    break;

  case IDC_SLIDER_SUN_SIZE:
    g_SunSize = (float)(((CDXUTSlider*)pControl)->GetValue());
    swprintf_s( szTemp, L"Sun Width : %.2f", g_SunSize );
    g_HUD.m_GUI.GetStatic( IDC_STATIC_SUN_SIZE )->SetText( szTemp );
    break;

  case IDC_SLIDER_DEPTH_OFFSET_TEST:
    g_DepthTestOffset = (((CDXUTSlider*)pControl)->GetValue()) * 0.00001f;
    swprintf_s( szTemp, L"Depth Test Offset : %.5f", g_DepthTestOffset );
    g_HUD.m_GUI.GetStatic( IDC_STATIC_DEPTH_TEST_OFFSET )->SetText( szTemp );
    break;

  case IDC_SLIDER_NORMAL_OFFSET_SCALE:
    g_NormalOffsetScale = (((CDXUTSlider*)pControl)->GetValue()) * 0.001f;
    swprintf_s( szTemp, L"Normal Offset Scale : %.3f", g_NormalOffsetScale );
    g_HUD.m_GUI.GetStatic( IDC_STATIC_NORMAL_OFFSET_SCALE )->SetText( szTemp );
    break;

  case IDC_COMBOBOX_SHADOWMAP_SIZE:
    g_ShadowMapSize = (float) (1 << ( ((CDXUTComboBox*)pControl)->GetSelectedIndex() + 9));

    for (int light = 0; light < CUBE_FACE_COUNT; light++)
    {
      g_LightData[light].m_BackBufferDim = float2(g_ShadowMapSize, g_ShadowMapSize);
      g_LightData[light].m_BackBufferDimRcp = float2(1.0f / g_ShadowMapSize, 1.0f / g_ShadowMapSize);
    }
    
    if (g_HUD.m_GUI.GetRadioButton( IDC_RADIO_SHADOW_MAP_T2DA )->GetChecked())
    {
      g_ShadowMap.Release();
      g_ShadowMap.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize, (unsigned int)g_ShadowMapSize, 1, 6, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, true, 0, NULL, NULL, g_ShadowMapCfxTransfer);

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1)
      g_ShadowMapCopy.Release();
      g_ShadowMapCopy.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize, (unsigned int)g_ShadowMapSize, 1, 6, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, true, 0, NULL, g_agsContext, g_ShadowMapCopyCfxTransfer);
#endif
    }

    if (g_HUD.m_GUI.GetRadioButton( IDC_RADIO_SHADOW_MAP_T2D )->GetChecked())
    {
      g_ShadowMap.Release();
      g_ShadowMap.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleW, (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleH, 1, 1, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_D32_FLOAT,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, NULL, g_ShadowMapCfxTransfer);

#if (EXPERIMENTAL_EMULATE_GPU_2_STEP_COPY == 1)
      g_ShadowMapCopy.Release();
      g_ShadowMapCopy.CreateSurface(DXUTGetD3D11Device(),
        (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleW, (unsigned int)g_ShadowMapSize * g_ShadowMapAtlasScaleH, 1, 1, 1,
        DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_R32_FLOAT,
		DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_UNKNOWN,
        D3D11_USAGE_DEFAULT, false, 0, NULL, g_agsContext, g_ShadowMapCopyCfxTransfer);
#endif
    }

    break;
  }
  // Call the MagnifyTool gui event handler
  g_MagnifyTool.OnGUIEvent( nEvent, nControlID, pControl, pUserContext );
}