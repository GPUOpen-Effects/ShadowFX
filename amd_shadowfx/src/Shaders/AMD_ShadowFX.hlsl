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

#include "AMD_ShadowFX_Common.hlsl"

#include "../../../amd_lib/shared/d3d11/src/Shaders/AMD_FullscreenPass.hlsl"

//--------------------------------------------------------------------------------------
// SHADOW MAP FILTERING
//--------------------------------------------------------------------------------------

uint transformWorldPositionToCubeFace(float3 ws_position)
{
  float3 cubeTexcoord = normalize(ws_position.xyz - g_cbShadowsData.m_Light[0].m_Camera.m_Position.xyz);

  float maxAxis = max( abs( cubeTexcoord.x ), max( abs( cubeTexcoord.y ), abs( cubeTexcoord.z ) ) );
  uint face = 6;

  [flatten]
  if( maxAxis == abs( cubeTexcoord.x ) )
  {
    face = cubeTexcoord.x > 0 ? 0 : 1;
  }

  [flatten]
  if( maxAxis == abs( cubeTexcoord.y ) )
  {
    face = cubeTexcoord.y > 0 ? 2 : 3;
  }

  [flatten]
  if( maxAxis == abs( cubeTexcoord.z ) )
  {
    face = cubeTexcoord.z > 0 ? 4 : 5;
  }

  return face;
}

