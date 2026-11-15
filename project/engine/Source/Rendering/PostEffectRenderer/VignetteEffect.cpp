#include "VignetteEffect.h"
#include "Device.h"
#include "World.h"
#include "ConstantBuffer.h"

void VignetteEffect::Bind() {
	Device *device = GetDevice();
	World *world = GetWorld();
	ID3D12GraphicsCommandList *commandList = device->GetCommandList();
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();

	commandList->SetGraphicsRootSignature(rootSignature_);
	commandList->SetPipelineState(pipelineState_);
	world->GetConstantBuffer(ConstantBufferType::kVignetteParam)->BindToGraphics(0, 0);
	gpuCbvSrvUavDescriptorHeap->BindToGraphics(1, world->GetGameRenderTextureSRVHandle());
}