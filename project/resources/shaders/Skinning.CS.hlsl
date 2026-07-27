struct Well
{
    float4x4 skeletonSpaceMatrix;
    float4x4 skeletonSpaceInverseTransposeMatrix;
};

struct Vertex
{
    float4 position;
    float2 texcoord;
    float3 normal;
    uint section;
};

struct VertexInfluence
{
    float4 weight;
    int4 index;
};

cbuffer SkinningInformation : register(b0)
{
    uint numVertices;
};

StructuredBuffer<Well> gMatrixPalette : register(t0);
StructuredBuffer<Vertex> gInputVertices : register(t1);
StructuredBuffer<VertexInfluence> gInfluences : register(t2);
RWStructuredBuffer<Vertex> gOutputVertices : register(u0);

[numthreads(1024, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;
    if (vertexIndex >= numVertices)
    {
        return;
    }
    
    Vertex input = gInputVertices[vertexIndex];
    VertexInfluence influence = gInfluences[vertexIndex];
    Vertex skinned;
    skinned.position = float4(0, 0, 0, 0);
    skinned.texcoord = input.texcoord;
    skinned.normal = float3(0, 0, 0);
    skinned.section = input.section;
    for (int i = 0; i < 4; ++i)
    {
        int index = influence.index[i];
        float weight = influence.weight[i];
        if (weight > 0.0f)
        {
            Well well = gMatrixPalette[index];
            float4 skinnedPosition = mul(input.position, well.skeletonSpaceMatrix);
            float3 skinnedNormal = mul(input.normal, (float3x3) well.skeletonSpaceInverseTransposeMatrix);
            skinned.position += skinnedPosition * weight;
            skinned.normal += skinnedNormal * weight;
        }
    }
    skinned.position.w = 1.0f;
    skinned.normal = normalize(skinned.normal);
    gOutputVertices[vertexIndex] = skinned;
}