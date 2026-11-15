#include "FullScreenEffect.h"
#include "Device.h"
#include "World.h"

void FullScreenEffect::Bind() {
	Device *device = GetDevice();
	World *world = GetWorld();
	ID3D12GraphicsCommandList *commandList = device->GetCommandList();
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();

	commandList->SetGraphicsRootSignature(rootSignature_);
	commandList->SetPipelineState(pipelineState_);
	gpuCbvSrvUavDescriptorHeap->BindToGraphics(0, world->GetGameRenderTextureSRVHandle());
}