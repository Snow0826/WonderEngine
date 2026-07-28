#include "Particle.hlsli"

struct EmitterSphere
{
    float3 translate;
    float radius;
    uint count;
    float frequency;
    float frequencyTime;
    uint emit;
};

ConstantBuffer<EmitterSphere> gEmitterSphere : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int> gFreeListIndex : register(u1);
RWStructuredBuffer<uint> gFreeList : register(u2);

float rand3dTo1d(float3 value, float3 dotDir = float3(12.9898, 78.233, 37.719));

float3 rand3dTo3d(float3 value);

class RandomGenerator
{
    float3 seed;
    float3 Generate3d()
    {
        seed = rand3dTo3d(seed);
        return seed;
    }
    float Generate1d()
    {
        float result = rand3dTo1d(seed);
        seed.x = result;
        return result;
    }
};

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (gEmitterSphere.emit == 0)
    {
        return;
    }
    
    RandomGenerator randomGenerator;
    randomGenerator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
    
    for (uint countIndex = 0; countIndex < gEmitterSphere.count; ++countIndex)
    {
        int freeListIndex;
        InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
        if (0 <= freeListIndex && freeListIndex < kMaxParticles)
        {
            uint particleIndex = gFreeList[freeListIndex];
            gParticles[particleIndex].scale = randomGenerator.Generate3d();
            gParticles[particleIndex].translate = randomGenerator.Generate3d() * 2.0f - 1.0f;
            gParticles[particleIndex].color.rgb = randomGenerator.Generate3d();
            gParticles[particleIndex].color.a = 1.0f;
            gParticles[particleIndex].lifeTime = randomGenerator.Generate1d();
            gParticles[particleIndex].velocity = randomGenerator.Generate3d() * 2.0f - 1.0f;
            gParticles[particleIndex].velocity *= gPerFrame.deltaTime;
            gParticles[particleIndex].currentTime = 0.0f;
        }
        else
        {
            InterlockedAdd(gFreeListIndex[0], 1);
            break;
        }
    }
}

float rand3dTo1d(float3 value, float3 dotDir)
{
    //make value smaller to avoid artefacts
    float3 smallValue = sin(value);
    //get scalar value from 3d vector
    float random = dot(smallValue, dotDir);
    //make value more random by making it bigger and then taking teh factional part
    random = frac(sin(random) * 143758.5453);
    return random;
}

float3 rand3dTo3d(float3 value)
{
    return float3(
        rand3dTo1d(value, float3(12.989, 78.233, 37.719)),
        rand3dTo1d(value, float3(39.346, 11.135, 83.155)),
        rand3dTo1d(value, float3(73.156, 52.235, 09.151))
    );
}