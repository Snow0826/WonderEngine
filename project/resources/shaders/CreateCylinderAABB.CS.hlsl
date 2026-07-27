#include "AABB.hlsli"

cbuffer Constants : register(b0)
{
    uint cylinderCount;
};

struct Cylinder
{
    float radius;
    float height;
    uint aabbIndex;
};

StructuredBuffer<Cylinder> gCylinders : register(t0); // SRV: cylinders
RWStructuredBuffer<AABB> gBoxes : register(u0); // UAV: boxes

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= cylinderCount)
    {
        return;
    }
    
    AABB box;
    Cylinder cylinder = gCylinders[DTid.x];
    box.min = float4(-cylinder.radius, 0.0f, -cylinder.radius, 1.0f);
    box.max = float4(cylinder.radius, cylinder.height, cylinder.radius, 1.0f);
    gBoxes[cylinder.aabbIndex] = box;
}