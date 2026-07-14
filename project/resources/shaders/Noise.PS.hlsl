#include "Fullscreen.hlsli"

cbuffer PerFrame : register(b0)
{
    float time;
};

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float rand2dTo1d(float2 value, float2 dotDir = float2(12.9898, 78.233));

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float random = rand2dTo1d(input.texcoord * time);
    output.color = gTexture.Sample(gSampler, input.texcoord) * random;
    return output;
}

float rand2dTo1d(float2 value, float2 dotDir)
{
    float2 smallValue = sin(value);
    float random = dot(smallValue, dotDir);
    random = frac(sin(random) * 143758.5453);
    return random;
}