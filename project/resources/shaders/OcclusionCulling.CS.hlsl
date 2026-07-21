#include "IndirectCommand.hlsli"

static const uint kMaxMeshType = 5; // Number of mesh types (Model, Plane, Box, Ring, Cylinder)
static const uint kMaxBlendMode = 6; // Number of blend modes (None, Normal, Additive, Subtractive, Multiplicative, Screen)
static const uint kCreatingCommandIndex = (uint) -2;

struct Object
{
    float4x4 world; // World matrix for the object
    uint meshType; // Mesh type for the object (0: Model, 1: Plane, 2: Box, 3: Ring, 4: Cylinder)
    uint blendMode; // Blend mode for the object (0: None, 1: Normal, 2: Additive, 3: Subtractive, 4: Multiplicative, 5: Screen)
};

struct AABB
{
    float4 min; // Minimum corner of the bounding box
    float4 max; // Maximum corner of the bounding box
};

struct Mesh
{
    AABB box;
    uint objectHandle;
    uint lodOffset;
    uint lodCount;
    uint useCulling;
};

struct UVAABB
{
    float2 min;
    float2 max;
    float minZ;
};

struct MeshLOD
{
    IndirectCommand command;
    float error;
};

cbuffer Frustum : register(b0)
{
    float4 frustumPlanes[6]; // 6 planes for frustum culling
    float4 frustumCorners[8]; // 8 corners of the frustum
};

cbuffer ViewProjection : register(b1)
{
    float4x4 view; // View matrix
    float4x4 projection; // Projection matrix
};

cbuffer CameraPosition : register(b2)
{
    float3 cameraPosition; // Camera position in world space
};

cbuffer Constants : register(b3)
{
    uint meshCount; // Number of meshes
    uint4 queueOffsets[(kMaxMeshType * kMaxBlendMode + 3) / 4]; // Offsets for each mesh type and blend mode combination
};

StructuredBuffer<Object> objects : register(t0); // SRV: Object data
StructuredBuffer<Mesh> meshes : register(t1); // SRV: Meshes
StructuredBuffer<MeshLOD> meshLODs : register(t2); // SRV: Mesh LODs
Texture2D<float> gHiZTexture : register(t3);
SamplerState gSampler : register(s0);
RWStructuredBuffer<IndirectCommand> commands : register(u0); // UAV: Processed Indirect Commands
RWStructuredBuffer<MeshCommandState> meshCommandStates : register(u1); // UAV: Mesh Command States
RWByteAddressBuffer commandCounters : register(u2); // UAV: Command Counters for Processed Indirect Commands

UVAABB GetBoxInUVSpace(AABB box);

uint isBoxInFrustum(AABB box);

uint isOccludedInHiZ(AABB box);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= meshCount)
    {
        return; // Out of bounds
    }
    
    Mesh mesh = meshes[DTid.x]; // Get culling data for this thread
    Object object = objects[mesh.objectHandle]; // Get object data for this mesh
    float3 center = (mesh.box.min.xyz + mesh.box.max.xyz) * 0.5f;
    float3 extent = (mesh.box.max.xyz - mesh.box.min.xyz) * 0.5f;
    center = mul(float4(center, 1.0f), object.world).xyz; // Transform center to world space
    extent = mul(extent, abs((float3x3) object.world)); // Transform extent to world space (ignore translation)
    AABB box;
    box.min.xyz = center - extent;
    box.min.w = 1.0f; // Set w to 1 for homogeneous coordinates
    box.max.xyz = center + extent;
    box.max.w = 1.0f; // Set w to 1 for homogeneous coordinates
    uint selectedLOD = 0;
    if (mesh.useCulling)
    {
        if (box.min.x > box.max.x || box.min.y > box.max.y || box.min.z > box.max.z)
        {
            return; // Cull invalid box
        }
    
        if (!isBoxInFrustum(box))
        {
            return; // Cull box if outside frustum
        }
        
        if (!isOccludedInHiZ(box))
        {
            return; // Cull box if behind HiZ depth
        }
        
        float distanceToCamera = length(center - cameraPosition);
        for (uint i = 0; i < mesh.lodCount; i++)
        {
            float projectedError = meshLODs[mesh.lodOffset + i].error * projection._11 / distanceToCamera;
            if (projectedError < 0.001f)
            {
                selectedLOD = i;
            }
            else
            {
                break;
            }
        }
    }
    
    uint stateIndex = mesh.lodOffset + selectedLOD;
    IndirectCommand command = meshLODs[stateIndex].command;
    uint old;
    InterlockedCompareExchange(meshCommandStates[stateIndex].commandIndex, kInvalidCommandIndex, kCreatingCommandIndex, old);
    if (old == kInvalidCommandIndex)
    {
        uint queueIndex = object.meshType * kMaxBlendMode + object.blendMode;
        uint commandIndex;
        commandCounters.InterlockedAdd(queueIndex * 4, 1, commandIndex);
        uint queueOffset = queueOffsets[queueIndex / 4][queueIndex % 4];
        commands[queueOffset + commandIndex] = command;
        meshCommandStates[stateIndex].commandIndex = queueOffset + commandIndex;
    }
    InterlockedAdd(meshCommandStates[stateIndex].instanceCount, 1);
}

