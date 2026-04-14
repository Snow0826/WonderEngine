#define NOMINMAX
#include "Capsule.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Collision.h"

void CapsuleRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Collision::Capsule, CapsuleRenderer>([&](uint32_t entity, Collision::Capsule *capsule, CapsuleRenderer *capsuleRenderer) {
		debugRenderer_->AddCapsule(*capsule);
		}, exclude<Disabled>());
#endif // DRAW_LINE
}

void CapsuleInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Capsule")) {
		Collision::Capsule *capsule = registry_->GetComponent<Collision::Capsule>(entity);
		if (capsule) {
			ImGui::DragFloat3("segment.origin", &capsule->segment.origin.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("segment.diff", &capsule->segment.diff.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat("radius", &capsule->radius, 0.01f, 0.0f, std::numeric_limits<float>::max());
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}