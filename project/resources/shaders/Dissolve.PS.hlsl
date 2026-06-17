#include "Fullscreen.hlsli"

cbuffer DissolveData : register(b0)
{
    float threshold;
    float edgeWidth;
    float3 edgeColor;
};

Texture2D<float4> gTexture : register(t0);
Texture2D<float> gMaskTexture : register(t1);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    float mask = gMaskTexture.Sample(gSampler, input.texcoord);
    if (mask <= threshold)
    {
        discard;
    }
    float edge = 1.0f - smoothstep(threshold, threshold + edgeWidth, mask);
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    output.color.rgb += edge * edgeColor;
    return output;
}