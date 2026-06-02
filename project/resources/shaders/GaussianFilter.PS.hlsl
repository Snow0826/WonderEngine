#include "Fullscreen.hlsli"

static const float PI = 3.14159265f;

cbuffer FilterData : register(b0)
{
    int kernelRadius;
    float2 texelSize;
    float sigma;
}

Texture2D<float4> gTexture : register(t0);
SamplerState gSampler : register(s0);

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

float gauss(float x, float y, float sigma);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    output.color.rgb = float3(0.0f, 0.0f, 0.0f);
    output.color.a = 1.0f;
    float weight = 0.0f;
    for (int y = -kernelRadius; y <= kernelRadius; ++y)
    {
        for (int x = -kernelRadius; x <= kernelRadius; ++x)
        {
            float kernel = gauss(x, y, sigma);
            float2 texcoord = input.texcoord + float2(x, y) * texelSize;
            float3 sampleColor = gTexture.Sample(gSampler, texcoord).rgb;
            output.color.rgb += sampleColor * kernel;
            weight += kernel;
        }
    }
    output.color.rgb *= rcp(weight);
    return output;
}

float gauss(float x, float y, float sigma)
{
    float exponent = -(x * x + y * y) * rcp(2.0f * sigma * sigma);
    float denominator = 2.0f * PI * sigma * sigma;
    return exp(exponent) * rcp(denominator);
}