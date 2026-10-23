#include "AABB.hlsli"

static const float FLT_MAX = 3.402823466e+38f;

struct Mesh
{
    uint vertexOffset;
    uint vertexCount;
    uint aabbIndex;
};

struct Vertex
{
    float4 position;
};

StructuredBuffer<Mesh> gMeshes : register(t0); // SRV: meshes
StructuredBuffer<Vertex> gVertices : register(t1); // SRV: vertices
RWStructuredBuffer<AABB> gBoxes : register(u0); // UAV: boxes

groupshared AABB sharedBoxes[64];

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID, uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID)
{   
    Mesh mesh = gMeshes[Gid.x];
    AABB box;
    box.min = float4(FLT_MAX, FLT_MAX, FLT_MAX, 1.0f);
    box.max = float4(-FLT_MAX, -FLT_MAX, -FLT_MAX, 1.0f);
    for (uint i = GTid.x; i < mesh.vertexCount; i += 64)
    {
        Vertex vertex = gVertices[mesh.vertexOffset + i];
        box.min = min(box.min, vertex.position);
        box.max = max(box.max, vertex.position);
    }
    
    sharedBoxes[GTid.x] = box;
    GroupMemoryBarrierWithGroupSync();
    
    for (uint stride = 32; stride > 0; stride >>= 1)
    {
        if (GTid.x < stride)
        {
            sharedBoxes[GTid.x].min = min(sharedBoxes[GTid.x].min, sharedBoxes[GTid.x + stride].min);
            sharedBoxes[GTid.x].max = max(sharedBoxes[GTid.x].max, sharedBoxes[GTid.x + stride].max);
        }
        GroupMemoryBarrierWithGroupSync();
    }
    
    if (GTid.x == 0)
    {
        gBoxes[mesh.aabbIndex] = sharedBoxes[0];
    }
}