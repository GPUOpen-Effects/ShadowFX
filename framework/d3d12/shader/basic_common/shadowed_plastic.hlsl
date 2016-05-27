
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


struct vertex
{
    float3 pos  : POSITION;
    float3 n    : NORMAL;
    float3 t    : TANGENT;
    float2 uv   : TEXCOORD;
    float3 inst_pos  : INSTANCEPOS;
};

struct v2p
{
    float4 pos : SV_POSITION;
    float3 n   : NORMAL;
    float3 p   : VIEW_SPACE_POS;
};

cbuffer plastic_ps_cb : register(b0)
{
    float4   ambient_color;
    float4   diffuse_color;
    float4   specular_color; // shininess in alpha
    float4   light_color[MAX_LIGHT];
    float4   light_pos[MAX_LIGHT]; // in view space
    uint     num_light;
};

cbuffer plastic_vs_cb : register(b1)
{
    float4x4 mvp; // model view projection
    float4x4 mv; // model view (assumed orthogonal)
};

Texture2D sh_mask : register(t0);

v2p vs_main(vertex vx)
{
    float3 pos = vx.pos + vx.inst_pos;
    v2p o;
    o.n = mul(mv, float4(vx.n, 0.f)).xyz;
    o.p = mul(mv, float4(pos, 1.f)).xyz;
    o.pos = mul(mvp, float4(pos, 1.f));
    return o;
}

float4 ps_main(v2p i) : SV_TARGET
{
    float shadow = sh_mask.Load(int3(i.pos.xy , 0)).x;

    float3 n = normalize(i.n);

    float3 diffuse = 0.f;
    float3 specular = 0.f;

    for (uint lid = 0; lid < num_light; ++lid)
    {
        float3 l = normalize(light_pos[lid].xyz - i.p);
        float d = max(dot(l, n), 0.f);
        if (d > 0.f)
        {
            diffuse += d * light_color[lid].xyz;

            float3 v = normalize(-i.p);
            float3 h = (v + l) * .5f;
            float s = max(dot(h, n), 0.f);
            specular += max(pow(s, specular_color.a), 0.f) * light_color[lid].xyz;
        }
    }

	float3 ds = diffuse * diffuse_color.rgb + specular * specular_color.rgb;
    float3 final = ambient_color.rgb + ds * shadow;
    return float4(final, 1.f);
}

