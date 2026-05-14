#include "Skybox.hlsli"

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
};

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(world, mul(view, projection))).xyww;
    output.texcoord = input.position.xyz;
    return output;
}