#include "InspectorWindow.h"
#include "EntityComponentSystem.h"
#include "ComponentDrawerRegistry.h"
#include "SelectionContext.h"
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void InspectorWindow::Draw() {
#ifdef USE_IMGUI
	if (selection_->selectedEntity == UINT32_MAX || !isOpen_) {
		return;
	}

	if (ImGui::Begin("Inspector", &isOpen_)) {
		ImGui::Text("Entity %u", selection_->selectedEntity);
		DrawComponents(selection_->selectedEntity);
		DrawTagComponents(selection_->selectedEntity);
	}
	ImGui::End();
#endif // USE_IMGUI
}

void InspectorWindow::DrawComponents(uint32_t entity) {
	const auto &componentDrawers = componentDrawerRegistry_->GetComponentDrawers();
	for (size_t i = 0; i < componentDrawers.size(); ++i) {
		if (!componentDrawers[i]) {
			continue;
		}

		if (!registry_->HasComponentById(i, entity)) {
			continue;
		}

		componentDrawers[i](entity);
	}
}

void InspectorWindow::DrawTagComponents([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	const auto &tagComponents = componentDrawerRegistry_->GetTagComponents();
	for (size_t i = 0; i < tagComponents.size(); i++) {
		if (tagComponents[i].empty()) {
			continue;
		}

		bool visible = registry_->HasComponentById(i, entity);
		if (ImGui::Checkbox(tagComponents[i].c_str(), &visible)) {
			if (visible) {
				registry_->AddComponentById(i, entity);
			} else {
				registry_->RemoveComponentById(i, entity);
			}
		}
	}
#endif // USE_IMGUI
}
