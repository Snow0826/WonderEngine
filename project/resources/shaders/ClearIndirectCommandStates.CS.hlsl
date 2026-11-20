#include "IndirectCommand.hlsli"

cbuffer Constants : register(b0)
{
    uint meshCount;
};

RWStructuredBuffer<IndirectCommandState> gIndirectCommandStates : register(u0); // UAV: Mesh Command States

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint meshIndex = DTid.x;
    if (meshIndex >= meshCount)
    {
        return;
    }
    
    gIndirectCommandStates[meshIndex].commandIndex = kInvalidCommandIndex;
    gIndirectCommandStates[meshIndex].instanceCount = 0;
    gIndirectCommandStates[meshIndex].currentOffset = 0;
    gIndirectCommandStates[meshIndex].instanceOffset = 0;
}