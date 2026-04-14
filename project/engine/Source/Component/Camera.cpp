#include "Camera.h"
#include "EntityComponentSystem.h"
#include "Transform.h"

ViewProjectionData MakeViewProjection(const Camera &camera, const Transform &transform) {
	ViewProjectionData viewProjection;
	viewProjection.view = transform.worldMatrix.inverse();
	viewProjection.projection = MakePerspectiveFovMatrix(camera.fovY, camera.aspectRatio, camera.nearZ, camera.farZ);
	return viewProjection;
}

void CameraSystem::SwitchCamera(uint32_t cameraEntity) {
	SwitchRenderingCamera(cameraEntity);

	// CullingCameraコンポーネントを無効化
	registry_->ForEach<CullingCamera>([this](uint32_t entity, CullingCamera *cullingCamera) {
		registry_->RemoveComponent<CullingCamera>(entity);
		}, exclude<Disabled>());

	// 指定されたカメラにCullingCameraコンポーネントを有効化
	registry_->AddComponent(cameraEntity, CullingCamera{});
}

void CameraSystem::SwitchRenderingCamera(uint32_t cameraEntity) {
	// RenderingCameraコンポーネントを無効化
	registry_->ForEach<RenderingCamera>([this](uint32_t entity, RenderingCamera *renderingCamera) {
		registry_->RemoveComponent<RenderingCamera>(entity);
		}, exclude<Disabled>());

	// 指定されたカメラにRenderingCameraコンポーネントを有効化
	registry_->AddComponent(cameraEntity, RenderingCamera{});
}

uint32_t CameraSystem::GetCullingCameraEntity() const {
	uint32_t cullingCameraEntity = 0;
	registry_->ForEach<CullingCamera>([this, &cullingCameraEntity](uint32_t entity, CullingCamera *cullingCamera) {
		cullingCameraEntity = entity;
		}, exclude<Disabled>());
	return cullingCameraEntity;
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