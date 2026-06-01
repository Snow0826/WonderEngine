#include "HierarchyWindow.h"
#include "EntityComponentSystem.h"
#include "SelectionContext.h"
#include <string>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void HierarchyWindow::Draw() {
#ifdef USE_IMGUI
	if (ImGui::Begin("Hierarchy")) {
		for (uint32_t e = 0; e < registry_->GetEntityCount(); ++e) {
			bool selected = (e == selection_->selectedEntity);
			if (ImGui::Selectable(("Entity " + std::to_string(e)).c_str(), selected)) {
				selection_->selectedEntity = e;
			}
		}
	}
	ImGui::End();
#endif // USE_IMGUI
}
