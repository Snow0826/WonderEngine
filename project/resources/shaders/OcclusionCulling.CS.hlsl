struct AABB
{
    float4 min; // Minimum corner of the bounding box
    float4 max; // Maximum corner of the bounding box
};

struct CullingData
{
    AABB box;
    uint useCulling;
};

struct UVAABB
{
    float2 min;
    float2 max;
    float minZ;
};

struct TextureData
{
    uint textureHandle;
    uint enableMipmaps;
};

struct VertexBufferView
{
    uint2 bufferLocation;
    uint sizeInBytes;
    uint strideInBytes;
};

struct IndexBufferView
{
    uint2 bufferLocation;
    uint sizeInBytes;
    uint format;
};

struct DrawIndexedArguments
{
    uint IndexCountPerInstance;
    uint InstanceCount;
    uint StartIndexLocation;
    int BaseVertexLocation;
    uint StartInstanceLocation;
};
    
struct IndirectCommand
{
    uint2 cbvAddress[2];
    TextureData textureData;
    VertexBufferView vertexBufferView;
    IndexBufferView indexBufferView;
    DrawIndexedArguments drawIndexedArguments;
};

struct BlendMode
{
    uint mode;
};

cbuffer cbuff0 : register(b0)
{
    float4 frustumPlanes[6]; // 6 planes for frustum culling
    float4 frustumCorners[8]; // 8 corners of the frustum
};

cbuffer cbuff1 : register(b1)
{
    float4x4 view; // View matrix
    float4x4 projection; // Projection matrix
};

cbuffer cbuff2 : register(b2)
{
    uint indirectCommandCount; // Number of indirect commands
};

StructuredBuffer<CullingData> cullingDataList : register(t0); // SRV: Culling data
StructuredBuffer<IndirectCommand> inputCommands : register(t1); // SRV: Indirect commands
StructuredBuffer<BlendMode> blendModes : register(t2); // SRV: Blend modes
Texture2D<float> gHiZTexture : register(t3);
SamplerState gSampler : register(s0);
AppendStructuredBuffer<IndirectCommand> noneBlendOutputCommands : register(u0); // UAV: NoneBlend processed indirect commands
AppendStructuredBuffer<IndirectCommand> normalBlendOutputCommands : register(u1); // UAV: NormalBlend processed indirect commands
AppendStructuredBuffer<IndirectCommand> additiveBlendOutputCommands : register(u2); // UAV: AdditiveBlend processed indirect commands
AppendStructuredBuffer<IndirectCommand> subtractiveBlendOutputCommands : register(u3); // UAV: SubtractiveBlend processed indirect commands
AppendStructuredBuffer<IndirectCommand> multiplicativeBlendOutputCommands : register(u4); // UAV: MultiplicativeBlend processed indirect commands
AppendStructuredBuffer<IndirectCommand> screenBlendOutputCommands : register(u5); // UAV: ScreenBlend processed indirect commands

UVAABB GetBoxInUVSpace(AABB box);

uint isBoxInFrustum(AABB box);

uint isOccludedInHiZ(AABB box);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= indirectCommandCount)
    {
        return; // Out of bounds
    }
    
    CullingData cullingData = cullingDataList[DTid.x]; // Get culling data for this thread
    if (cullingData.useCulling)
    {
        if (cullingData.box.min.x > cullingData.box.max.x || cullingData.box.min.y > cullingData.box.max.y || cullingData.box.min.z > cullingData.box.max.z)
        {
            return; // Cull invalid box
        }
    
        if (!isBoxInFrustum(cullingData.box))
        {
            return; // Cull box if outside frustum
        }
        
        if (!isOccludedInHiZ(cullingData.box))
        {
            return; // Cull box if behind HiZ depth
        }
    }
    
    switch (blendModes[DTid.x].mode)
    {
        case 0: // NoneBlend
            noneBlendOutputCommands.Append(inputCommands[DTid.x]);
            break;
        case 1: // NormalBlend
            normalBlendOutputCommands.Append(inputCommands[DTid.x]);
            break;
        case 2: // AdditiveBlend
            additiveBlendOutputCommands.Append(inputCommands[DTid.x]);
            break;
        case 3: // SubtractiveBlend
            subtractiveBlendOutputCommands.Append(inputCommands[DTid.x]);
            break;
        case 4: // MultiplicativeBlend
            multiplicativeBlendOutputCommands.Append(inputCommands[DTid.x]);
            break;
        case 5: // ScreenBlend
            screenBlendOutputCommands.Append(inputCommands[DTid.x]);
            break;
        default:
            break;
    }
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