#include "SceneViewWindow.h"
#include "EntityComponentSystem.h"
#include "TransformManipulator.h"
#include "Device.h"
#include "World.h"
#include "Camera.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <ImGuizmo.h>
#endif // USE_IMGUI

SceneViewWindow::SceneViewWindow(Registry *registry, Device *device, World *world, SelectionContext *selection) : registry_(registry), device_(device), world_(world), selection_(selection) {
	// 変換マニピュレーターの生成
	transformManipulator_ = std::make_unique<TransformManipulator>(registry_, selection_);
}

SceneViewWindow::~SceneViewWindow() = default;

void SceneViewWindow::Draw() {
#ifdef USE_IMGUI
	if (!isOpen_) {
		return;
	}

	DescriptorHeap *gpuCbvSrvUavDescriptorHeap = device_->GetGpuCbvSrvUavDescriptorHeap();
	if (ImGui::Begin("SceneView", &isOpen_)) {
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
		D3D12_GPU_DESCRIPTOR_HANDLE gpuSceneHandle = gpuCbvSrvUavDescriptorHeap->GetGPUDescriptorHandle(world_->GetSceneRenderTextureSRVHandle());
		ImGui::Image(static_cast<ImTextureID>(gpuSceneHandle.ptr), ImVec2(width, height));
		ImVec2 pos = ImGui::GetItemRectMin();
		ImVec2 size = ImGui::GetItemRectSize();
		ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

		// 変換マニピュレーターの描画
		transformManipulator_->Draw();
	}
	ImGui::End();
#endif // USE_IMGUI
}