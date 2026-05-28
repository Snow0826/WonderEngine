#define NOMINMAX
#include "DirectionalLight.h"
#include "EntityComponentSystem.h"

void DirectionalLightInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("DirectionalLight")) {
		DirectionalLight *directionalLight = registry_->GetComponent<DirectionalLight>(entity);
		if (directionalLight) {
			ImGui::ColorEdit4("color", &directionalLight->color.x);
			ImGui::DragFloat3("direction", &directionalLight->direction.x, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("intensity", &directionalLight->intensity, 0.01f, 0.0f, std::numeric_limits<float>::max());
			directionalLight->direction = directionalLight->direction.normalized();

			if (ImGui::Button("Reset")) {
				*directionalLight = DirectionalLight{};
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}