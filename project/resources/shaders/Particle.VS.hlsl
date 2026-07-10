#include "Particle.hlsli"

struct Particle
{
    float3 translate;
    float3 scale;
    float lifeTime;
    float3 velocity;
    float currentTime;
    float4 color;
};

StructuredBuffer<Particle> gParticles : register(t0);

cbuffer PerView : register(b0)
{
    float4x4 view;
    float4x4 projection;
    float4x4 billboard;
};

struct VertexShaderInput
{
    float4 position : POSITION0;
    float2 texcoord : TEXCOORD0;
    float3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    Particle particle = gParticles[instanceId];
    float4x4 world = billboard;
    world[0] *= particle.scale.x;
    world[1] *= particle.scale.y;
    world[2] *= particle.scale.z;
    world[3].xyz = particle.translate;
    output.position = mul(input.position, mul(world, mul(view, projection)));
    output.texcoord = input.texcoord;
    output.color = particle.color;
    return output;
}