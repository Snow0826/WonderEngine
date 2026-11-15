#include "BoxFilterEffect.h"
#include "Device.h"
#include "World.h"
#include "ConstantBuffer.h"

void BoxFilterEffect::Bind() {
	Device *device = GetDevice();
	World *world = GetWorld();
	ID3D12GraphicsCommandList *commandList = device->GetCommandList();
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();

	commandList->SetGraphicsRootSignature(rootSignature_);
	commandList->SetPipelineState(pipelineState_);
	world->GetConstantBuffer(ConstantBufferType::kBoxFilterParam)->BindToGraphics(0, 0);
	gpuCbvSrvUavDescriptorHeap->BindToGraphics(1, world->GetGameRenderTextureSRVHandle());
}