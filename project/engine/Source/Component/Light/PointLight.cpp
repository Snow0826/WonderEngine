#define NOMINMAX
#include "PointLight.h"
#include "EntityComponentSystem.h"

void PointLightInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("PointLight")) {
		PointLight *pointLight = registry_->GetComponent<PointLight>(entity);
		if (pointLight) {
			ImGui::ColorEdit4("color", &pointLight->color.x);
			ImGui::DragFloat3("position", &pointLight->position.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat("intensity", &pointLight->intensity, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("radius", &pointLight->radius, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("decay", &pointLight->decay, 0.01f, 0.0f, std::numeric_limits<float>::max());

			if (ImGui::Button("Reset")) {
				*pointLight = PointLight{};
			}
		}

		ImGui::TreePop();
	}
#endif // USE_IMGUI
}