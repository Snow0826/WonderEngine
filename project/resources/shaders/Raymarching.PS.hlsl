#include "Fullscreen.hlsli"

cbuffer RaymarchData : register(b0)
{
    float4x4 inverseView;
    float4x4 inverseProjection;
    float3 cameraPosition;
    float maxDistance;
    uint maxSteps;
    float epsilon;
    float2 screenSize;
};

cbuffer SdSceneData : register(b1)
{
    uint numSpheres;
    uint numBoxes;
    float smoothness;
};

struct SdSphereData
{
    float4x4 inverseWorld;
    float radius;
    float scale;
};

struct SdBoxData
{
    float4x4 inverseWorld;
    float3 size;
    float scale;
};

StructuredBuffer<SdSphereData> spheres : register(t0);
StructuredBuffer<SdBoxData> boxes : register(t1);

float sdScene(float3 worldPosition);

float sdSphere(float3 position, float radius);

float sdBox(float3 position, float3 size);

float smin(float d1, float d2, float k);

float4 main(VertexShaderOutput input) : SV_TARGET
{
    float2 uv = input.position.xy * screenSize;
    float2 ndc = uv * 2.0f - 1.0f;
    float4 rayClip = float4(ndc, 1.0f, 1.0f);
    float4 rayView = mul(rayClip, inverseProjection);
    rayView.xyz /= rayView.w;
    float4 rayWorld = mul(float4(rayView.xyz, 0.0f), inverseView);
    float3 rayDirection = normalize(rayWorld.xyz);
    float3 rayPosition = cameraPosition;
    bool hit = false;
    for (uint i = 0; i < maxSteps; ++i)
    {
        float distance = sdScene(rayPosition);
        if (distance < epsilon)
        {
            hit = true;
            break;
        }
        if (length(rayPosition - cameraPosition) > maxDistance)
        {
            break;
        }
        rayPosition += rayDirection * distance;
    }
    
    if (!hit)
    {
        return float4(0.0f, 0.0f, 0.0f, 1.0f);
    }
    
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}

float sdScene(float3 worldPosition)
{
    float distance = 1e10f;
    
    for (uint i = 0; i < numSpheres; ++i)
    {
        float3 localPosition = mul(float4(worldPosition, 1.0f), spheres[i].inverseWorld).xyz;
        float d = sdSphere(localPosition, spheres[i].radius);
        d *= spheres[i].scale;
        distance = smin(distance, d, smoothness);
    }
    
    for (uint i = 0; i < numBoxes; ++i)
    {
        float3 localPosition = mul(float4(worldPosition, 1.0f), boxes[i].inverseWorld).xyz;
        float d = sdBox(localPosition, boxes[i].size);
        d *= boxes[i].scale;
        distance = smin(distance, d, smoothness);
    }
    
    return distance;
}

float sdSphere(float3 position, float radius)
{
    return length(position) - radius;
}

float sdBox(float3 position, float3 size)
{
    float3 d = abs(position) - size;
    return length(max(d, 0.0f)) + min(max(d.x, max(d.y, d.z)), 0.0f);
}

float smin(float d1, float d2, float k)
{
    k *= 1.0f;
    float r = exp2(-d1 / k) + exp2(-d2 / k);
    return -k * log2(r);
}