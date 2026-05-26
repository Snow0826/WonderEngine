#include "Fullscreen.hlsli"

cbuffer FilterData : register(b0)
{
    int kernelRadius;
    float2 texelSize;
}

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    uint count = 0;
    for (int y = -kernelRadius; y <= kernelRadius; ++y)
    {
        for (int x = -kernelRadius; x <= kernelRadius; ++x)
        {
            float2 texcoord = input.texcoord + float2(x, y) * texelSize;
            output.color.rgb += gTexture.Sample(gSampler, texcoord).rgb;
            count++;
        }
    }
    output.color.rgb /= count;
    return output;
}