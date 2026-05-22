#include "Fullscreen.hlsli"

cbuffer VignetteBuffer : register(b0)
{
    float scale;
    float intensity;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color = gTexture.Sample(gSampler, input.texcoord);
    float2 correct = input.texcoord * (1.0f - input.texcoord.yx);
    float vignette = correct.x * correct.y * scale;
    vignette = saturate(pow(vignette, intensity));
    output.color.rgb *= vignette;
    return output;
}