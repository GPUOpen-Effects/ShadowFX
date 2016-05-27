
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


// required #define
// int MAX_STR_SZ max number of char per string
// float CHAR_WIDTH width of one char in texture space

#define CHAR_HEIGHT 1.f

cbuffer cbuffer_data : register(b0)
{
    float4   scale_bias; // xy bias, zw scale. zw multiplied by 2 in CPU
    float4   u_start[MAX_STR_SZ / 4]; // uv start for each char in the word
};

struct v2p
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};

v2p vs_main(uint id : SV_VertexID)
{
    float2 offsets[6] = 
    {
        float2(0,0), float2(1,0), float2(0,1),
        float2(0,1), float2(1,0), float2(1,1)
    };

    uint gid = id / 6; // global quad id
    uint lid = id % 6; // local id in quad

    float2 uv_start = float2(u_start[gid / 4][gid % 4], 0.f);
    float2 uv = uv_start + offsets[lid] * float2(CHAR_WIDTH, CHAR_HEIGHT);
    uv.y = 1.f - uv.y;

    float2 pos_start = scale_bias.xy + float2(float(gid) * scale_bias.z, 0.f);
    float2 pos = pos_start + offsets[lid] * scale_bias.zw;

    v2p o;
    o.uv = uv;
    o.pos = float4(pos, 0.f, 1.f);
    return o;
}

Texture2D tex : register(t0);
SamplerState smplr : register(s0);

float4 ps_main(v2p i) : SV_TARGET
{
    float3 color = float3(1, 1, 1); // TODO put in cbuffer
    return float4(color, tex.Sample(smplr, i.uv).r);
}

