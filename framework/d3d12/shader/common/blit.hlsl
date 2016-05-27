
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



struct v2p
{
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD;
};

v2p vs_main(uint id : SV_VertexID)
{
    v2p o;
    o.uv = float2(float(id & 1u), float(id >> 1u));
    o.pos = float4(o.uv * 4.f - 1.f, 0.f, 1.f);
    o.uv.y = 1.f - o.uv.y;
    o.uv = o.uv * 2.f;
    return o;
}

Texture2D tex : register(t0);
SamplerState smplr : register(s0);

float4 ps_main(v2p i) : SV_TARGET
{
    return tex.Sample(smplr, i.uv);
}
