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
    float4 localPosition = input.position;
    float3 normal = input.normal;
    if (input.section == 0)
    {
        float t = input.position.y * instanceData.inverseBranchLength;
        localPosition.xz *= lerp(instanceData.bottomRadius, instanceData.topRadius, t);
        normal.y = (instanceData.bottomRadius - instanceData.topRadius) * instanceData.inverseBranchLength;
        normal = normalize(normal);
    }
    else if (input.section == 1)
    {
        localPosition.xz *= instanceData.topRadius;
    }
    else if (input.section == 2)
    {
        localPosition.xz *= instanceData.bottomRadius;
    }
    output.position = mul(localPosition, mul(instanceData.world, mul(gPerView.view, gPerView.projection)));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(normal, (float3x3) instanceData.worldInverseTranspose));
    output.worldPosition = mul(localPosition, instanceData.world).xyz;
    output.instanceIndex = instanceIndex;
    return output;
}