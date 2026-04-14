#define NOMINMAX
#include "Footprint.h"
#include "EntityComponentSystem.h"

Footprint FootprintManager::CreateFootprint(uint32_t entity, const Vector4 &color) {
	Footprint footprint{ .id = static_cast<uint32_t>(entities_.size()), .color = color };
	entities_.emplace_back(entity);
	return footprint;
}

void FootprintManager::RemoveFootprint(uint32_t entity) {
	Footprint *footprint = registry_->GetComponent<Footprint>(entity);
	Footprint *lastFootprint = registry_->GetComponent<Footprint>(entities_.back());
	if (!footprint || !lastFootprint) {
		return;
	}
	// 最後尾のフットプリントと入れ替える
	if (entity != entities_.back()) {
		entities_[footprint->id] = entities_.back();
		lastFootprint->id = footprint->id;
	}
	entities_.pop_back();
}

void FootprintManager::RemoveOnceFootprint() {
	registry_->ForEach<OnceFootprint>([this](uint32_t entity, OnceFootprint *onceFootprint) {
		RemoveFootprint(entity);
		registry_->RemoveComponent<OnceFootprint>(entity);
		});
}

void FootprintManager::Debug() const {
#ifdef USE_IMGUI
	ImGui::Text("Footprint Count: %zu", entities_.size());
#endif // USE_IMGUI
}

void FootprintInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Footprint")) {
		Footprint *footprint = registry_->GetComponent<Footprint>(entity);
		if (footprint) {
			ImGui::Text("ID: %u", footprint->id);
			ImGui::ColorEdit4("Color", &footprint->color.x);

			if (ImGui::Button("Reset")) {
				*footprint = Footprint{};
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}