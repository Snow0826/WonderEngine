#define NOMINMAX
#include "Transform.h"
#include "EntityComponentSystem.h"
#include "Vector2.h"
#include "Model.h"

void TransformSystem::MarkDirty(uint32_t entity) {
	registry_->AddComponent<DirtyTransform>(entity, DirtyTransform{});
	registry_->ForEach<Transform, HasParent>([&](uint32_t child, Transform *transform, HasParent *hasParent) {
		if (transform->parentEntity == entity) {
			MarkDirty(child);
		}
		}, exclude<Disabled>());
}

void TransformSystem::UpdateWorldMatrix() {
	registry_->ForEach<Transform, DirtyTransform>([&](uint32_t entity, Transform *transform, DirtyTransform *dirtyTransform) {
		if (!transform->pivot.isZero()) {
			// ピボット補正付きの変換
			Matrix4x4 toPivot = MakeTranslateMatrix(-transform->pivot);
			Matrix4x4 srt = MakeAffineMatrix(transform->scale, transform->rotate, transform->translate);
			Matrix4x4 fromPivot = MakeTranslateMatrix(transform->pivot);
			transform->worldMatrix = toPivot * srt * fromPivot;
		} else if (!transform->rotate.isZero()) {
			// オイラー角の変換
			transform->worldMatrix = MakeAffineMatrix(transform->scale, transform->rotate, transform->translate);
		} else if (transform->quaternion != Quaternion::IdentityQuaternion()) {
			// クォータニオンの変換
			transform->worldMatrix = MakeAffineMatrix(transform->scale, transform->quaternion, transform->translate);
		} else {
			// 回転行列の変換
			transform->worldMatrix = MakeAffineMatrix(transform->scale, transform->rotateMatrix, transform->translate);
		}

		// 親のワールド行列を適用
		if (registry_->HasComponent<HasParent>(entity)) {
			Transform *parentTransform = registry_->GetComponent<Transform>(transform->parentEntity);
			if (parentTransform) {
				transform->worldMatrix *= parentTransform->worldMatrix;
			}
		}
		}, exclude<Disabled>());
}

Vector3 TransformSystem::GetRight(uint32_t entity) {
	Transform *transform = registry_->GetComponent<Transform>(entity);
	if (!transform) {
		return Vector3{};
	}
	Vector3 right = { transform->worldMatrix.m[0][0], transform->worldMatrix.m[0][1], transform->worldMatrix.m[0][2] };
	return right.normalized();
}

Vector3 TransformSystem::GetUp(uint32_t entity) {
	Transform *transform = registry_->GetComponent<Transform>(entity);
	if (!transform) {
		return Vector3{};
	}
	Vector3 up = { transform->worldMatrix.m[1][0], transform->worldMatrix.m[1][1], transform->worldMatrix.m[1][2] };
	return up.normalized();
}

Vector3 TransformSystem::GetForward(uint32_t entity) {
	Transform *transform = registry_->GetComponent<Transform>(entity);
	if (!transform) {
		return Vector3{};
	}
	Vector3 forward = { transform->worldMatrix.m[2][0], transform->worldMatrix.m[2][1], transform->worldMatrix.m[2][2] };
	return forward.normalized();
}

Vector3 TransformSystem::GetWorldPosition(uint32_t entity) {
	Transform *transform = registry_->GetComponent<Transform>(entity);
	if (!transform) {
		return Vector3{};
	}
	Vector3 worldPosition = { transform->worldMatrix.m[3][0], transform->worldMatrix.m[3][1], transform->worldMatrix.m[3][2] };
	return worldPosition;
}

void TransformInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Transform")) {
		Transform *transform = registry_->GetComponent<Transform>(entity);
		if (transform) {
			TransformSystem transformSystem{ registry_ };
			if (ImGui::DragFloat3("scale", &transform->scale.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::SliderAngle("rotateX", &transform->rotate.x)) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::SliderAngle("rotateY", &transform->rotate.y)) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::SliderAngle("rotateZ", &transform->rotate.z)) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::DragFloat3("translate", &transform->translate.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::DragFloat3("pivot", &transform->pivot.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::DragFloat4("quaternion", &transform->quaternion.x, 0.001f, -1.0f, 1.0f)) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::TreeNode("RotateMatrix")) {
				for (size_t i = 0; i < 4; i++) {
					if (ImGui::DragFloat4(("Row " + std::to_string(i)).c_str(), &transform->rotateMatrix.m[i][0], 0.001f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
						transformSystem.MarkDirty(entity);
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("WorldMatrix")) {
				for (size_t i = 0; i < 4; i++) {
					if (ImGui::DragFloat4(("Row " + std::to_string(i)).c_str(), &transform->worldMatrix.m[i][0], 0.001f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
						transformSystem.MarkDirty(entity);
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::DragScalar("parentEntity", ImGuiDataType_U32, &transform->parentEntity, 1.0f, nullptr, nullptr)) {
				transformSystem.MarkDirty(entity);
			}

			if (ImGui::Button("Reset")) {
				*transform = Transform{};
				transformSystem.MarkDirty(entity);
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}