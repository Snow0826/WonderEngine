#include "Fullscreen.hlsli"

cbuffer BlurData : register(b0)
{
    float2 center;
    float blurWidth;
    uint sampleCount;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float2 direction = input.texcoord - center;
    float3 outputColor = float3(0.0f, 0.0f, 0.0f);
    for (uint sampleIndex = 0; sampleIndex < sampleCount; ++sampleIndex)
    {
        float2 texcoord = input.texcoord + direction * blurWidth * float(sampleIndex);
        outputColor.rgb += gTexture.Sample(gSampler, texcoord).rgb;
    }
    outputColor.rgb *= rcp(float(sampleCount));
    PixelShaderOutput output;
    output.color.rgb = outputColor;
    output.color.a = 1.0f;
    return output;
}