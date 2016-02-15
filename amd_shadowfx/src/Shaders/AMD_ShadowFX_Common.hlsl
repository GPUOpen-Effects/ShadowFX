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

#ifndef AMD_SHADOWFX_COMMON_HLSL
#define AMD_SHADOWFX_COMMON_HLSL

// These defines come from the fxc compilation parameters
// The default values are provided to make sure the code is still compiling
// even if these are not specified
#define AMD_SHADOWFX_NORMAL_OPTION_NONE                     0
#define AMD_SHADOWFX_NORMAL_OPTION_CALC_FROM_DEPTH          1
#define AMD_SHADOWFX_NORMAL_OPTION_READ_FROM_SRV            2
#ifndef AMD_SHADOWFX_NORMAL_OPTION
# define AMD_SHADOWFX_NORMAL_OPTION                         0    // Don't use Normal Offset
#endif

#define AMD_SHADOWFX_FILTER_SIZE_7                          7
#define AMD_SHADOWFX_FILTER_SIZE_9                          9
#define AMD_SHADOWFX_FILTER_SIZE_11                         11
#define AMD_SHADOWFX_FILTER_SIZE_13                         13
#define AMD_SHADOWFX_FILTER_SIZE_15                         15
#ifndef AMD_SHADOWFX_FILTER_SIZE
# define AMD_SHADOWFX_FILTER_SIZE                           AMD_SHADOWFX_FILTER_SIZE_11
#endif

#define AMD_SHADOWFX_TEXTURE_FETCH_GATHER4                  0
#define AMD_SHADOWFX_TEXTURE_FETCH_PCF                      1
#ifndef AMD_SHADOWFX_TEXTURE_FETCH
# define AMD_SHADOWFX_TEXTURE_FETCH                         AMD_SHADOWFX_TEXTURE_FETCH_GATHER4
#endif

#define AMD_SHADOWFX_FILTERING_UNIFORM                      0
#define AMD_SHADOWFX_FILTERING_CONTACT                      1
#define AMD_SHADOWFX_FILTERING_DEBUG_POINT                  10
#ifndef AMD_SHADOWFX_FILTERING
# define AMD_SHADOWFX_FILTERING                             AMD_SHADOWFX_FILTERING_POINT
#endif

#define AMD_SHADOWFX_TAP_TYPE_FIXED                         0
#define AMD_SHADOWFX_TAP_TYPE_POISSON                       1
#ifndef AMD_SHADOWFX_TAP_TYPE
# define AMD_SHADOWFX_TAP_TYPE                              AMD_SHADOWFX_TAP_TYPE_FIXED
#endif

#define AMD_SHADOWFX_EXECUTION_UNION                        0
#define AMD_SHADOWFX_EXECUTION_CASCADE                      1
#define AMD_SHADOWFX_EXECUTION_CUBE                         2
#ifndef AMD_SHADOWFX_EXECUTION                              
# define AMD_SHADOWFX_EXECUTION                             AMD_SHADOWFX_EXECUTION_UNION
#endif

#define AMD_SHADOWFX_TEXTURE_2D                             0
#define AMD_SHADOWFX_TEXTURE_2D_ARRAY                       1
#ifndef AMD_SHADOWFX_TEXTURE_TYPE
# define AMD_SHADOWFX_TEXTURE_TYPE                          AMD_SHADOWFX_TEXTURE_2D
#endif

#define AMD_SHADOWS_FILTER_RADIUS                          ( AMD_SHADOWFX_FILTER_SIZE/2 )
#define FS                                                 AMD_SHADOWFX_FILTER_SIZE    // abbreviation for Filter Size
#define FR                                                 AMD_SHADOWS_FILTER_RADIUS  // abbreviation for Filter Radius

#ifndef AMD_SHADOWFX_ACTIVE_LIGHT_COUNT     
# define AMD_SHADOWFX_ACTIVE_LIGHT_COUNT                    6      
#endif

#define DEPTH_BIAS                                         0.0000f
#define DEPTH_SCALE                                        1.0000f

//--------------------------------------------------------------------------------------
// CHS filter
//--------------------------------------------------------------------------------------
#if   (AMD_SHADOWFX_FILTER_SIZE == AMD_SHADOWFX_FILTER_SIZE_7)
#include "AMD_SHADOWFX_FILTER_SIZE_7_FIXED.inc"
#include "AMD_SHADOWFX_FILTER_SIZE_7_POISSON.inc"
#elif (AMD_SHADOWFX_FILTER_SIZE == AMD_SHADOWFX_FILTER_SIZE_9)
#include "AMD_SHADOWFX_FILTER_SIZE_9_FIXED.inc"
#include "AMD_SHADOWFX_FILTER_SIZE_9_POISSON.inc"
#elif (AMD_SHADOWFX_FILTER_SIZE == AMD_SHADOWFX_FILTER_SIZE_11)
#include "AMD_SHADOWFX_FILTER_SIZE_11_FIXED.inc"
#include "AMD_SHADOWFX_FILTER_SIZE_11_POISSON.inc"
#elif (AMD_SHADOWFX_FILTER_SIZE == AMD_SHADOWFX_FILTER_SIZE_13)
#include "AMD_SHADOWFX_FILTER_SIZE_13_FIXED.inc"
#include "AMD_SHADOWFX_FILTER_SIZE_13_POISSON.inc"
#elif (AMD_SHADOWFX_FILTER_SIZE == AMD_SHADOWFX_FILTER_SIZE_15)
#include "AMD_SHADOWFX_FILTER_SIZE_15_FIXED.inc"
#include "AMD_SHADOWFX_FILTER_SIZE_15_POISSON.inc"
#endif

