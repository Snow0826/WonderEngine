#define NOMINMAX
#include "DebugCamera.h"
#include "EntityComponentSystem.h"
#include "Camera.h"
#include "Transform.h"
#include "Input.h"
#include <algorithm>

void DebugCamera::Initialize(uint32_t cameraEntity) {
	cameraEntity_ = cameraEntity;
	Transform *transform = registry_->GetComponent<Transform>(cameraEntity_);
	if (transform) {
		resetPosition_ = transform->translate;
	}
	Reset();
}

void DebugCamera::Update() {
	Transform *transform = registry_->GetComponent<Transform>(cameraEntity_);
	if (!transform) {
		return;
	}

	// マウスの移動量を取得
	Vector3 move = input_->GetMouseMove();

	if (input_->IsPressMouse(2)) {
		if (input_->IsPressKey(DIK_LSHIFT)) {
			Vector3 forward = -ToCartesian(sphericalPosition_).normalized();
			Vector3 right = Vector3{ 0.0f, 1.0f, 0.0f }.cross(forward).normalized();
			Vector3 up = forward.cross(right).normalized();

			// カメラの移動
			targetPosition_ -= right * move.x * 0.1f;
			targetPosition_ += up * move.y * 0.1f;
		} else {
			// カメラの回転
			sphericalPosition_.y += move.x * std::numbers::pi_v<float> / 180.0f;
			sphericalPosition_.z -= move.y * std::numbers::pi_v<float> / 180.0f;
			sphericalPosition_.z = std::clamp(sphericalPosition_.z, 0.01f, std::numbers::pi_v<float> -0.01f); // 仰角の回転制限
		}
	}

	// カメラのズーム
	sphericalPosition_.x -= move.z * 0.01f;
	sphericalPosition_.x = std::max(sphericalPosition_.x, 1.0f); // 最小距離制限

	// カメラの位置を更新
	transform->translate = targetPosition_ + ToCartesian(sphericalPosition_);

	// カメラの回転を更新
	transform->rotateMatrix = LookAt(transform->translate, targetPosition_, { .y = 1.0f });

	// 変換フラグを立てる
	TransformSystem transformSystem{ registry_ };
	transformSystem.MarkDirty(cameraEntity_);
}

void DebugCamera::Reset() {
	targetPosition_ = { 0.0f, 0.0f, 0.0f };
	Transform *transform = registry_->GetComponent<Transform>(cameraEntity_);
	if (transform) {
		transform->translate = resetPosition_;
		transform->rotateMatrix = LookAt(transform->translate, targetPosition_, { .y = 1.0f });
		sphericalPosition_ = ToSpherical(transform->translate);
	}
}

void DebugCamera::Edit([[maybe_unused]] const std::string &label) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode(label.c_str())) {
		ImGui::DragFloat3("TargetPosition", &targetPosition_.x, 0.1f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		ImGui::DragFloat3("SphericalPosition", &sphericalPosition_.x, 0.1f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}