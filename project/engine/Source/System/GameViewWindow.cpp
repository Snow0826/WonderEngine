#include "GameViewWindow.h"
#include "EntityComponentSystem.h"
#include "Device.h"
#include "World.h"
#include "Camera.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void GameViewWindow::Draw() {
#ifdef USE_IMGUI
	if (!isOpen_) {
		return;
	}

	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device_->GetGpuCbvSrvUavDescriptorHeap();
	if (ImGui::Begin("GameView", &isOpen_)) {
		float aspectRatio = 16.0f / 9.0f;
		registry_->ForEach<Camera, RenderingCamera>([&](uint32_t entity, Camera *camera, RenderingCamera *renderingCamera) {
			aspectRatio = camera->aspectRatio;
			}, exclude<Disabled>());
		ImVec2 avail = ImGui::GetContentRegionAvail();
		float width = avail.x;
		float height = width / aspectRatio;
		if (height > avail.y) {
			height = avail.y;
			width = height * aspectRatio;
		}
		D3D12_GPU_DESCRIPTOR_HANDLE gpuGameHandle = gpuCbvSrvUavDescriptorHeap->GetGPUDescriptorHandle(world_->GetPostEffectRenderTextureSRVHandle());
		ImGui::Image(static_cast<ImTextureID>(gpuGameHandle.ptr), ImVec2(width, height));
	}
	ImGui::End();
#endif // USE_IMGUI
}