struct Camera
{
  float4x4                                                 m_View;
  float4x4                                                 m_Projection;
  float4x4                                                 m_ViewProjection;
  float4x4                                                 m_View_Inv;
  float4x4                                                 m_Projection_Inv;
  float4x4                                                 m_ViewProjection_Inv;
  float3                                                   m_Position;
  float                                                    m_Fov;
  float3                                                   m_Direction;
  float                                                    m_FarPlane;
  float3                                                   m_Right;
  float                                                    m_NearPlane;
  float3                                                   m_Up;
  float                                                    m_Aspect;
  float4                                                   m_Color;
};

struct ShadowsLightData
{
  Camera                                                   m_Camera;
  float2                                                   m_Size;
  float2                                                   m_SizeInv;
  float4                                                   m_Region;

  float                                                    m_SunArea;
  float                                                    m_DepthTestOffset;
  float                                                    m_NormalOffsetScale;
  uint                                                     m_ArraySlice;
};

struct ShadowsData
{
  Camera                                                   m_Viewer;
  float2                                                   m_Size; // Viewer Depth Buffer Size
  float2                                                   m_SizeInv; // Viewer Depth Buffer inverse Size

  ShadowsLightData                                         m_Light[AMD_SHADOWFX_ACTIVE_LIGHT_COUNT];
  uint                                                     m_ActiveLightCount;
  uint                                                     _pad[3];
};

Texture2D<float>                                           g_t2dDepth              : register( t0 );
Texture2D<float4>                                          g_t2dNormal             : register( t1 );

#if (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D)
Texture2D<float>                                           g_t2dShadow             : register( t2 );
#elif (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D_ARRAY)
Texture2DArray<float>                                      g_t2dShadow             : register( t2 );
#endif

SamplerState                                               g_ssPoint               : register( s0 );
SamplerState                                               g_ssLinear              : register( s1 );
SamplerComparisonState                                     g_scsPoint              : register( s2 );
SamplerComparisonState                                     g_scsLinear             : register( s3 );

cbuffer CB_SHADOWS_DATA                                                            : register( b0 )
{ 
  ShadowsData                                              g_cbShadowsData;
}

struct PS_ShadowMaskOutput
{
  float4                                                   shadow                  : SV_Target0;
};

//--------------------------------------------------------------------------------------
// Utility functions 
//--------------------------------------------------------------------------------------
float shadowSample(SamplerState samplerState, float2 uv, uint slice)
{
#if (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D)
  return g_t2dShadow.SampleLevel(samplerState, uv, 0);
#elif (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D_ARRAY)
  return g_t2dShadow.SampleLevel(samplerState, float3(uv, slice), 0);
#endif
}

float shadowSampleCmp(SamplerComparisonState samplerCmpState, float2 uv, float z, uint slice)
{
#if (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D)
  return g_t2dShadow.SampleCmpLevelZero(samplerCmpState, uv, z);
#elif (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D_ARRAY)
  return g_t2dShadow.SampleCmpLevelZero(samplerCmpState, float3(uv, slice), z);
#endif
}

float4 shadowGather(SamplerState samplerState, float2 uv, uint slice)
{
#if (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D)
  return g_t2dShadow.GatherRed(samplerState, uv);
#elif (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D_ARRAY)
  return g_t2dShadow.GatherRed(samplerState, float3(uv, slice));
#endif
}

float4 shadowGatherCmp(SamplerComparisonState samplerCmpState, float2 uv, float z, uint slice)
{
#if (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D)
  return g_t2dShadow.GatherCmpRed(samplerCmpState, uv, z);
#elif (AMD_SHADOWFX_TEXTURE_TYPE == AMD_SHADOWFX_TEXTURE_2D_ARRAY)
  return g_t2dShadow.GatherCmpRed(samplerCmpState, float3(uv, slice), z);
#endif
}

float4 transformPositionWithProjection(float4 position, float4x4 m)
{
  float4 m_position = mul(position, m);
  return m_position / m_position.w;
}

float4 calculateWorldSpacePosition(float4 cs_position)
{
  return transformPositionWithProjection(cs_position, g_cbShadowsData.m_Viewer.m_ViewProjection_Inv);
}

float4 calculateShadowSpacePosition(float4 ws_position, ShadowsLightData lightData)
{
  return transformPositionWithProjection(ws_position, lightData.m_Camera.m_ViewProjection);
}

