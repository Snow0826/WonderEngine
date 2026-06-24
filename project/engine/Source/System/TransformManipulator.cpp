#include "TransformManipulator.h"
#include "EntityComponentSystem.h"
#include "Transform.h"
#include "Camera.h"
#include "SelectionContext.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <ImGuizmo.h>
#endif // USE_IMGUI

void TransformManipulator::Draw() {
#ifdef USE_IMGUI
	TransformSystem transformSystem{ registry_ };
	auto eulerTransform = registry_->GetComponent<EulerTransform>(selection_->selectedEntity);
	auto quaternionTransform = registry_->GetComponent<QuaternionTransform>(selection_->selectedEntity);
	registry_->ForEach<Camera, QuaternionTransform, RenderingCamera>([&](uint32_t cameraEntity, Camera *camera, QuaternionTransform *cameraQuaternionTransform, RenderingCamera *renderingCamera) {
		ViewProjectionData viewProjectionData = MakeViewProjection(*camera, *cameraQuaternionTransform);
		if (eulerTransform) {
			if (transformSystem.ManipulateTransform(&viewProjectionData.view.m[0][0], &viewProjectionData.projection.m[0][0], &eulerTransform->worldMatrix.m[0][0])) {
				Vector3 rotate;
				ImGuizmo::DecomposeMatrixToComponents(&eulerTransform->worldMatrix.m[0][0], &eulerTransform->translate.x, &rotate.x, &eulerTransform->scale.x);
				eulerTransform->rotate = rotate * (std::numbers::pi_v<float> / 180.0f);	// Convert degrees to radians
				transformSystem.MarkDirty(selection_->selectedEntity);
			}
		} else if (quaternionTransform) {
			if (transformSystem.ManipulateTransform(&viewProjectionData.view.m[0][0], &viewProjectionData.projection.m[0][0], &quaternionTransform->worldMatrix.m[0][0])) {
				Vector3 rotate;
				ImGuizmo::DecomposeMatrixToComponents(&quaternionTransform->worldMatrix.m[0][0], &quaternionTransform->translate.x, &rotate.x, &quaternionTransform->scale.x);
				quaternionTransform->rotate = Quaternion::EulerToQuaternion(rotate * (std::numbers::pi_v<float> / 180.0f));
				transformSystem.MarkDirty(selection_->selectedEntity);
			}
		}
		}, exclude<Disabled>());
	bool isOver = ImGuizmo::IsOver();
	ImGui::Checkbox("IsOver", &isOver);
	bool isUsing = ImGuizmo::IsUsing();
	ImGui::Checkbox("IsUsing", &isUsing);
#endif // USE_IMGUI
}