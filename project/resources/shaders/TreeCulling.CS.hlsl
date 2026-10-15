#include "AABB.hlsli"

cbuffer Frustum : register(b0)
{
    float4 frustumPlanes[6];
    float4 frustumCorners[8];
};

cbuffer ViewProjection : register(b1)
{
    float4x4 view;
    float4x4 projection;
};

cbuffer Constants : register(b2)
{
    uint treeCount;
};

StructuredBuffer<AABB> treeBounds : register(t0);
Texture2D<float> gHiZTexture : register(t1);
SamplerState gSampler : register(s0);
RWStructuredBuffer<uint> treeVisibility : register(u0);

bool IsInsideFrustum(AABB box);

bool IsOccluded(AABB box);

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint treeIndex = DTid.x;
    if (treeIndex >= treeCount)
    {
        return;
    }

    AABB bounds = treeBounds[treeIndex];
    treeVisibility[treeIndex] = IsInsideFrustum(bounds) && !IsOccluded(bounds);
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