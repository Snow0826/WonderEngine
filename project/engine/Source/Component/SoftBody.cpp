#define NOMINMAX
#include "SoftBody.h"
#include "EntityComponentSystem.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void SoftBodyInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (auto softBody = registry_->GetComponent<SoftBodyDataForCPU>(entity)) {
		if (ImGui::TreeNode("SoftBody")) {
			ImGui::DragFloat("ImpactVelocity", &softBody->impactVelocity, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("HitTime", &softBody->hitTime, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("ImpactScale", &softBody->impactScale, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("MaxImpact", &softBody->maxImpact, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("Damping", &softBody->damping, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("Frequency", &softBody->frequency, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("SquashAmount", &softBody->squashAmount, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("ExpandAmount", &softBody->expandAmount, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("LowerExpandWeight", &softBody->lowerExpandWeight, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("UpperExpandWeight", &softBody->upperExpandWeight, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::Checkbox("WasGrounded", &softBody->wasGrounded);
			ImGui::Checkbox("IsGrounded", &softBody->isGrounded);
			if (ImGui::Button("Reset")) {
				*softBody = SoftBodyDataForCPU{};
			}
			ImGui::TreePop();
		}
	}
#endif // USE_IMGUI
}