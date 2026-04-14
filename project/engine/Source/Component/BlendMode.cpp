#include "BlendMode.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"

namespace {
	std::vector<std::string> blendModeNames = {
		"NoneBlend",
		"NormalBlend",
		"AdditiveBlend",
		"SubtractiveBlend",
		"MultiplicativeBlend",
		"ScreenBlend"
	};
}

void BlendModeInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	BlendMode *blendMode = registry_->GetComponent<BlendMode>(entity);
	if (blendMode) {
		if (ImGui::BeginCombo("BlendMode", blendModeNames[static_cast<size_t>(*blendMode)].c_str())) {
			for (size_t i = 0; i < blendModeNames.size(); i++) {
				bool selected = (blendModeNames[i] == blendModeNames[static_cast<size_t>(*blendMode)]);
				if (ImGui::Selectable(blendModeNames[i].c_str(), selected)) {
					*blendMode = static_cast<BlendMode>(i);
					indirectCommandManager_->SetBlendModeData(entity);
				}
				if (selected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}
	}
#endif // USE_IMGUI
}