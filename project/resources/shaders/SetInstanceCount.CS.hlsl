#include "IndirectCommand.hlsli"

cbuffer Constants : register(b0)
{
    uint meshCount;
};

StructuredBuffer<IndirectCommandState> gIndirectCommandStates : register(t0); // SRV: Mesh Command States
RWStructuredBuffer<IndirectCommand> gIndirectCommands : register(u0); // UAV: Processed Indirect Commands

[numthreads(64, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint meshIndex = DTid.x;
    if (meshIndex >= meshCount)
    {
        return;
    }
    
    IndirectCommandState state = gIndirectCommandStates[meshIndex];
    if (state.commandIndex != kInvalidCommandIndex)
    {
        gIndirectCommands[state.commandIndex].instanceOffset = state.instanceOffset;
        gIndirectCommands[state.commandIndex].drawIndexedArguments.InstanceCount = state.instanceCount;
    }
}