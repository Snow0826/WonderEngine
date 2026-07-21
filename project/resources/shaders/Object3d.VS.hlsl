#include "Object3d.hlsli"

cbuffer ViewProjection : register(b0)
{
    float4x4 view;
    float4x4 projection;
};

cbuffer MeshOffset : register(b1)
{
    uint meshOffset;
};

struct WorldTransform
{
    float4x4 world;
    float4x4 worldInverseTranspose;
};

StructuredBuffer<uint> gInstanceIndices : register(t0);
StructuredBuffer<WorldTransform> gWorldTransforms : register(t1);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    uint instanceIndex = gInstanceIndices[meshOffset + instanceId];
    WorldTransform worldTransform = gWorldTransforms[instanceIndex];
    output.position = mul(input.position, mul(worldTransform.world, mul(view, projection)));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) worldTransform.worldInverseTranspose));
    output.worldPosition = mul(input.position, worldTransform.world).xyz;
    output.instanceIndex = instanceIndex;
    return output;
}