float3 calculateWorldSpaceNormal(float2 uv)
{
  float3 ws_normal;

# if (AMD_SHADOWFX_NORMAL_OPTION == AMD_SHADOWFX_NORMAL_OPTION_NONE)
  ws_normal = float3(0.0f, 0.0f, 0.0f);
# endif

# if (AMD_SHADOWFX_NORMAL_OPTION == AMD_SHADOWFX_NORMAL_OPTION_CALC_FROM_DEPTH)
  float4 cs_position;
  float cs_depth = g_t2dDepth.Load( int3( uv, 0 ) ).x;

  float4 extra_cs_depth;
  extra_cs_depth.x = g_t2dDepth.Load( int3( uv + float2(1,0), 0 ) );
  extra_cs_depth.y = g_t2dDepth.Load( int3( uv - float2(1,0), 0 ) );
  extra_cs_depth.z = g_t2dDepth.Load( int3( uv + float2(0,1), 0 ) );
  extra_cs_depth.w = g_t2dDepth.Load( int3( uv - float2(0,1), 0 ) );

  float2 dd_depth;
  float2 dd_offset = float2(0,0);

  dd_depth.x  = abs(cs_depth-extra_cs_depth.x) < abs(cs_depth-extra_cs_depth.y) ? extra_cs_depth.x : extra_cs_depth.y;
  dd_offset.x = abs(cs_depth-extra_cs_depth.x) < abs(cs_depth-extra_cs_depth.y) ?  1  : -1;
  dd_depth.y  = abs(cs_depth-extra_cs_depth.z) < abs(cs_depth-extra_cs_depth.w) ? extra_cs_depth.z : extra_cs_depth.w;
  dd_offset.y = abs(cs_depth-extra_cs_depth.z) < abs(cs_depth-extra_cs_depth.w) ?  1  : -1;

  // calculate DY WS POSITION
  cs_position.xy = ((uv+float2(0,dd_offset.y)) * float2(g_cbShadowsData.m_SizeInv.x, -g_cbShadowsData.m_SizeInv.y) - float2(0.5, -0.5))*2.0;
  cs_position.z = dd_depth.y;
  cs_position.w = 1.0;
  float4 ws_position_dy = calculateWorldSpacePosition(cs_position);

  // calculate DX WS POSITION
  cs_position.xy = ((uv+float2(dd_offset.x,0)) * float2(g_cbShadowsData.m_SizeInv.x, -g_cbShadowsData.m_SizeInv.y) - float2(0.5, -0.5))*2.0;
  cs_position.z = dd_depth.x;
  cs_position.w = 1.0;
  float4 ws_position_dx = calculateWorldSpacePosition(cs_position);

  // calculate CENTRAL WS POSITION
  cs_position.xy = (uv * float2(g_cbShadowsData.m_SizeInv.x, -g_cbShadowsData.m_SizeInv.y) - float2(0.5, -0.5))*2.0;
  cs_position.z = cs_depth;
  cs_position.w = 1.0;
  float4 ws_position = calculateWorldSpacePosition(cs_position);

  // calculate NORMAL
  float3 ddx_ws_position = -ws_position.xyz * dd_offset.x + ws_position_dx.xyz * dd_offset.x;
  float3 ddy_ws_position = -ws_position.xyz * dd_offset.y + ws_position_dy.xyz * dd_offset.y;
  //ws_normal = normalize( cross( ddx(ws_position.xyz), ddy(ws_position.xyz) ) );
  ws_normal = normalize(cross(ddx_ws_position, ddy_ws_position));
# endif

# if (AMD_SHADOWFX_NORMAL_OPTION == AMD_SHADOWFX_NORMAL_OPTION_READ_FROM_SRV)
  ws_normal = normalize( g_t2dNormal.Load( int3( uv, 0 ) ).xyz  * 2.0f - (1.0f).xxx );
# endif

  return ws_normal;
}

//--------------------------------------------------------------------------------------
// UNIFORM shadow filtering 
//--------------------------------------------------------------------------------------
float uniformFixedGather4( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
#if (AMD_SHADOWFX_TEXTURE_TYPE == 1)
  shadowRegion.xy = float2(1, 1);
  shadowRegion.zw = float2(0, 0);
#else
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;
#endif

  // calculate integer and fractional parts of shadow space texture coordinate
  // discard the fractional part of shadow space texture coordinate
  float3 shadowUVZ = float3(shadowSpaceCoord.xy, shadowSpaceCoord.z - lightData.m_DepthTestOffset);
  float2 shadowTextureCoord = (( lightData.m_Size.xy * shadowUVZ.xy ) + float2( 0.5, 0.5 ));
  float2 shadowTextureCoordFloor = floor( shadowTextureCoord );
  float2 shadowTextureCoordFrac = frac( shadowTextureCoord );
  shadowUVZ.xy = shadowTextureCoordFloor * lightData.m_SizeInv.xy * shadowRegion.xy + shadowRegion.zw;

  float  accumulatedShadow = 0.0f;

  for( int row = -AMD_SHADOWS_FILTER_RADIUS; row <= AMD_SHADOWS_FILTER_RADIUS; row += 2 )
  {
    [unroll]for( int col = -AMD_SHADOWS_FILTER_RADIUS; col <= AMD_SHADOWS_FILTER_RADIUS; col += 2 )
    {
      float4 shadow = shadowGatherCmp( g_scsPoint, shadowUVZ.xy + float2(col, row) * lightData.m_SizeInv.xy * shadowRegion.xy, shadowUVZ.z, lightData.m_ArraySlice );

      float4 filter_weight = float4(1,1,1,1);
      // this is Edge Tap Smoothing as described in
      // http://developer.amd.com/wordpress/media/2012/10/Isidoro-ShadowMapping.pdf
      // slides 10 - 11
      // for shadow results on the border of filter window
      // weights are selected the same way as for the bilinear interpolation
      if( row == -AMD_SHADOWS_FILTER_RADIUS )
      {
        filter_weight.z *= 1.0-shadowTextureCoordFrac.y;
        filter_weight.w *= 1.0-shadowTextureCoordFrac.y;
      }
      if( row == +AMD_SHADOWS_FILTER_RADIUS )
      {
        filter_weight.x *= shadowTextureCoordFrac.y;
        filter_weight.y *= shadowTextureCoordFrac.y;
      }
      if( col == -AMD_SHADOWS_FILTER_RADIUS )
      {
        filter_weight.x *= 1-shadowTextureCoordFrac.x;
        filter_weight.w *= 1-shadowTextureCoordFrac.x;
      }
      if( col == +AMD_SHADOWS_FILTER_RADIUS )
      {
        filter_weight.y *= shadowTextureCoordFrac.x;
        filter_weight.z *= shadowTextureCoordFrac.x;
      }

      accumulatedShadow += dot(filter_weight, shadow);
    }
  }

  return accumulatedShadow*(1.0f/(FS*FS)); // FS is AMD_SHADOWFX_FILTER_SIZE
}

