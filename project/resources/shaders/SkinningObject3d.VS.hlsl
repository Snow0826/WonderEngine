#include "Object3d.hlsli"

struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTransposeMatrix;
};

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

cbuffer MatrixPaletteData : register(b2)
{
    uint matrixPaletteHandle;
};

StructuredBuffer<Well> gMatrixPalletes[] : register(t0);

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
    float4 weight : WEIGHT0;
    int4 index : INDEX0;
};

struct Skinned
{
    float4 position;
    float3 normal;
};

Skinned Skinning(VertexShaderInput input);

VertexShaderOutput main(VertexShaderInput input)
{
    VertexShaderOutput output;
    Skinned skinned = Skinning(input);
    output.position = mul(skinned.position, mul(world, mul(view, projection)));
    output.texcoord = input.texcoord;
    output.normal = normalize(mul(skinned.normal, (float3x3)worldInverseTranspose));
    output.worldPosition = mul(skinned.position, world).xyz;
    return output;
}

Skinned Skinning(VertexShaderInput input)
{
    Skinned skinned;
    skinned.position = float4(0, 0, 0, 0);
    skinned.normal = float3(0, 0, 0);
    for (int i = 0; i < 4; ++i)
    {
        int index = input.index[i];
        float weight = input.weight[i];
        if (weight > 0.0f)
        {
            Well well = gMatrixPalletes[matrixPaletteHandle][index];
            float4 skinnedPosition = mul(input.position, well.skeletonSpaceMatrix);
            float3 skinnedNormal = mul(input.normal, (float3x3) well.skeletonSpaceInverseTransposeMatrix);
            skinned.position += skinnedPosition * weight;
            skinned.normal += skinnedNormal * weight;
        }
    }
    skinned.position.w = 1.0f;
    skinned.normal = normalize(skinned.normal);
    return skinned;
}