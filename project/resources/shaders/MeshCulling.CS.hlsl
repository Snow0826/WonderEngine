#include "IndirectCommand.hlsli"
#include "AABB.hlsli"

static const uint kMaxMeshType = 6; // Number of mesh types (Model, Plane, Box, Ring, Cylinder, Tree)
static const uint kMaxBlendMode = 6; // Number of blend modes (None, Normal, Additive, Subtractive, Multiplicative, Screen)
static const uint kCreatingCommandIndex = (uint) -2; // Special value indicating that a command is being created

struct Object
{
    float4x4 world; // World matrix for the object
    uint meshType; // Mesh type for the object (0: Model, 1: Plane, 2: Box, 3: Ring, 4: Cylinder, 5: Tree)
    uint blendMode; // Blend mode for the object (0: None, 1: Normal, 2: Additive, 3: Subtractive, 4: Multiplicative, 5: Screen)
    uint meshIndex; // Index into the meshes buffer for this object
    uint instanceIndex; // Index into the instance buffer for this object
    uint useCulling; // Flag indicating whether to use culling for this mesh (0: no culling, 1: use culling)
};

struct Mesh
{
    VertexBufferView vertexBufferView; // Vertex buffer view for the mesh
    IndexBufferView indexBufferView; // Index buffer view for the mesh
    uint indexCount; // Number of indices in the mesh
};

cbuffer Constants : register(b0)
{
    uint objectCount; // Number of meshes
    uint4 queueOffsets[(kMaxMeshType * kMaxBlendMode + 3) / 4]; // Offsets for each mesh type and blend mode combination
};

cbuffer Frustum : register(b1)
{
    float4 frustumPlanes[6]; // 6 planes for frustum culling
    float4 frustumCorners[8]; // 8 corners of the frustum
};

cbuffer ViewProjection : register(b2)
{
    float4x4 view; // View matrix
    float4x4 projection; // Projection matrix
};

cbuffer CameraPosition : register(b3)
{
    float3 cameraPosition; // Camera position in world space
};

StructuredBuffer<Object> gObjects : register(t0); // SRV: Object data
StructuredBuffer<AABB> gBoxes : register(t1); // SRV: boxes
StructuredBuffer<Mesh> gMeshes : register(t2); // SRV: meshes
Texture2D<float> gHiZTexture : register(t3); // SRV: Hi-Z texture for occlusion culling
SamplerState gSampler : register(s0); // Sampler for Hi-Z texture
RWStructuredBuffer<ObjectState> gObjectState : register(u0); // UAV: Visibility flags for each mesh
RWStructuredBuffer<IndirectCommandState> gIndirectCommandStates : register(u1); // UAV: Mesh Command States
RWStructuredBuffer<IndirectCommand> gIndirectCommands : register(u2); // UAV: Processed Indirect Commands
RWByteAddressBuffer gCommandCounters : register(u3); // UAV: Command Counters for Processed Indirect Commands

bool IsInsideFrustum(AABB box);

bool IsOccluded(AABB box);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint objectIndex = DTid.x;
    if (objectIndex >= objectCount)
    {
        return; // Out of bounds
    }
    
    gObjectState[objectIndex].visible = 0;
    gObjectState[objectIndex].meshIndex = 0;
    gObjectState[objectIndex].instanceIndex = 0;
    
    Object object = gObjects[objectIndex]; // Get object data for this mesh
    if (object.useCulling)
    {
        AABB box = gBoxes[object.meshIndex]; // Get bounding box for this mesh
        float3 center = (box.min.xyz + box.max.xyz) * 0.5f;
        float3 extent = (box.max.xyz - box.min.xyz) * 0.5f;
        center = mul(float4(center, 1.0f), object.world).xyz; // Transform center to world space
        extent = mul(extent, abs((float3x3) object.world)); // Transform extent to world space (ignore translation)
        AABB worldAABB;
        worldAABB.min.xyz = center - extent;
        worldAABB.min.w = 1.0f; // Set w to 1 for homogeneous coordinates
        worldAABB.max.xyz = center + extent;
        worldAABB.max.w = 1.0f; // Set w to 1 for homogeneous coordinates
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
    }
    
    gObjectState[objectIndex].visible = 1; // Mark mesh as visible
    gObjectState[objectIndex].meshIndex = object.meshIndex; // Store mesh index for this object
    gObjectState[objectIndex].instanceIndex = object.instanceIndex; // Store instance index for this object
    
    uint old;
    InterlockedCompareExchange(gIndirectCommandStates[object.meshIndex].commandIndex, kInvalidCommandIndex, kCreatingCommandIndex, old);
    if (old == kInvalidCommandIndex)
    {
        uint queueIndex = object.meshType * kMaxBlendMode + object.blendMode;
        uint commandIndex;
        gCommandCounters.InterlockedAdd(queueIndex * 4, 1, commandIndex);
        uint queueOffset = queueOffsets[queueIndex / 4][queueIndex % 4];
        gIndirectCommands[queueOffset + commandIndex].vertexBufferView = gMeshes[object.meshIndex].vertexBufferView;
        gIndirectCommands[queueOffset + commandIndex].indexBufferView = gMeshes[object.meshIndex].indexBufferView;
        gIndirectCommands[queueOffset + commandIndex].drawIndexedArguments.IndexCountPerInstance = gMeshes[object.meshIndex].indexCount;
        gIndirectCommandStates[object.meshIndex].commandIndex = queueOffset + commandIndex;
    }
    InterlockedAdd(gIndirectCommandStates[object.meshIndex].instanceCount, 1);
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