#define NOMINMAX
#include "Transform.h"
#include "EntityComponentSystem.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void TransformSystem::MarkDirty(uint32_t entity) {
	registry_->AddComponent<DirtyTransform>(entity, DirtyTransform{});
	registry_->ForEach<EulerTransform, HasParent>([&](uint32_t child, EulerTransform *transform, HasParent *hasParent) {
		if (transform->parentEntity == entity) {
			MarkDirty(child);
		}
		}, exclude<Disabled>());
	registry_->ForEach<QuaternionTransform, HasParent>([&](uint32_t child, QuaternionTransform *transform, HasParent *hasParent) {
		if (transform->parentEntity == entity) {
			MarkDirty(child);
		}
		}, exclude<Disabled>());
}

void TransformSystem::UpdateWorldMatrix() {
	registry_->ForEach<EulerTransform, DirtyTransform>([&](uint32_t entity, EulerTransform *transform, DirtyTransform *dirtyTransform) {
		// 回転行列の更新
		if (!transform->rotate.isZero()) {
			transform->rotateMatrix = MakeRotateMatrix(transform->rotate);
		}

		// ピボット補正付きの変換
		Matrix4x4 toPivot = MakeTranslateMatrix(-transform->pivot);
		Matrix4x4 srt = MakeAffineMatrix(transform->scale, transform->rotateMatrix, transform->translate);
		Matrix4x4 fromPivot = MakeTranslateMatrix(transform->pivot);
		transform->worldMatrix = toPivot * srt * fromPivot;

		// 親のワールド行列を適用
		if (registry_->HasComponent<HasParent>(entity)) {
			EulerTransform *parentTransform = registry_->GetComponent<EulerTransform>(transform->parentEntity);
			if (parentTransform) {
				transform->worldMatrix *= parentTransform->worldMatrix;
			}
		}
		}, exclude<Disabled>());

	registry_->ForEach<QuaternionTransform, DirtyTransform>([&](uint32_t entity, QuaternionTransform *transform, DirtyTransform *dirtyTransform) {
		// 回転行列の更新
		if (transform->rotate != Quaternion::IdentityQuaternion()) {
			transform->rotateMatrix = transform->rotate.MakeRotateMatrix();
		}

		// ピボット補正付きの変換
		Matrix4x4 toPivot = MakeTranslateMatrix(-transform->pivot);
		Matrix4x4 srt = MakeAffineMatrix(transform->scale, transform->rotateMatrix, transform->translate);
		Matrix4x4 fromPivot = MakeTranslateMatrix(transform->pivot);
		transform->worldMatrix = toPivot * srt * fromPivot;

		// 親のワールド行列を適用
		if (registry_->HasComponent<HasParent>(entity)) {
			QuaternionTransform *parentTransform = registry_->GetComponent<QuaternionTransform>(transform->parentEntity);
			if (parentTransform) {
				transform->worldMatrix *= parentTransform->worldMatrix;
			}
		}
		}, exclude<Disabled>());
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

			if (ImGui::DragScalar("parentEntity", ImGuiDataType_U32, &eulerTransform->parentEntity, 1.0f, nullptr, nullptr)) {
				transformSystem.MarkDirty(entity);
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

			if (ImGui::DragScalar("parentEntity", ImGuiDataType_U32, &quaternionTransform->parentEntity, 1.0f, nullptr, nullptr)) {
				transformSystem.MarkDirty(entity);
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