float uniformFixedPCF( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;
  float3 shadowUVZ = float3(shadowSpaceCoord.xy * shadowRegion.xy + shadowRegion.zw, shadowSpaceCoord.z - lightData.m_DepthTestOffset);

  float  accumulatedShadow = 0.0f;

  for( float row = -AMD_SHADOWS_FILTER_RADIUS; row <= AMD_SHADOWS_FILTER_RADIUS; row += 1 )
  {
    [unroll]for( float col = -AMD_SHADOWS_FILTER_RADIUS; col <= AMD_SHADOWS_FILTER_RADIUS; col += 1 )
    {
      float shadow = shadowSampleCmp( g_scsLinear, shadowUVZ.xy + float2(col, row) * lightData.m_SizeInv.xy * shadowRegion.xy, shadowUVZ.z, lightData.m_ArraySlice );
      accumulatedShadow += shadow;
    }
  }

  return accumulatedShadow*(1.0f/(FS*FS));
}

float uniformPoissonGather4( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;

  float accumulatedShadow = 0.0f;
  float accumulatedWeight = 0.0f;

  [unroll]
  for( uint i = 0; i < g_PoissonSamplesCount; i += 1.0f )
  {
    float weight = exp( -((g_PoissonSamples[i].x*g_PoissonSamples[i].x) + (g_PoissonSamples[i].y*g_PoissonSamples[i].y)) / (FR*FR) );

    // calculate integer and fractional parts of shadow space texture coordinate
    float3 shadowUVZ = float3(shadowSpaceCoord.xy, shadowSpaceCoord.z - lightData.m_DepthTestOffset);
    float2 shadowTextureCoord       = (( lightData.m_Size.xy * shadowSpaceCoord.xy ) + float2( 0.5, 0.5 ) + g_PoissonSamples[i].xy);
    float2 shadowTextureCoordFloor = floor( shadowTextureCoord );
    float2 shadowTextureCoordFrac  = frac( shadowTextureCoord );
    // discard the fractional part of shadow space texture coordinate
    shadowUVZ.xy = shadowTextureCoordFloor * lightData.m_SizeInv.xy * shadowRegion.xy + shadowRegion.zw;

    float4 shadow = shadowGatherCmp( g_scsPoint, shadowUVZ.xy, shadowUVZ.z, lightData.m_ArraySlice );

    float4 filter_weight = float4(1,1,1,1);
    // this is Edge Tap Smoothing as described in
    // http://developer.amd.com/wordpress/media/2012/10/Isidoro-ShadowMapping.pdf
    // slides 10 - 11
    // for shadow results on the border of filter window
    // weights are selected the same way as for the bilinear interpolation
    if( g_PoissonSamples[i].y == -AMD_SHADOWS_FILTER_RADIUS )
    {
      filter_weight.z *= 1.0-shadowTextureCoordFrac.y;
      filter_weight.w *= 1.0-shadowTextureCoordFrac.y;
    }
    if( g_PoissonSamples[i].y == +AMD_SHADOWS_FILTER_RADIUS )
    {
      filter_weight.x *= shadowTextureCoordFrac.y;
      filter_weight.y *= shadowTextureCoordFrac.y;
    }
    if( g_PoissonSamples[i].x == -AMD_SHADOWS_FILTER_RADIUS )
    {
      filter_weight.x *= 1-shadowTextureCoordFrac.x;
      filter_weight.w *= 1-shadowTextureCoordFrac.x;
    }
    if( g_PoissonSamples[i].x == +AMD_SHADOWS_FILTER_RADIUS )
    {
      filter_weight.y *= shadowTextureCoordFrac.x;
      filter_weight.z *= shadowTextureCoordFrac.x;
    }

    accumulatedShadow += dot(filter_weight, shadow) * weight;
    accumulatedWeight += dot(filter_weight, weight);
  }

  return accumulatedShadow * ( 1.0f / accumulatedWeight) ;
}

