#define NOMINMAX
#include "UVTransform.h"
#include "EntityComponentSystem.h"

void UVTransformInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("UVTransform")) {
		UVTransform *uvTransform = registry_->GetComponent<UVTransform>(entity);
		if (uvTransform) {
			ImGui::DragFloat3("scale", &uvTransform->scale.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::SliderAngle("rotateX", &uvTransform->rotate.x);
			ImGui::SliderAngle("rotateY", &uvTransform->rotate.y);
			ImGui::SliderAngle("rotateZ", &uvTransform->rotate.z);
			ImGui::DragFloat3("translate", &uvTransform->translate.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());

			if (ImGui::Button("Reset")) {
				*uvTransform = UVTransform{};
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}