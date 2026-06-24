#define NOMINMAX
#include "Transform.h"
#include "EntityComponentSystem.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <ImGuizmo.h>
#endif // USE_IMGUI

void TransformSystem::Update() {
	registry_->ForEach<Relationship>([&](uint32_t entity, Relationship *relationship) {
		if (relationship->parent == UINT_MAX) {
			UpdateWorldMatrix(entity, MakeIdentity4x4());
		}
		}, exclude<Disabled>());
}

void TransformSystem::MarkDirty(uint32_t entity) {
	if (!registry_->HasComponent<DirtyTransform>(entity)) {
		registry_->AddComponent(entity, DirtyTransform{});
	}

	if (auto relationship = registry_->GetComponent<Relationship>(entity)) {
		for (uint32_t child : relationship->children) {
			MarkDirty(child);
		}
	}
}

bool TransformSystem::ManipulateTransform([[maybe_unused]] float *cameraView, [[maybe_unused]] float *cameraProjection, [[maybe_unused]] float *matrix) {
#ifdef USE_IMGUI
	static ImGuizmo::OPERATION currentGizmoOperation(ImGuizmo::ROTATE);
	static ImGuizmo::MODE currentGizmoMode(ImGuizmo::WORLD);
	if (ImGui::IsKeyPressed(ImGuiKey_T)) {
		currentGizmoOperation = ImGuizmo::TRANSLATE;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_E)) {
		currentGizmoOperation = ImGuizmo::ROTATE;
	}

	if (ImGui::IsKeyPressed(ImGuiKey_R)) {
		currentGizmoOperation = ImGuizmo::SCALE;
	}

	if (ImGui::RadioButton("Translate", currentGizmoOperation == ImGuizmo::TRANSLATE)) {
		currentGizmoOperation = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();

	if (ImGui::RadioButton("Rotate", currentGizmoOperation == ImGuizmo::ROTATE)) {
		currentGizmoOperation = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();

	if (ImGui::RadioButton("Scale", currentGizmoOperation == ImGuizmo::SCALE)) {
		currentGizmoOperation = ImGuizmo::SCALE;
	}

	if (currentGizmoOperation != ImGuizmo::SCALE) {
		if (ImGui::RadioButton("Local", currentGizmoMode == ImGuizmo::LOCAL)) {
			currentGizmoMode = ImGuizmo::LOCAL;
		}
		ImGui::SameLine();
		if (ImGui::RadioButton("World", currentGizmoMode == ImGuizmo::WORLD)) {
			currentGizmoMode = ImGuizmo::WORLD;
		}
	}

	static bool useSnap(false);
	if (ImGui::IsKeyPressed(ImGuiKey_S)) {
		useSnap = !useSnap;
	}
	ImGui::Checkbox("##useSnap", &useSnap);
	ImGui::SameLine();
	static Vector3 snap;
	switch (currentGizmoOperation) {
		case ImGuizmo::TRANSLATE:
			ImGui::DragFloat3("Snap", &snap.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			break;
		case ImGuizmo::ROTATE:
			ImGui::SliderAngle("AngleSnap", &snap.x);
			break;
		case ImGuizmo::SCALE:
			ImGui::DragFloat("ScaleSnap", &snap.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			break;
		default:
			break;
	}
	ImGuizmo::Enable(true);
	ImGuizmo::SetDrawlist();
	return ImGuizmo::Manipulate(cameraView, cameraProjection, currentGizmoOperation, currentGizmoMode, matrix, nullptr, useSnap ? &snap.x : nullptr);
#endif // USE_IMGUI
}

Vector3 TransformSystem::GetRight(uint32_t entity) {
	EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
	QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
	if (!eulerTransform && !quaternionTransform) {
		return Vector3{};
	}
	Vector3 right;
	if (eulerTransform) {
		right = { eulerTransform->worldMatrix.m[0][0], eulerTransform->worldMatrix.m[0][1], eulerTransform->worldMatrix.m[0][2] };
	} else {
		right = { quaternionTransform->worldMatrix.m[0][0], quaternionTransform->worldMatrix.m[0][1], quaternionTransform->worldMatrix.m[0][2] };
	}
	return right.normalized();
}

Vector3 TransformSystem::GetUp(uint32_t entity) {
	EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
	QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
	if (!eulerTransform && !quaternionTransform) {
		return Vector3{};
	}
	Vector3 up;
	if (eulerTransform) {
		up = { eulerTransform->worldMatrix.m[1][0], eulerTransform->worldMatrix.m[1][1], eulerTransform->worldMatrix.m[1][2] };
	} else {
		up = { quaternionTransform->worldMatrix.m[1][0], quaternionTransform->worldMatrix.m[1][1], quaternionTransform->worldMatrix.m[1][2] };
	}
	return up.normalized();
}

Vector3 TransformSystem::GetForward(uint32_t entity) {
	EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
	QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
	if (!eulerTransform && !quaternionTransform) {
		return Vector3{};
	}
	Vector3 forward;
	if (eulerTransform) {
		forward = { eulerTransform->worldMatrix.m[2][0], eulerTransform->worldMatrix.m[2][1], eulerTransform->worldMatrix.m[2][2] };
	} else {
		forward = { quaternionTransform->worldMatrix.m[2][0], quaternionTransform->worldMatrix.m[2][1], quaternionTransform->worldMatrix.m[2][2] };
	}
	return forward.normalized();
}

Vector3 TransformSystem::GetWorldPosition(uint32_t entity) {
	EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
	QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
	if (!eulerTransform && !quaternionTransform) {
		return Vector3{};
	}
	Vector3 worldPosition;
	if (eulerTransform) {
		worldPosition = { eulerTransform->worldMatrix.m[3][0], eulerTransform->worldMatrix.m[3][1], eulerTransform->worldMatrix.m[3][2] };
	} else {
		worldPosition = { quaternionTransform->worldMatrix.m[3][0], quaternionTransform->worldMatrix.m[3][1], quaternionTransform->worldMatrix.m[3][2] };
	}
	return worldPosition;
}

void TransformSystem::UpdateWorldMatrix(uint32_t entity, const Matrix4x4 &parentWorldMatrix) {
	Matrix4x4 currentWorldMatrix = parentWorldMatrix;
	if (auto eulerTransform = registry_->GetComponent<EulerTransform>(entity)) {
		// 回転行列の更新
		if (!eulerTransform->rotate.isZero()) {
			eulerTransform->rotateMatrix = MakeRotateMatrix(eulerTransform->rotate);
		}

		// ピボット補正付きの変換
		Matrix4x4 toPivot = MakeTranslateMatrix(-eulerTransform->pivot);
		Matrix4x4 srt = MakeAffineMatrix(eulerTransform->scale, eulerTransform->rotateMatrix, eulerTransform->translate);
		Matrix4x4 fromPivot = MakeTranslateMatrix(eulerTransform->pivot);
		Matrix4x4 local = toPivot * srt * fromPivot;
		eulerTransform->worldMatrix = local * parentWorldMatrix;
		currentWorldMatrix = eulerTransform->worldMatrix;
	} else if (auto quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity)) {
		// 回転行列の更新
		if (quaternionTransform->rotate != Quaternion::IdentityQuaternion()) {
			quaternionTransform->rotateMatrix = quaternionTransform->rotate.MakeRotateMatrix();
		}

		// ピボット補正付きの変換
		Matrix4x4 toPivot = MakeTranslateMatrix(-quaternionTransform->pivot);
		Matrix4x4 srt = MakeAffineMatrix(quaternionTransform->scale, quaternionTransform->rotateMatrix, quaternionTransform->translate);
		Matrix4x4 fromPivot = MakeTranslateMatrix(quaternionTransform->pivot);
		Matrix4x4 local = toPivot * srt * fromPivot;
		quaternionTransform->worldMatrix = local * parentWorldMatrix;
		currentWorldMatrix = quaternionTransform->worldMatrix;
	}

	// 子エンティティのワールド行列を更新
	if (auto relationship = registry_->GetComponent<Relationship>(entity)) {
		for (uint32_t child : relationship->children) {
			UpdateWorldMatrix(child, currentWorldMatrix);
		}
	}
}

void TransformInspector::DrawEulerTransform([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	EulerTransform *eulerTransform = registry_->GetComponent<EulerTransform>(entity);
	if (eulerTransform) {
		if (ImGui::TreeNode("EulerTransform")) {
			TransformSystem transformSystem{ registry_ };
			if (ImGui::DragFloat3("scale", &eulerTransform->scale.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::SliderAngle("rotateX", &eulerTransform->rotate.x)) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::SliderAngle("rotateY", &eulerTransform->rotate.y)) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::SliderAngle("rotateZ", &eulerTransform->rotate.z)) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::DragFloat3("translate", &eulerTransform->translate.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::DragFloat3("pivot", &eulerTransform->pivot.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::TreeNode("RotateMatrix")) {
				for (size_t i = 0; i < 4; i++) {
					if (ImGui::DragFloat4(("Row " + std::to_string(i)).c_str(), &eulerTransform->rotateMatrix.m[i][0], 0.001f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
						transformSystem.MarkDirty(entity);
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("WorldMatrix")) {
				for (size_t i = 0; i < 4; i++) {
					if (ImGui::DragFloat4(("Row " + std::to_string(i)).c_str(), &eulerTransform->worldMatrix.m[i][0], 0.001f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
						transformSystem.MarkDirty(entity);
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::Button("Reset")) {
				*eulerTransform = EulerTransform{};
				transformSystem.MarkDirty(entity);
			}
			ImGui::TreePop();
		}
	}
#endif // USE_IMGUI
}

void TransformInspector::DrawQuaternionTransform([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	QuaternionTransform *quaternionTransform = registry_->GetComponent<QuaternionTransform>(entity);
	if (quaternionTransform) {
		if (ImGui::TreeNode("QuaternionTransform")) {
			TransformSystem transformSystem{ registry_ };
			if (ImGui::DragFloat3("scale", &quaternionTransform->scale.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::DragFloat4("rotate", &quaternionTransform->rotate.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::DragFloat3("translate", &quaternionTransform->translate.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::DragFloat3("pivot", &quaternionTransform->pivot.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::TreeNode("RotateMatrix")) {
				for (size_t i = 0; i < 4; i++) {
					if (ImGui::DragFloat4(("Row " + std::to_string(i)).c_str(), &quaternionTransform->rotateMatrix.m[i][0], 0.001f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
						transformSystem.MarkDirty(entity);
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("WorldMatrix")) {
				for (size_t i = 0; i < 4; i++) {
					if (ImGui::DragFloat4(("Row " + std::to_string(i)).c_str(), &quaternionTransform->worldMatrix.m[i][0], 0.001f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
						transformSystem.MarkDirty(entity);
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::Button("Reset")) {
				*quaternionTransform = QuaternionTransform{};
				transformSystem.MarkDirty(entity);
			}
			ImGui::TreePop();
		}
	}
#endif // USE_IMGUI
}