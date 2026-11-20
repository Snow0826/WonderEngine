#include "IndirectCommand.hlsli"

cbuffer Constants : register(b0)
{
    uint meshCount;
};

RWStructuredBuffer<IndirectCommandState> gIndirectCommandStates : register(u0); // UAV: Mesh Command States

[numthreads(1, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint sum = 0;
    for (uint i = 0; i < meshCount; ++i)
    {
        gIndirectCommandStates[i].instanceOffset = sum;
        sum += gIndirectCommandStates [i].instanceCount;
    }
}