float uniformPoissonPCF( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float accumulatedShadow = 0.0f;
  float accumulatedWeight = 0.0f;

  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;
  float3 shadowUVZ = float3(shadowSpaceCoord.xy * shadowRegion.xy + shadowRegion.zw, shadowSpaceCoord.z - lightData.m_DepthTestOffset);
  
  [unroll]
  for( uint i = 0; i < g_PoissonSamplesCount; i += 1 )
  {
    float weight = exp( -((g_PoissonSamples[i].x*g_PoissonSamples[i].x) + (g_PoissonSamples[i].y*g_PoissonSamples[i].y)) / (FR*FR) );
    float shadow = shadowSampleCmp( g_scsLinear, shadowUVZ.xy + g_PoissonSamples[i].xy * lightData.m_SizeInv.xy * shadowRegion.xy, shadowUVZ.z, lightData.m_ArraySlice );

    accumulatedShadow += shadow * weight;
    accumulatedWeight += weight;
  }

  return accumulatedShadow * ( 1.0f / accumulatedWeight ) ;
}

//--------------------------------------------------------------------------------------
// CONTACT shadow filtering
//--------------------------------------------------------------------------------------
float percentageCloserSoftShadows(float depth, float blockerDepth, float sunWidth)
{
  // compute ratio using formulas from PCSS article http://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
  return pow( saturate( (depth - blockerDepth) * sunWidth / blockerDepth ), 0.5);
}

float cubicBezierCurve(float v1, float v2, float v3, float v4, float t)
{
  return (1.0-t)*(1.0-t)*(1.0-t) * v1 + 3.0f*(1.0-t)*(1.0-t)*t * v2 + 3.0f*t*t*(1.0-t) * v3 + t*t*t * v4;
}

float fetchFilterWeight( int r, int c, float ratio )
{
  if (r < 0 || r >= FS) return 0.0f;
  if (c < 0 || c >= FS) return 0.0f;
  return cubicBezierCurve(g_lowGaussianWeight[r][c], g_mediumGaussianWeight[r][c], g_highGaussianWeight[r][c], g_ultraGaussianWeight[r][c], ratio);
}

float calculateFilterWeight( float x, float y, float ratio )
{
  if (x < -FS || x > FS) return 0.0f;
  if (y < -FS || y > FS) return 0.0f;

  float sigma = (FS - 1) * 0.5f;

  float lowGaussianWeight = exp( -(x*x + y*y) / (0.25*sigma * 0.25*sigma) );
  float mediumGaussianWeight = exp( -(x*x + y*y) / (0.5*sigma * 0.5*sigma) );
  float highGaussianWeight = exp( -(x*x + y*y) / (0.75*sigma * 0.75*sigma) );
  float ultraGaussianWeight = exp( -(x*x + y*y) / (sigma * sigma) );

  return cubicBezierCurve(lowGaussianWeight, mediumGaussianWeight, highGaussianWeight, ultraGaussianWeight, ratio);
}

