#define NOMINMAX
#include "Plane.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Collision.h"

void PlaneRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Collision::Plane, PlaneRenderer>([&](uint32_t entity, Collision::Plane *plane, PlaneRenderer *planeRenderer) {
		debugRenderer_->AddPlane(*plane);
		}, exclude<Disabled>());
#endif // DRAW_LINE
}

void PlaneInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Plane")) {
		Collision::Plane *plane = registry_->GetComponent<Collision::Plane>(entity);
		if (plane) {
			ImGui::DragFloat3("normal", &plane->normal.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat("distance", &plane->distance, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			plane->normal = plane->normal.normalized();
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}