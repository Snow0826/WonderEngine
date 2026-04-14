#include "Object3d.hlsli"

cbuffer WorldTransform : register(b0)
{
    float4x4 world;
    float4x4 worldInverseTranspose;
};

cbuffer ViewProjection : register(b1)
{
    float4x4 view;
    float4x4 projection;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(world, mul(view, projection)));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3)worldInverseTranspose));
    output.worldPosition = mul(input.position, world).xyz;
    return output;
}