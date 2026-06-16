#include "Fullscreen.hlsli"

static const float2 kIndex3x3[3][3] =
{
    { float2(-1.0f, -1.0f), float2(0.0f, -1.0f), float2(1.0f, -1.0f) },
    { float2(-1.0f, 0.0f), float2(0.0f, 0.0f), float2(1.0f, 0.0f) },
    { float2(-1.0f, 1.0f), float2(0.0f, 1.0f), float2(1.0f, 1.0f) }
};

static const float kPrewittHorizontalKernel[3][3] =
{
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f },
    { -1.0f / 6.0f, 0.0f, 1.0f / 6.0f }
};

static const float kPrewittVerticalKernel[3][3] =
{
    { -1.0f / 6.0f, -1.0f / 6.0f, -1.0f / 6.0f },
    { 0.0f, 0.0f, 0.0f },
    { 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f }
};

cbuffer FilterData : register(b0)
{
    float2 texelSize;
    float scale;
}

cbuffer MaterialData : register(b1)
{
    float4x4 projectionInverse;
}

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gDepthTexture : register(t1);
SamplerState gSamplerLinear : register(s0);
SamplerState gSamplerPoint : register(s1);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float Luminance(float3 v);

PixelShaderOutput main(VertexShaderOutput input)
{
    float2 difference = float2(0.0f, 0.0f);
    for (int x = 0; x < 3; ++x)
    {
        for (int y = 0; y < 3; ++y)
        {
            float2 texcoord = input.texcoord + kIndex3x3[x][y] * texelSize;
            float ndcDepth = gDepthTexture.Sample(gSamplerPoint, texcoord);
            float4 viewSpace = mul(float4(0.0f, 0.0f, ndcDepth, 1.0f), projectionInverse);
            float viewZ = viewSpace.z * rcp(viewSpace.w);
            difference.x += viewZ * kPrewittHorizontalKernel[x][y];
            difference.y += viewZ * kPrewittVerticalKernel[x][y];
        }
    }
    
    float weight = length(difference);
    weight = saturate(weight * scale);
    PixelShaderOutput output;
    output.color.rgb = (1.0f - weight) * gTexture.Sample(gSamplerLinear, input.texcoord).rgb;
    output.color.a = 1.0f;
    return output;
}

float Luminance(float3 v)
{
    return dot(v, float3(0.2125f, 0.7154f, 0.0721f));
}