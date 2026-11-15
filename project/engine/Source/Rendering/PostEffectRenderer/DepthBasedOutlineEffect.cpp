#include "DepthBasedOutlineEffect.h"
#include "Device.h"
#include "World.h"
#include "ConstantBuffer.h"

void DepthBasedOutlineEffect::Bind() {
	Device *device = GetDevice();
	World *world = GetWorld();
	ID3D12GraphicsCommandList *commandList = device->GetCommandList();
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();

	commandList->SetGraphicsRootSignature(rootSignature_);
	commandList->SetPipelineState(pipelineState_);
	world->GetConstantBuffer(ConstantBufferType::kPrewittFilterParam)->BindToGraphics(0, 1);
	world->GetConstantBuffer(ConstantBufferType::kDepthMaterial)->BindToGraphics(1, 0);
	gpuCbvSrvUavDescriptorHeap->BindToGraphics(2, world->GetGameRenderTextureSRVHandle());
	gpuCbvSrvUavDescriptorHeap->BindToGraphics(3, world->GetMainCameraDepthStencilTextureSRVHandle());
}