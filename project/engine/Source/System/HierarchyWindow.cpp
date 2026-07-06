#include "HierarchyWindow.h"
#include "EntityComponentSystem.h"
#include "SelectionContext.h"
#include "Transform.h"
#include <string>
#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void HierarchyWindow::Draw() {
#ifdef USE_IMGUI
	if (!isOpen_) {
		return;
	}

	if (ImGui::Begin("Hierarchy", &isOpen_)) {
		for (uint32_t e = 0; e < registry_->GetEntityCount(); ++e) {
			auto relationship = registry_->GetComponent<Relationship>(e);
			if (relationship && relationship->parent == std::numeric_limits<uint32_t>::max()) {
				DrawEntity(e);
			}
		}
	}
	ImGui::End();
#endif // USE_IMGUI
}

void HierarchyWindow::DrawEntity([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	auto relationship = registry_->GetComponent<Relationship>(entity);
	ImGuiTreeNodeFlags flags = 0;
	if (!relationship || relationship->children.empty()) {
		flags |= ImGuiTreeNodeFlags_Leaf;
	}

	bool open = ImGui::TreeNodeEx(("Entity " + std::to_string(entity)).c_str(), flags);

	if (ImGui::IsWindowFocused() && ImGui::IsItemClicked()) {
		selection_->selectedEntity = entity;
	}

	if (open) {
		if (relationship) {
			for (uint32_t child : relationship->children) {
				DrawEntity(child);
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}