float filterWeightSum(float ratio)
{
  float weight = 0.0f;
  float C[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

  // sum up weights of dynamic filter matrix
  // this should be unrolled by compiler and become 4 literal constants
  [unroll]for( int row = 0; row < FS; ++row )
  {
    [unroll]for( int col = 0; col < FS; ++col )
    {
      C[0] += g_lowGaussianWeight[row][col];
      C[1] += g_mediumGaussianWeight[row][col];
      C[2] += g_highGaussianWeight[row][col];
      C[3] += g_ultraGaussianWeight[row][col];
    }
  }

  float weightSum = cubicBezierCurve(C[0], C[1], C[2], C[3], ratio);
  return weightSum;
}

float2 uniformBlockerSearch( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;

  float2 blockerInfo = { 0.0f, 0.0f };

  [loop]for (int row = -BFR; row <= BFR; row += 2 ) // BFR is Blocker Filter Size
  {
    [loop]for(int col = -BFR; col <= BFR; col += 2 )
    {
      float4 depth = shadowGather( g_ssPoint, shadowSpaceCoord.xy + float2(col, row) * lightData.m_SizeInv.xy * shadowRegion.xy, lightData.m_ArraySlice);
      float4 blocker = ( (shadowSpaceCoord.z).xxxx <= depth ) ? (0.0).xxxx : (1.0).xxxx;
      blockerInfo.x += dot( blocker, (1.0).xxxx );
      blockerInfo.y += dot( depth, blocker );
    }
  }

  if (blockerInfo.x > 0.0)
    blockerInfo.y /= blockerInfo.x;

  blockerInfo.x /= ((BFS+1)*(BFS+1));

  return blockerInfo;
}

float2 poissonBlockerSearch( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;

  float2 blockerInfo = { 0.0f, 0.0f };

  [unroll]for (uint i = 0; i < g_PoissonSamplesCount; i++) 
  {
    float4 depth = shadowGather( g_ssPoint, shadowSpaceCoord.xy + g_PoissonSamples[i].xy * lightData.m_SizeInv.xy * shadowRegion.xy, lightData.m_ArraySlice);
    float4 blocker = ( (shadowSpaceCoord.z).xxxx <= depth ) ? (0.0).xxxx : (1.0).xxxx;
    blockerInfo.x += dot( blocker, (1.0).xxxx );
    blockerInfo.y += dot( depth, blocker );
  }

  if (blockerInfo.x > 0.0)
    blockerInfo.y /= blockerInfo.x;

  blockerInfo.x /= (g_PoissonSamplesCount * 4.0f); // each jittered sample actually fetches 4 samples through gather4

  return blockerInfo;
}

float contactFixedGather4( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;

  float3 shadowUVZ = float3(shadowSpaceCoord.xy, shadowSpaceCoord.z - lightData.m_DepthTestOffset);
  float2 shadowTextureCoord = (( lightData.m_Size.xy * shadowUVZ.xy ) + float2( 0.5, 0.5 ));
  float2 shadowTextureCoordFrac = frac(shadowTextureCoord);
  float2 shadowTextureCoordFloor = floor(shadowTextureCoord);
  shadowUVZ.xy = shadowTextureCoordFloor * lightData.m_SizeInv.xy * shadowRegion.xy + shadowRegion.zw;
  float4 linearWeight = float4(shadowTextureCoordFrac.x, shadowTextureCoordFrac.y, 1.0f-shadowTextureCoordFrac.x, 1.0f-shadowTextureCoordFrac.y );

  float2 blockerInfo = uniformBlockerSearch(shadowUVZ, lightData); // TODO: OPTIMIZE THIS WITH HiZ ACCESS
  float  blockerCount = blockerInfo.x;
  float  blockerDepth = blockerInfo.y;

  // compute ratio using formulas from PCSS article http://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
  float  ratio = 0.0f;
  if( blockerCount > 0.0 && 
      blockerCount < 1.0f )
  {
    ratio = percentageCloserSoftShadows( shadowSpaceCoord.z, blockerDepth, lightData.m_SunArea); 
  }
  else // Early out - fully lit or fully in shadow depends on blockerCount
  {
    return 1.0f - blockerCount;
  }

  /* filter shadow map samples using the dynamic weights */
  float  shadowSum = 0.0f;
  
  // because we'll be using gather4, each fetch provides us with 2 rows and 2 columns
  // current  depth row Top :     X Y X Y X Y ...
  // current  depth row Bottom:   W X W Z W Z ...
  // previous depth row :         X Y X Y X Y ...
  float4 currDepthRow[ FR + 1 ];
  float2 prevDepthRow[ FR + 1 ];
  for (int i = 0; i < FR+1; i++) prevDepthRow[i] = float2(0.0f, 0.0f); // without this initialization fxc won't compile the shader

  [unroll(FS)]for( int row = -FR; row <= FR; row += 2 ) // because we are using gather4 each iteration actually processes 2 rows
  {
    // rIdx is a Row Index into an array of filter weights {g_lowGaussianWeight, g_mediumGaussianWeight, g_highGaussianWeight, g_ultraGaussianWeight}
    uint rIdx = row + FR; 

    [unroll]for( int col = -FR; col <= FR; col += 2 ) // because we are using gather4 each iteration actually processes 2 columns
    {
      // cIdx is a Column Index into an array of filter weights {g_lowGaussianWeight, g_mediumGaussianWeight, g_highGaussianWeight, g_ultraGaussianWeight}
      uint cIdx = col + FR; 
      // dIdx is a Depth Index into an array of stored depth values (prevDepthRow and currDepthRow), 
      uint dIdx = cIdx / 2; 

      // on each row iteration, depth values are fetched and stored inside currDepthRow array
      // because of gather 4 we are actually sampling 2x2 depth values, so 2 rows and 2 columns at a time
      currDepthRow[dIdx] = shadowGatherCmp( g_scsPoint, shadowUVZ.xy + float2(col, row) * lightData.m_SizeInv.xy * shadowRegion.xy, shadowUVZ.z, lightData.m_ArraySlice );

      uint lCol = cIdx - 1;
      uint rCol = cIdx + 1;
      uint bRow = rIdx - 1;

      // accumulate filtered shadow between the two rows that were fetched on current iteration via gather 4
      shadowSum += linearWeight.w * ( currDepthRow[dIdx].w * ( linearWeight.x * fetchFilterWeight(rIdx, lCol, ratio) + linearWeight.z * fetchFilterWeight(rIdx, cIdx, ratio) ) + currDepthRow[dIdx].z * ( linearWeight.x * fetchFilterWeight(rIdx, cIdx, ratio) + linearWeight.z * fetchFilterWeight(rIdx, rCol, ratio) ) );
      shadowSum += linearWeight.y * ( currDepthRow[dIdx].x * ( linearWeight.x * fetchFilterWeight(rIdx, lCol, ratio) + linearWeight.z * fetchFilterWeight(rIdx, cIdx, ratio) ) + currDepthRow[dIdx].y * ( linearWeight.x * fetchFilterWeight(rIdx, cIdx, ratio) + linearWeight.z * fetchFilterWeight(rIdx, rCol, ratio) ) );
      
      // additionally accumulate filtered shadow between the bottom row that was fetched on current iteration and the top row from previous iteration
      shadowSum += linearWeight.w * ( prevDepthRow[dIdx].x * ( linearWeight.x * fetchFilterWeight(bRow, lCol, ratio) + linearWeight.z * fetchFilterWeight(bRow, cIdx, ratio) ) + prevDepthRow[dIdx].y * ( linearWeight.x * fetchFilterWeight(bRow, cIdx, ratio) + linearWeight.z * fetchFilterWeight(bRow, rCol, ratio) ) );
      shadowSum += linearWeight.y * ( currDepthRow[dIdx].w * ( linearWeight.x * fetchFilterWeight(bRow, lCol, ratio) + linearWeight.z * fetchFilterWeight(bRow, cIdx, ratio) ) + currDepthRow[dIdx].z * ( linearWeight.x * fetchFilterWeight(bRow, cIdx, ratio) + linearWeight.z * fetchFilterWeight(bRow, rCol, ratio) ) );

      // once the row is filtered, copy the top row from current iteration into prevDepthRow array 
      // this top row will be used on the next row iteration
      if( row != FR ) 
      {
        prevDepthRow[dIdx] = currDepthRow[dIdx].xy; 
      }
    }
  }

  return shadowSum / filterWeightSum(ratio);
}

float contactFixedPCF( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;

  float3 shadowUVZ = float3(shadowSpaceCoord.xy, shadowSpaceCoord.z - lightData.m_DepthTestOffset);
  float2 shadowTextureCoord = (( lightData.m_Size.xy * shadowUVZ.xy ) + float2( 0.5, 0.5 ));
  float2 shadowTextureCoordFrac = frac(shadowTextureCoord);
  float2 shadowTextureCoordFloor = floor(shadowTextureCoord);
  shadowUVZ.xy = shadowTextureCoordFloor * lightData.m_SizeInv.xy * shadowRegion.xy + shadowRegion.zw;

  float2 blockerInfo = uniformBlockerSearch(shadowUVZ, lightData);
  float blockerCount = blockerInfo.x;
  float blockerDepth = blockerInfo.y;
  
  // compute ratio using formulas from PCSS
  float ratio = 0.0f;
  if( blockerCount > 0.0 && 
      blockerCount < 1.0f )
  {
    ratio = percentageCloserSoftShadows( shadowSpaceCoord.z, blockerDepth, lightData.m_SunArea); 
  }
  else // Early out - fully lit or fully in shadow depends on blockerCount
  {
    return 1.0f - blockerCount;
  }

  float accumulatedShadow = 0.0f;
  float accumulatedWeight = 0.0f;

  shadowUVZ.xy = shadowSpaceCoord.xy * shadowRegion.xy + shadowRegion.zw; 

  for ( float row = -FR; row <= FR; row += 1 )
  {
    [unroll]for( float col = -FR; col <= FR; col += 1 )
    {
      float weight = fetchFilterWeight(row+FR, col+FR, ratio);
      float shadow = shadowSampleCmp( g_scsLinear, shadowUVZ.xy + float2(col, row) * lightData.m_SizeInv.xy * shadowRegion.xy, shadowUVZ.z, lightData.m_ArraySlice );

      accumulatedShadow += shadow * weight;
      accumulatedWeight += weight;
    }
  }

  accumulatedShadow = accumulatedShadow / ( accumulatedWeight );

  return accumulatedShadow; 
}

float contactPoissonGather4( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;

  float3 shadowUVZ = float3(shadowSpaceCoord.xy, shadowSpaceCoord.z - lightData.m_DepthTestOffset);
  float2 shadowTextureCoord = (( lightData.m_Size.xy * shadowUVZ.xy ) + float2( 0.5, 0.5 ));
  float2 shadowTextureCoordFrac = frac(shadowTextureCoord);
  float2 shadowTextureCoordFloor = floor(shadowTextureCoord);
  shadowUVZ.xy = shadowTextureCoordFloor * lightData.m_SizeInv.xy * shadowRegion.xy + shadowRegion.zw;

  float2 blockerInfo = poissonBlockerSearch(shadowUVZ, lightData);
  float blockerCount = blockerInfo.x;
  float blockerDepth = blockerInfo.y;
  
  // compute ratio using formulas from PCSS
  float ratio = 0.0f;
  if( blockerCount > 0.0 && blockerCount < 1.0f ) 
  {
    ratio = percentageCloserSoftShadows( shadowSpaceCoord.z, blockerDepth, lightData.m_SunArea); 
  }
  else // Early out - fully lit or fully in shadow depends on blockerCount
  {
    return 1.0f - blockerCount; 
  }

  float accumulatedShadow = 0.0f;
  float accumulatedWeight = 0.0f;

  [unroll]for (uint i = 0; i < g_PoissonSamplesCount; i++) 
  {
    float2 sample = g_PoissonSamples[i];

    float weight = calculateFilterWeight(g_PoissonSamples[i].x, g_PoissonSamples[i].y, ratio);

    // calculate integer and fractional parts of shadow space texture coordinate
    float3 shadowUVZ = float3(shadowSpaceCoord.xy, shadowSpaceCoord.z - lightData.m_DepthTestOffset);
    shadowTextureCoord     = (( lightData.m_Size.xy * shadowUVZ.xy ) + float2( 0.5, 0.5 ) + g_PoissonSamples[i].xy);
    shadowTextureCoordFrac = frac( shadowTextureCoord );
    shadowTextureCoordFloor = floor( shadowTextureCoord );
    // discard the fractional part of shadow space texture coordinate
    shadowUVZ.xy = shadowTextureCoordFloor * lightData.m_SizeInv.xy * shadowRegion.xy + shadowRegion.zw;

    // the sampling code below is actually INCORRECT because it uses g_PoissonSamples offsets twice (first when shadowUVZ is calculated and second time during the fetch
    float4 shadow = shadowGatherCmp( g_scsPoint, shadowUVZ.xy /*+ g_PoissonSamples[i].xy * lightData.m_SizeInv.xy * shadowRegion.xy*/, shadowUVZ.z, lightData.m_ArraySlice );

    float4 filter_weight = float4(1,1,1,1);
    // this is Edge Tap Smoothing as described in
    // http://developer.amd.com/wordpress/media/2012/10/Isidoro-ShadowMapping.pdf
    // slides 10 - 11
    // for shadow results on the border of filter window
    // weights are selected the same way as for the bilinear interpolation
    if( g_PoissonSamples[i].y == -AMD_SHADOWS_FILTER_RADIUS )
    {
      filter_weight.z *= 1.0-shadowTextureCoordFrac.y;
      filter_weight.w *= 1.0-shadowTextureCoordFrac.y;
    }
    if( g_PoissonSamples[i].y == +AMD_SHADOWS_FILTER_RADIUS )
    {
      filter_weight.x *= shadowTextureCoordFrac.y;
      filter_weight.y *= shadowTextureCoordFrac.y;
    }
    if( g_PoissonSamples[i].x == -AMD_SHADOWS_FILTER_RADIUS )
    {
      filter_weight.x *= 1-shadowTextureCoordFrac.x;
      filter_weight.w *= 1-shadowTextureCoordFrac.x;
    }
    if( g_PoissonSamples[i].x == +AMD_SHADOWS_FILTER_RADIUS )
    {
      filter_weight.y *= shadowTextureCoordFrac.x;
      filter_weight.z *= shadowTextureCoordFrac.x;
    }

    // accumulate filtered shadow between the two rows that were fetched on current iteration via gather 4
    accumulatedShadow += dot(filter_weight, shadow) * weight;
    accumulatedWeight += dot(filter_weight, weight);
  }

  accumulatedShadow = accumulatedShadow / accumulatedWeight;

  return accumulatedShadow; 
}

float contactPoissonPCF( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;

  float3 shadowUVZ = float3(shadowSpaceCoord.xy, shadowSpaceCoord.z - lightData.m_DepthTestOffset);
  float2 shadowTextureCoord = (( lightData.m_Size.xy * shadowUVZ.xy ) + float2( 0.5, 0.5 ));
  float2 shadowTextureCoordFrac = frac(shadowTextureCoord);
  float2 shadowTextureCoordFloor = floor( shadowTextureCoord );
  shadowUVZ.xy = shadowTextureCoordFloor * lightData.m_SizeInv.xy * shadowRegion.xy + shadowRegion.zw;

  float2 blockerInfo = poissonBlockerSearch(shadowUVZ, lightData);
  float blockerCount = blockerInfo.x;
  float blockerDepth = blockerInfo.y;
  
  // compute ratio using formulas from PCSS
  float ratio = 0.0f;
  if( blockerCount > 0.0 && blockerCount < 1.0f) 
  {
    ratio = percentageCloserSoftShadows( shadowSpaceCoord.z, blockerDepth, lightData.m_SunArea); 
  }
  else // Early out - fully lit or fully in shadow depends on blockerCount
  {
    return 1.0f - blockerCount; 
  }

  float accumulatedShadow = 0.0f;
  float accumulatedWeight = 0.0f;

  shadowUVZ.xy = shadowSpaceCoord.xy * shadowRegion.xy + shadowRegion.zw;

  [unroll]
  for( uint i = 0; i < g_PoissonSamplesCount; i += 1 )
  {
    float weight = calculateFilterWeight(g_PoissonSamples[i].x, g_PoissonSamples[i].y, ratio);
    float shadow = shadowSampleCmp( g_scsLinear, shadowUVZ.xy + g_PoissonSamples[i].xy * lightData.m_SizeInv.xy * shadowRegion.xy, shadowUVZ.z, lightData.m_ArraySlice );

    accumulatedShadow += shadow * weight;
    accumulatedWeight += weight;
  }

  accumulatedShadow = accumulatedShadow / accumulatedWeight;

  return accumulatedShadow; 
}

//--------------------------------------------------------------------------------------
// DEBUG shadow filtering
//--------------------------------------------------------------------------------------

float pointFilter( float3 shadowSpaceCoord, ShadowsLightData lightData )
{
  float4 shadowRegion;
  shadowRegion.xy = lightData.m_Region.zw - lightData.m_Region.xy;
  shadowRegion.zw = lightData.m_Region.xy;
  
  shadowSpaceCoord.xy = shadowSpaceCoord.xy * shadowRegion.xy + shadowRegion.zw;
  shadowSpaceCoord.xy = clamp(shadowSpaceCoord.xy, lightData.m_Region.xy, lightData.m_Region.zw);
  
  return ( shadowSpaceCoord.z-lightData.m_DepthTestOffset <= shadowSample( g_ssPoint, shadowSpaceCoord.xy, lightData.m_ArraySlice ) ) ? ( 1.0f ) : ( 0.0f );
}



#endif