#define NOMINMAX
#include "UVTransform.h"
#include "EntityComponentSystem.h"
#include "Material.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void UVTransformInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("UVTransform")) {
		UVTransform *uvTransform = registry_->GetComponent<UVTransform>(entity);
		if (uvTransform) {
			if (ImGui::DragFloat3("scale", &uvTransform->scale.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				registry_->AddComponent(entity, DirtyMaterial{});
			}

			if (ImGui::SliderAngle("rotateX", &uvTransform->rotate.x)) {
				registry_->AddComponent(entity, DirtyMaterial{});
			}

			if (ImGui::SliderAngle("rotateY", &uvTransform->rotate.y)) {
				registry_->AddComponent(entity, DirtyMaterial{});
			}

			if (ImGui::SliderAngle("rotateZ", &uvTransform->rotate.z)) {
				registry_->AddComponent(entity, DirtyMaterial{});
			}

			if (ImGui::DragFloat3("translate", &uvTransform->translate.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max())) {
				registry_->AddComponent(entity, DirtyMaterial{});
			}

			if (ImGui::Button("Reset")) {
				*uvTransform = UVTransform{};
				registry_->AddComponent(entity, DirtyMaterial{});
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}