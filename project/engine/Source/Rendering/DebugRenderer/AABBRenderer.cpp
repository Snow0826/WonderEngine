#define NOMINMAX
#include "AABBRenderer.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Collision.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void AABBRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Collision::AABB, AABBRenderer>([&](uint32_t entity, Collision::AABB *aabb, AABBRenderer *aabbRenderer) {
		debugRenderer_->AddAABB(*aabb);
		}, exclude<Disabled>());
#endif // DRAW_LINE
}

void AABBInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("AABB")) {
		Collision::AABB *aabb = registry_->GetComponent<Collision::AABB>(entity);
		if (aabb) {
			ImGui::DragFloat3("min", &aabb->min.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("max", &aabb->max.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}