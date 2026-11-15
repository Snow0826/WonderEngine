#include "DissolveEffect.h"
#include "Device.h"
#include "World.h"
#include "Texture.h"
#include "ConstantBuffer.h"

void DissolveEffect::Bind() {
	Device *device = GetDevice();
	World *world = GetWorld();
	ID3D12GraphicsCommandList *commandList = device->GetCommandList();
	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device->GetGpuCbvSrvUavDescriptorHeap();

	commandList->SetGraphicsRootSignature(rootSignature_);
	commandList->SetPipelineState(pipelineState_);
	world->GetConstantBuffer(ConstantBufferType::kDissolveParam)->BindToGraphics(0, 0);
	gpuCbvSrvUavDescriptorHeap->BindToGraphics(1, world->GetGameRenderTextureSRVHandle());
	gpuCbvSrvUavDescriptorHeap->BindToGraphics(2, textureManager_->GetTextureReadHandle("noise0.png"));
}