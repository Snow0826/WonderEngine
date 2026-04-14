#define NOMINMAX
#include "CameraController.h"
#include "EntityComponentSystem.h"
#include "Transform.h"
#include "Input.h"
#include "Easing.h"
#include <algorithm>
#include <numbers>

void CameraController::Initialize(uint32_t cameraEntity, uint32_t targetEntity) {
	cameraEntity_ = cameraEntity;
	targetEntity_ = targetEntity;
	Transform *cameraTransform = registry_->GetComponent<Transform>(cameraEntity_);
	Transform *targetTransform = registry_->GetComponent<Transform>(targetEntity_);

	// カメラの球面座標を取得
	if (cameraTransform && targetTransform) {
		sphericalPosition_ = ToSpherical(cameraTransform->translate - targetTransform->translate);
		targetPosition_ = targetTransform->translate;
	}
}

void CameraController::Update() {
	Transform *cameraTransform = registry_->GetComponent<Transform>(cameraEntity_);
	Transform *targetTransform = registry_->GetComponent<Transform>(targetEntity_);
	if (!cameraTransform || !targetTransform) {
		return;
	}

	// マウスの移動量を取得
	Vector3 move = input_->GetMouseMove();

	// コントローラー入力による回転
	if (input_->Connected()) {
		// コントローラーの右スティックの値（正規化済み）
		XINPUT_STATE joyStickState = input_->GetJoyStickState();
		move.x = input_->NormalizeAxis(joyStickState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		move.y = input_->NormalizeAxis(joyStickState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
		move *= 2.0f; // コントローラーの感度調整
	} else {
#ifdef USE_IMGUI
		ImGui::Text("No Controller Connected");
#endif // USE_IMGUI
	}

	// カメラの回転
	sphericalPosition_.y += move.x * std::numbers::pi_v<float> / 180.0f;
	sphericalPosition_.z -= move.y * std::numbers::pi_v<float> / 180.0f;
	sphericalPosition_.z = std::clamp(sphericalPosition_.z, 0.01f, std::numbers::pi_v<float> -0.01f); // 仰角の回転制限

	// カメラの位置を更新
	targetPosition_ = Easing<Vector3>::Lerp(targetPosition_, targetTransform->translate, 0.1f);
	cameraTransform->translate = targetPosition_ + ToCartesian(sphericalPosition_);

	// カメラの向きを更新
	Quaternion quaternion = Quaternion::LookRotation(targetPosition_ - cameraTransform->translate, { .y = 1.0f });
	cameraTransform->quaternion = Quaternion::Slerp(cameraTransform->quaternion, quaternion, 0.1f);

	// 変換フラグを立てる
	TransformSystem transformSystem{ registry_ };
	transformSystem.MarkDirty(cameraEntity_);

#ifdef USE_IMGUI
	ImGui::DragFloat3("CameraSphericalPosition", &sphericalPosition_.x, 0.1f, 0.0f, 0.0f, "%.3f");
	ImGui::DragFloat3("CameraTargetPosition", &targetPosition_.x, 0.1f, 0.0f, 0.0f, "%.3f");
#endif // USE_IMGUI
}