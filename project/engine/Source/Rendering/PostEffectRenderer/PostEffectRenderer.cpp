#include "PostEffectRenderer.h"
#include "Device.h"
#include "World.h"
#include "Resource.h"

void PostEffectRenderer::Render() {
	ID3D12GraphicsCommandList *commandList = device_->GetCommandList();

	world_->GetGameRenderTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	device_->GetMainCameraDepthStencilTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	Bind();

	commandList->DrawInstanced(3, 1, 0, 0);
	device_->GetMainCameraDepthStencilTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_DEPTH_WRITE);
	world_->GetGameRenderTexture()->TransitionBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET);
}