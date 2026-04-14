#include "Instance3d.hlsli"

struct InstanceData
{
    float4x4 world;
    float4 color;
};

StructuredBuffer<InstanceData> gInstanceData : register(t0);

cbuffer ViewProjection : register(b0)
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

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, mul(gInstanceData[instanceId].world, mul(view, projection)));
    output.texcoord = input.texcoord;
    output.color = gInstanceData[instanceId].color;
    return output;
}