UVAABB GetBoxInUVSpace(AABB box)
{
    float4 corners[8];
    corners[0] = float4(box.min.x, box.min.y, box.min.z, 1.0f);
    corners[1] = float4(box.max.x, box.min.y, box.min.z, 1.0f);
    corners[2] = float4(box.min.x, box.max.y, box.min.z, 1.0f);
    corners[3] = float4(box.max.x, box.max.y, box.min.z, 1.0f);
    corners[4] = float4(box.min.x, box.min.y, box.max.z, 1.0f);
    corners[5] = float4(box.max.x, box.min.y, box.max.z, 1.0f);
    corners[6] = float4(box.min.x, box.max.y, box.max.z, 1.0f);
    corners[7] = float4(box.max.x, box.max.y, box.max.z, 1.0f);

    float2 uvMin = float2(1.0f, 1.0f);
    float2 uvMax = float2(0.0f, 0.0f);
    float minZ = 1.0f;
    
    for (uint i = 0; i < 8; i++)
    {
        float4 clipSpacePos = mul(mul(corners[i], view), projection);
        if (clipSpacePos.w <= 0.0f)
        {
            continue;
        }
        
        float3 ndcPos = clipSpacePos.xyz / clipSpacePos.w;
        float2 uvPos = ndcPos.xy * 0.5f + 0.5f;
        uvPos.y = 1.0f - uvPos.y; // Flip Y for texture space
        uvMin = min(uvMin, uvPos);
        uvMax = max(uvMax, uvPos);
        minZ = min(minZ, ndcPos.z);
    }
    
    UVAABB uvBox;
    uvBox.min = uvMin;
    uvBox.max = uvMax;
    uvBox.minZ = minZ;
    return uvBox;
}

uint isBoxInFrustum(AABB box)
{
    for (uint i = 0; i < 6; i++)
    {
        uint r = 0;
        r += (dot(frustumPlanes[i], float4(box.min.x, box.min.y, box.min.z, 1.0f)) < 0.0f) ? 1 : 0;
        r += (dot(frustumPlanes[i], float4(box.max.x, box.min.y, box.min.z, 1.0f)) < 0.0f) ? 1 : 0;
        r += (dot(frustumPlanes[i], float4(box.min.x, box.max.y, box.min.z, 1.0f)) < 0.0f) ? 1 : 0;
        r += (dot(frustumPlanes[i], float4(box.max.x, box.max.y, box.min.z, 1.0f)) < 0.0f) ? 1 : 0;
        r += (dot(frustumPlanes[i], float4(box.min.x, box.min.y, box.max.z, 1.0f)) < 0.0f) ? 1 : 0;
        r += (dot(frustumPlanes[i], float4(box.max.x, box.min.y, box.max.z, 1.0f)) < 0.0f) ? 1 : 0;
        r += (dot(frustumPlanes[i], float4(box.min.x, box.max.y, box.max.z, 1.0f)) < 0.0f) ? 1 : 0;
        r += (dot(frustumPlanes[i], float4(box.max.x, box.max.y, box.max.z, 1.0f)) < 0.0f) ? 1 : 0;
        if (r == 8)
        {
            return 0; // Box is outside the frustum
        }
    }
    
    uint r = 0;
    for (uint a = 0; a < 8; a++)
    {
        r += ((frustumCorners[a].x > box.max.x) ? 1 : 0);
        if (r == 8)
        {
            return 0; // Box is outside the frustum
        }
    }
    
    r = 0;
    for (uint b = 0; b < 8; b++)
    {
        r += ((frustumCorners[b].x < box.min.x) ? 1 : 0);
        if (r == 8)
        {
            return 0; // Box is outside the frustum
        }
    }
    
    r = 0;
    for (uint c = 0; c < 8; c++)
    {
        r += ((frustumCorners[c].y > box.max.y) ? 1 : 0);
        if (r == 8)
        {
            return 0; // Box is outside the frustum
        }
    }
    
    r = 0;
    for (uint d = 0; d < 8; d++)
    {
        r += ((frustumCorners[d].y < box.min.y) ? 1 : 0);
        if (r == 8)
        {
            return 0; // Box is outside the frustum
        }
    }
    
    r = 0;
    for (uint e = 0; e < 8; e++)
    {
        r += ((frustumCorners[e].z > box.max.z) ? 1 : 0);
        if (r == 8)
        {
            return 0; // Box is outside the frustum
        }
    }
    
    r = 0;
    for (uint f = 0; f < 8; f++)
    {
        r += ((frustumCorners[f].z < box.min.z) ? 1 : 0);
        if (r == 8)
        {
            return 0; // Box is outside the frustum
        }
    }
    
    return 1; // Box is inside the frustum
}

uint isOccludedInHiZ(AABB box)
{
    uint baseWidth, baseHeight;
    gHiZTexture.GetDimensions(baseWidth, baseHeight);
    const UVAABB uvBox = GetBoxInUVSpace(box);
    const float2 boxWidth = uvBox.max - uvBox.min;
    const float2 texelSize = boxWidth * float2(baseWidth, baseHeight);
    const float mip = max(floor(log2(max(texelSize.x, texelSize.y))), 0.0f);
    const float sample1 = gHiZTexture.SampleLevel(gSampler, uvBox.min, mip);
    const float sample2 = gHiZTexture.SampleLevel(gSampler, float2(uvBox.max.x, uvBox.min.y), mip);
    const float sample3 = gHiZTexture.SampleLevel(gSampler, float2(uvBox.min.x, uvBox.max.y), mip);
    const float sample4 = gHiZTexture.SampleLevel(gSampler, uvBox.max, mip);
    const float maxZ = max(max(sample1, sample2), max(sample3, sample4));
    return (uvBox.minZ > maxZ) ? 0 : 1; // Return 0 if occluded, else 1
}