PS_ShadowMaskOutput shadowFiltering( PS_FullscreenInput I )
{
  PS_ShadowMaskOutput O;

  // calculate pixel WS POSITION moved slightly along a WS NORMAL
  float3 ws_normal = calculateWorldSpaceNormal(I.position.xy);
  float4 clip_space_position;
  clip_space_position.xy = (I.position.xy * float2(g_cbShadowsData.m_SizeInv.x, -g_cbShadowsData.m_SizeInv.y) - float2(0.5, -0.5))*2.0;
  clip_space_position.z = g_t2dDepth.Load( int3( I.position.xy, 0 ) ).x;
  clip_space_position.w = 1.0;
  float4 world_space_position = calculateWorldSpacePosition(clip_space_position);

  bool continueShadow = true;
  uint active = 0;

#if (AMD_SHADOWFX_EXECUTION == SHADOWFX_EXECUTION_WEIGHTED_AVG)
  float shadow = 0.0f;
#else
  float shadow = 1.0f;
#endif
  
#if (AMD_SHADOWFX_EXECUTION == AMD_SHADOWFX_EXECUTION_UNION || AMD_SHADOWFX_EXECUTION == AMD_SHADOWFX_EXECUTION_CASCADE || AMD_SHADOWFX_EXECUTION == SHADOWFX_EXECUTION_WEIGHTED_AVG)
  for (active = 0; (active < g_cbShadowsData.m_ActiveLightCount) && continueShadow; active++)
#endif
  {
    world_space_position.xyz += ws_normal * g_cbShadowsData.m_Light[active].m_NormalOffsetScale;

#if (AMD_SHADOWFX_EXECUTION == AMD_SHADOWFX_EXECUTION_CUBE)
    active = transformWorldPositionToCubeFace(world_space_position);
#endif

    float4 shadow_space_pos = calculateShadowSpacePosition(world_space_position, g_cbShadowsData.m_Light[active]);
    shadow_space_pos.xy = shadow_space_pos.xy * 0.5  + float2(0.5, 0.5) ;
    shadow_space_pos.y = 1.0 - shadow_space_pos.y;
    shadow_space_pos.z = (shadow_space_pos.z ) * DEPTH_SCALE - DEPTH_BIAS;

    float filteredShadow = 1.0f;

    if (shadow_space_pos.x>=0 && shadow_space_pos.x<=1 &&
      shadow_space_pos.y>=0 && shadow_space_pos.y<=1 &&
      shadow_space_pos.z>=0 && shadow_space_pos.z<=1)
    {

#if (AMD_SHADOWFX_FILTERING == AMD_SHADOWFX_FILTERING_UNIFORM)

# if (AMD_SHADOWFX_TAP_TYPE == AMD_SHADOWFX_TAP_TYPE_FIXED)

#   if (AMD_SHADOWFX_TEXTURE_FETCH == AMD_SHADOWFX_TEXTURE_FETCH_GATHER4) // gather4
      filteredShadow = uniformFixedGather4( shadow_space_pos.xyz, g_cbShadowsData.m_Light[active] );
#   elif (AMD_SHADOWFX_TEXTURE_FETCH == AMD_SHADOWFX_TEXTURE_FETCH_PCF) // pcf
      filteredShadow = uniformFixedPCF( shadow_space_pos.xyz, g_cbShadowsData.m_Light[active] );
#   endif

# elif (AMD_SHADOWFX_TAP_TYPE == AMD_SHADOWFX_TAP_TYPE_POISSON)

#   if (AMD_SHADOWFX_TEXTURE_FETCH == AMD_SHADOWFX_TEXTURE_FETCH_GATHER4) // gather4
      filteredShadow = uniformPoissonGather4( shadow_space_pos.xyz, g_cbShadowsData.m_Light[active] );
#   elif (AMD_SHADOWFX_TEXTURE_FETCH == AMD_SHADOWFX_TEXTURE_FETCH_PCF) // pcf
      filteredShadow = uniformPoissonPCF( shadow_space_pos.xyz, g_cbShadowsData.m_Light[active] );
#   endif
# endif

#elif (AMD_SHADOWFX_FILTERING == AMD_SHADOWFX_FILTERING_CONTACT)

# if (AMD_SHADOWFX_TAP_TYPE == AMD_SHADOWFX_TAP_TYPE_FIXED)

#   if (AMD_SHADOWFX_TEXTURE_FETCH == AMD_SHADOWFX_TEXTURE_FETCH_GATHER4)
      filteredShadow = contactFixedGather4( shadow_space_pos.xyz, g_cbShadowsData.m_Light[active] );
#   elif (AMD_SHADOWFX_TEXTURE_FETCH == AMD_SHADOWFX_TEXTURE_FETCH_PCF)
      filteredShadow = contactFixedPCF( shadow_space_pos.xyz, g_cbShadowsData.m_Light[active] );
#   endif

# elif (AMD_SHADOWFX_TAP_TYPE == AMD_SHADOWFX_TAP_TYPE_POISSON)

#   if (AMD_SHADOWFX_TEXTURE_FETCH == AMD_SHADOWFX_TEXTURE_FETCH_GATHER4)
      filteredShadow = contactPoissonGather4(shadow_space_pos.xyz, g_cbShadowsData.m_Light[active]); // not yet implemented
#   elif (AMD_SHADOWFX_TEXTURE_FETCH == AMD_SHADOWFX_TEXTURE_FETCH_PCF) // pcf
      filteredShadow = contactPoissonPCF(shadow_space_pos.xyz, g_cbShadowsData.m_Light[active]); // not yet implemented
#   endif

# endif

#elif (AMD_SHADOWFX_FILTERING == AMD_SHADOWFX_FILTERING_DEBUG_POINT)

      filteredShadow = pointFilter(shadow_space_pos.xyz, g_cbShadowsData.m_Light[active]);

#endif

#if (AMD_SHADOWFX_EXECUTION == AMD_SHADOWFX_EXECUTION_CASCADE)
      continueShadow = false;
#endif

    }

#if (AMD_SHADOWFX_EXECUTION == SHADOWFX_EXECUTION_WEIGHTED_AVG)
    shadow += filteredShadow * g_cbShadowsData.m_Light[active].m_Weight.x; 
#else
    shadow = min(shadow, filteredShadow);
#endif
  }

  O.shadow = shadow.xxxx;

  return O;
}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
