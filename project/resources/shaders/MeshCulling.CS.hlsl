#include "IndirectCommand.hlsli"
#include "AABB.hlsli"

static const uint kMaxMeshType = 5; // Number of mesh types (Model, Plane, Box, Ring, Cylinder)
static const uint kMaxBlendMode = 6; // Number of blend modes (None, Normal, Additive, Subtractive, Multiplicative, Screen)
static const uint kCreatingCommandIndex = (uint) -2; // Special value indicating that a command is being created

struct Object
{
    float4x4 world; // World matrix for the object
    uint meshType; // Mesh type for the object (0: Model, 1: Plane, 2: Box, 3: Ring, 4: Cylinder)
    uint blendMode; // Blend mode for the object (0: None, 1: Normal, 2: Additive, 3: Subtractive, 4: Multiplicative, 5: Screen)
};

struct Mesh
{
    uint objectIndex; // Index of the object this mesh belongs to
    uint lodOffset; // Offset into the meshLODs buffer for this mesh's LODs
    uint lodCount; // Number of LODs for this mesh
    uint useCulling; // Flag indicating whether to use culling for this mesh (0: no culling, 1: use culling)
    uint treeIndex; // Tree visibility index, or 0xffffffff for non-tree objects
};

struct MeshLOD
{
    IndirectCommand command; // Indirect command for this LOD
    float error; // Error metric for this LOD (used for LOD selection)
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
StructuredBuffer<AABB> boxes : register(t3); // SRV: boxes
StructuredBuffer<uint> treeVisibility : register(t4); // SRV: Tree visibility buffer
Texture2D<float> gHiZTexture : register(t5); // SRV: Hi-Z texture for occlusion culling
SamplerState gSampler : register(s0); // Sampler for Hi-Z texture
RWStructuredBuffer<MeshCommandState> meshCommandStates : register(u0); // UAV: Mesh Command States
RWStructuredBuffer<MeshLODState> meshLODStates : register(u1); // UAV: Mesh LOD States
RWStructuredBuffer<IndirectCommand> commands : register(u2); // UAV: Processed Indirect Commands
RWByteAddressBuffer commandCounters : register(u3); // UAV: Command Counters for Processed Indirect Commands

bool IsInsideFrustum(AABB box);

bool IsOccluded(AABB box);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= meshCount)
    {
        return; // Out of bounds
    }
    
    meshLODStates[DTid.x].meshLODIndex = 0; // Initialize mesh LOD index to 0
    meshLODStates[DTid.x].visible = 0; // Initialize visibility to false
    meshLODStates[DTid.x].instanceIndex = 0; // Initialize instance index to 0
    Mesh mesh = meshes[DTid.x]; // Get culling data for this thread
    if (mesh.treeIndex != 0xffffffff && treeVisibility[mesh.treeIndex] == 0)
    {
        return; // Skip detailed branch culling when the parent tree was rejected
    }
    AABB box = boxes[DTid.x]; // Get bounding box for this mesh
    Object object = objects[mesh.objectIndex]; // Get object data for this mesh
    float3 center = (box.min.xyz + box.max.xyz) * 0.5f;
    float3 extent = (box.max.xyz - box.min.xyz) * 0.5f;
    center = mul(float4(center, 1.0f), object.world).xyz; // Transform center to world space
    extent = mul(extent, abs((float3x3) object.world)); // Transform extent to world space (ignore translation)
    AABB worldAABB;
    worldAABB.min.xyz = center - extent;
    worldAABB.min.w = 1.0f; // Set w to 1 for homogeneous coordinates
    worldAABB.max.xyz = center + extent;
    worldAABB.max.w = 1.0f; // Set w to 1 for homogeneous coordinates
    uint selectedLOD = 0;
    if (mesh.useCulling)
    {
        if (worldAABB.min.x > worldAABB.max.x || worldAABB.min.y > worldAABB.max.y || worldAABB.min.z > worldAABB.max.z)
        {
            return; // Cull invalid box
        }
    
        if (!IsInsideFrustum(worldAABB))
        {
            return; // Cull box if outside frustum
        }
        
        if (IsOccluded(worldAABB))
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
    
    uint meshLODIndex = mesh.lodOffset + selectedLOD;
    meshLODStates[DTid.x].meshLODIndex = meshLODIndex; // Store selected LOD index
    meshLODStates[DTid.x].visible = 1; // Mark mesh as visible
    meshLODStates[DTid.x].instanceIndex = mesh.objectIndex; // Store instance index for this mesh
    
    uint old;
    InterlockedCompareExchange(meshCommandStates[meshLODIndex].commandIndex, kInvalidCommandIndex, kCreatingCommandIndex, old);
    if (old == kInvalidCommandIndex)
    {
        uint queueIndex = object.meshType * kMaxBlendMode + object.blendMode;
        uint commandIndex;
        commandCounters.InterlockedAdd(queueIndex * 4, 1, commandIndex);
        uint queueOffset = queueOffsets[queueIndex / 4][queueIndex % 4];
        commands[queueOffset + commandIndex] = meshLODs[meshLODIndex].command;
        meshCommandStates[meshLODIndex].commandIndex = queueOffset + commandIndex;
    }
    InterlockedAdd(meshCommandStates[meshLODIndex].instanceCount, 1);
}

