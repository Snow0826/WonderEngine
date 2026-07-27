#include "Camera.h"
#include "EntityComponentSystem.h"
#include "Transform.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

ViewProjectionData MakeViewProjection(const Camera &camera, const QuaternionTransform &transform) {
	ViewProjectionData viewProjection;
	viewProjection.view = transform.worldMatrix.inverse();
	viewProjection.projection = MakePerspectiveFovMatrix(camera.fovY, camera.aspectRatio, camera.nearZ, camera.farZ);
	return viewProjection;
}

void CameraSystem::SwitchRenderingCamera(uint32_t cameraEntity) {
	// RenderingCameraコンポーネントを無効化
	registry_->ClearComponent<RenderingCamera>();

	// 指定されたカメラにRenderingCameraコンポーネントを有効化
	registry_->AddComponent(cameraEntity, RenderingCamera{});
}

void CameraInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Camera")) {
		Camera *camera = registry_->GetComponent<Camera>(entity);
		if (camera) {
			ImGui::DragFloat("fovY", &camera->fovY, 0.01f, 1.0f, 179.0f);
			ImGui::DragFloat("aspectRatio", &camera->aspectRatio, 0.01f, 0.1f, 10.0f);
			ImGui::DragFloat("nearZ", &camera->nearZ, 0.01f, 0.01f, camera->farZ - 0.01f);
			ImGui::DragFloat("farZ", &camera->farZ, 0.1f, camera->nearZ + 0.1f, 1000.0f);

			if (ImGui::Button("Reset")) {
				*camera = Camera{};
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}