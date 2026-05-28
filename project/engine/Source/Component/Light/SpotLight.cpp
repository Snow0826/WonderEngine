#define NOMINMAX
#include "SpotLight.h"
#include "EntityComponentSystem.h"

void SpotLightInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("SpotLight")) {
		SpotLight *spotLight = registry_->GetComponent<SpotLight>(entity);
		if (spotLight) {
			ImGui::ColorEdit4("color", &spotLight->color.x);
			ImGui::DragFloat3("position", &spotLight->position.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat("intensity", &spotLight->intensity, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat3("direction", &spotLight->direction.x, 0.01f, -1.0f, 1.0f);
			ImGui::DragFloat("distance", &spotLight->distance, 0.01f, 0.0f, std::numeric_limits<float>::max());
			ImGui::DragFloat("decay", &spotLight->decay, 0.01f, 0.0f, std::numeric_limits<float>::max());
			spotLight->direction = spotLight->direction.normalized();

			SpotLightFalloff *spotLightFalloff = registry_->GetComponent<SpotLightFalloff>(entity);
			if (spotLightFalloff) {
				ImGui::SliderAngle("falloffStart", &spotLightFalloff->start, 0.0f, 90.0f);
				ImGui::SliderAngle("falloffEnd", &spotLightFalloff->end, 0.0f, 90.0f);
				spotLight->cosFalloffStart = std::cos(spotLightFalloff->start);
				spotLight->cosAngle = std::cos(spotLightFalloff->end);

				// もし同じ値になったら少しだけずらす
				constexpr float kEpsilon = 1e-4f;
				if (std::abs(spotLight->cosFalloffStart - spotLight->cosAngle) < kEpsilon) {
					spotLight->cosFalloffStart = spotLight->cosAngle + kEpsilon;
				}

				if (ImGui::Button("Reset")) {
					*spotLight = SpotLight{};
					*spotLightFalloff = SpotLightFalloff{};
				}
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}