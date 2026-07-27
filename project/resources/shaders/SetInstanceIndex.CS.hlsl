#include "IndirectCommand.hlsli"

cbuffer Constants : register(b0)
{
    uint meshCount;
};

StructuredBuffer<MeshLODState> meshLODStates : register(t0); // SRV: Mesh LOD States
RWStructuredBuffer<MeshCommandState> meshCommandStates : register(u0); // UAV: Mesh Command States
RWStructuredBuffer<uint> processedInstanceIndices : register(u1); // UAV: Processed Instance Indices

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    if (DTid.x >= meshCount)
    {
        return;
    }
    
    MeshLODState meshLODState = meshLODStates[DTid.x];
    if (!meshLODState.visible)
    {
        return;
    }
    
    uint meshLODIndex = meshLODState.meshLODIndex;
    uint localIndex;
    InterlockedAdd(meshCommandStates[meshLODIndex].currentOffset, 1, localIndex);
    processedInstanceIndices[meshCommandStates[meshLODIndex].startInstanceLocation + localIndex] = meshLODState.instanceIndex;
}