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


//--------------------------------------------------------------------------------------
// Constant buffer
//--------------------------------------------------------------------------------------

//=================================================================================================================================
// Pixel shader input structure
//=================================================================================================================================

struct S_MODEL_DATA
{
  float4x4    m_World;
  float4x4    m_WorldViewProjection;
  float4x4    m_WorldViewProjectionLight;
  float4      m_Diffuse;
  float4      m_Ambient;
  float4      m_Parameter0;
};

struct S_CAMERA_DATA
{
  float4x4    m_View;
  float4x4    m_Projection;
  float4x4    m_ViewInv;
  float4x4    m_ProjectionInv;
  float4x4    m_ViewProjection;
  float4x4    m_ViewProjectionInv;

  float4      m_BackBufferDim;
  float4      m_Color;

  float4      m_Eye;
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

cbuffer CB_MODEL_DATA  : register( b0 )
{
  S_MODEL_DATA  g_Model;
}

cbuffer CB_VIEWER_DATA : register( b1 )
{
  S_CAMERA_DATA g_Viewer;
}

cbuffer CB_LIGHT_ARRAY_DATA  : register( b2 )
{
  S_CAMERA_DATA g_Light[6];
}

//--------------------------------------------------------------------------------------
// Buffers, Textures and Samplers
//--------------------------------------------------------------------------------------

// Textures
Texture2D               g_t2dDiffuse         : register( t0 );
Texture2D<float4>       g_t2dShadowMask      : register( t1 );
Texture2D<float>        g_t2dDepth           : register( t2 );

// Samplers
SamplerState            g_SampleLinear      : register( s0 );

//--------------------------------------------------------------------------------------
// Shader structures
//--------------------------------------------------------------------------------------

struct VS_RenderSceneInput
{
  float3 f3Position   : POSITION;
  float3 f3Normal     : NORMAL;
  float2 f2TexCoord   : TEXCOORD;
  float3 f3Tangent   : TANGENT;
};

struct PS_RenderSceneInput
{
  float4 f4Position     : SV_Position;
  float3 f3PositionWS   : WS_POSITION;
  float3 f3Normal       : NORMAL;
  float2 f2TexCoord     : TEXCOORD0;
};

struct PS_RenderOutput
{
  float4 m_Color      : SV_Target0;
};

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_RenderSceneInput VS_RenderScene( VS_RenderSceneInput I )
{
  PS_RenderSceneInput O;

  // Transform the position from object space to homogeneous projection space
  O.f4Position = mul( float4( I.f3Position, 1.0f ), g_Model.m_WorldViewProjection );
  O.f3PositionWS = mul( float4( I.f3Position, 1.0f ), g_Model.m_World );

  // Transform the normal from object space to world space
  O.f3Normal = normalize( mul( I.f3Normal, (float3x3)g_Model.m_World ) );

  // Pass through texture coords
  O.f2TexCoord = I.f2TexCoord;

  return O;
}


//--------------------------------------------------------------------------------------
// Render Shadow Map
// Adjusts World Space Position by offsetting it along the normal
// This is done to reduce shadow acne when generating shadow masks
//--------------------------------------------------------------------------------------
PS_RenderSceneInput VS_RenderShadowMap( VS_RenderSceneInput I )
{
  PS_RenderSceneInput O;

  // Transform the position from object space to homogeneous projection space
  O.f4Position = mul( float4( I.f3Position, 1.0f ), g_Model.m_WorldViewProjection );

  // Transform the normal from object space to world space
  O.f3Normal = normalize( mul( I.f3Normal, (float3x3)g_Model.m_World ) );

  // Pass through texture coords
  O.f2TexCoord = I.f2TexCoord;

  return O;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
void PS_DepthPassScene( PS_RenderSceneInput I )
{
  float4 color = g_t2dDiffuse.Sample( g_SampleLinear, I.f2TexCoord );

  if (color.a <= 0.50) discard;

  return;
}


float4 PS_DepthAndNormalPassScene( PS_RenderSceneInput I ) : SV_Target0
{
  float4 color = g_t2dDiffuse.Sample( g_SampleLinear, I.f2TexCoord );

  if (color.a <= 0.50) discard;

  return float4( I.f3Normal, 1.0f );
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
float4 PS_RenderShadowMap( PS_RenderSceneInput I ) : SV_Target0
{
  float4 color = g_t2dDiffuse.Sample( g_SampleLinear, I.f2TexCoord );

  if (color.a <= 0.50) discard;

  return color * g_Viewer.m_Color;
}

//--------------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------------
PS_RenderOutput PS_RenderShadowedScene( PS_RenderSceneInput I )
{
  PS_RenderOutput O;

  float4 f4TextureColor = g_t2dDiffuse.Sample( g_SampleLinear, I.f2TexCoord );

  if (f4TextureColor.a <= 0.50) discard;

  float4 shadow = g_t2dShadowMask.Load( int3(I.f4Position.xy, 0 ) ) * 0.4 + 0.6;

  float3 normal = normalize( I.f3Normal );

  float3 LightDir;
  LightDir = normalize(g_Light[0].m_Eye - I.f3PositionWS);

  float lightness;
  lightness = max(0, dot(normal, LightDir) );

  O.m_Color.xyz  = g_Model.m_Ambient.xyz;
  O.m_Color.xyz += lightness * g_Light[0].m_Color.xyz * g_Model.m_Diffuse.xyz * shadow.x;

  O.m_Color.w    = 1.0f;
  O.m_Color     *= f4TextureColor;

  return O;
}


//--------------------------------------------------------------------------------------
// EOF
//--------------------------------------------------------------------------------------
