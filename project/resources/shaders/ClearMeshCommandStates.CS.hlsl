#include "IndirectCommand.hlsli"

cbuffer Constants : register(b0)
{
    uint meshLODCount;
};

RWStructuredBuffer<MeshCommandState> meshCommandStates : register(u0); // UAV: Mesh Command States

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint meshId = DTid.x;
    if (meshId >= meshLODCount)
    {
        return;
    }
    
    meshCommandStates[meshId].commandIndex = kInvalidCommandIndex;
    meshCommandStates[meshId].instanceCount = 0;
}