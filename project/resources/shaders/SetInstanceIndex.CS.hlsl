#include "IndirectCommand.hlsli"

cbuffer Constants : register(b0)
{
    uint objectCount;
};

StructuredBuffer<ObjectState> gObjectStates : register(t0); // SRV: Mesh LOD States
RWStructuredBuffer<IndirectCommandState> gIndirectCommandStates : register(u0); // UAV: Mesh Command States
RWStructuredBuffer<uint> gProcessedInstanceIndices : register(u1); // UAV: Processed Instance Indices

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint objectIndex = DTid.x;
    if (objectIndex >= objectCount)
    {
        return;
    }
    
    ObjectState objectState = gObjectStates[objectIndex];
    if (!objectState.visible)
    {
        return;
    }
    
    uint meshIndex = objectState.meshIndex;
    uint localIndex;
    InterlockedAdd(gIndirectCommandStates[meshIndex].currentOffset, 1, localIndex);
    gProcessedInstanceIndices[gIndirectCommandStates[meshIndex].instanceOffset + localIndex] = objectState.instanceIndex;
}