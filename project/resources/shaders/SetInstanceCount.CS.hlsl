#include "IndirectCommand.hlsli"

cbuffer Constants : register(b0)
{
    uint meshLODCount;
};

StructuredBuffer<MeshCommandState> meshCommandStates : register(t0); // SRV: Mesh Command States
RWStructuredBuffer<IndirectCommand> commands : register(u0); // UAV: Processed Indirect Commands

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint meshId = DTid.x;
    if (meshId >= meshLODCount)
    {
        return;
    }
    
    MeshCommandState state = meshCommandStates[meshId];
    if (state.commandIndex != kInvalidCommandIndex)
    {
        commands[state.commandIndex].drawIndexedArguments.InstanceCount = state.instanceCount;
    }
}