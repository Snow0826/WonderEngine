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

	// ウィンドウの描画
	if (ImGui::Begin("Inspector", &isOpen_)) {
		auto name = registry_->GetComponent<Name>(selection_->selectedEntity);
		if (name && !name->value.empty()) {
			ImGui::Text("%s", name->value.c_str());
		} else {
			ImGui::Text("Entity %u", selection_->selectedEntity);
		}

		// コンポーネントの描画
		DrawComponents(selection_->selectedEntity);

		// タグコンポーネントの描画
		DrawTagComponents(selection_->selectedEntity);
	}
	ImGui::End();
#endif // USE_IMGUI
}

void InspectorWindow::Edit() {
#ifdef USE_IMGUI
	ImGui::Checkbox("Inspector", &isOpen_);
#endif // USE_IMGUI
}

void InspectorWindow::DrawComponents(uint32_t entity) {
	const auto &componentDrawers = componentDrawerRegistry_->GetComponentDrawers();
	for (size_t i = 0; i < componentDrawers.size(); ++i) {
		// コンポーネント描画関数が登録されていない場合はスキップ
		if (!componentDrawers[i]) {
			continue;
		}

		// エンティティがコンポーネントを持っていない場合はスキップ
		if (!registry_->HasComponentById(i, entity)) {
			continue;
		}

		// コンポーネントの描画
		componentDrawers[i](entity);
	}
}

void InspectorWindow::DrawTagComponents([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	const auto &tagComponents = componentDrawerRegistry_->GetTagComponents();
	for (size_t i = 0; i < tagComponents.size(); i++) {
		// タグコンポーネントが登録されていない場合はスキップ
		if (tagComponents[i].empty()) {
			continue;
		}

		// チェックボックスがtrueならタグコンポーネントを追加、falseならタグコンポーネントを削除
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
