#include "Instance3d.hlsli"

struct PixelShaderOutput
{
    float4 color : SV_TARGET0;
};

cbuffer TextureHandle : register(b0)
{
    uint textureHandle;
};

Texture2D<float4> gTexture[] : register(t0);
SamplerState gSampler : register(s0);

PixelShaderOutput main(VertexShaderOutput input)
{
    PixelShaderOutput output;
    float4 textureColor = gTexture[textureHandle].Sample(gSampler, input.texcoord);
    output.color = input.color * textureColor;
    
    if (output.color.a == 0.0f)
    {
        discard;
    }
    
    return output;
}