bool IsInsideFrustum(AABB box)
{
    for (uint planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
        float3 positive = float3(
            frustumPlanes[planeIndex].x >= 0.0f ? box.max.x : box.min.x,
            frustumPlanes[planeIndex].y >= 0.0f ? box.max.y : box.min.y,
            frustumPlanes[planeIndex].z >= 0.0f ? box.max.z : box.min.z);
        if (dot(frustumPlanes[planeIndex], float4(positive, 1.0f)) < 0.0f)
        {
            return false;
        }
    }
    return true;
}

bool IsOccluded(AABB box)
{
    float3 corners[8] =
    {
        float3(box.min.x, box.min.y, box.min.z), float3(box.max.x, box.min.y, box.min.z),
        float3(box.min.x, box.max.y, box.min.z), float3(box.max.x, box.max.y, box.min.z),
        float3(box.min.x, box.min.y, box.max.z), float3(box.max.x, box.min.y, box.max.z),
        float3(box.min.x, box.max.y, box.max.z), float3(box.max.x, box.max.y, box.max.z)
    };

    float2 uvMin = float2(1.0f, 1.0f);
    float2 uvMax = float2(0.0f, 0.0f);
    float nearestDepth = 1.0f;
    for (uint i = 0; i < 8; ++i)
    {
        float4 clip = mul(mul(float4(corners[i], 1.0f), view), projection);
        // Near-plane intersections need a conservative visible result.
        if (clip.w <= 0.0001f)
        {
            return false;
        }
        float3 ndc = clip.xyz / clip.w;
        float2 uv = ndc.xy * 0.5f + 0.5f;
        uv.y = 1.0f - uv.y;
        uvMin = min(uvMin, uv);
        uvMax = max(uvMax, uv);
        nearestDepth = min(nearestDepth, ndc.z);
    }

    uint width, height;
    gHiZTexture.GetDimensions(width, height);
    const float2 pixelExtent = (uvMax - uvMin) * float2(width, height);
    const float mip = max(floor(log2(max(pixelExtent.x, pixelExtent.y))), 0.0f);
    const float sample1 = gHiZTexture.SampleLevel(gSampler, uvMin, mip);
    const float sample2 = gHiZTexture.SampleLevel(gSampler, float2(uvMax.x, uvMin.y), mip);
    const float sample3 = gHiZTexture.SampleLevel(gSampler, float2(uvMin.x, uvMax.y), mip);
    const float sample4 = gHiZTexture.SampleLevel(gSampler, uvMax, mip);
    float farthestOccluder = max(max(sample1, sample2), max(sample3, sample4));
    return nearestDepth > farthestOccluder;
}
