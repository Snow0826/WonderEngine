#include "Object3d.hlsli"

ConstantBuffer<PerView> gPerView : register(b0);
ConstantBuffer<MeshData> gMeshData : register(b1);
StructuredBuffer<uint> gInstanceIndices : register(t0);
StructuredBuffer<InstanceData> gInstanceData : register(t1);

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    uint instanceIndex = gInstanceIndices[gMeshData.meshOffset + instanceId];
    InstanceData instanceData = gInstanceData[instanceIndex];
    output.position = mul(input.position, mul(instanceData.world, mul(gPerView.view, gPerView.projection)));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(input.normal, (float3x3) instanceData.worldInverseTranspose));
    output.worldPosition = mul(input.position, instanceData.world).xyz;
    output.instanceIndex = instanceIndex;
    return output;
}