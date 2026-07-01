#define NOMINMAX
#include "SphereRenderer.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Collision.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void SphereRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Collision::Sphere, SphereRenderer>([&](uint32_t entity, Collision::Sphere *sphere, SphereRenderer *sphereRenderer) {
		debugRenderer_->AddSphere(*sphere);
		}, exclude<Disabled>());
#endif // DRAW_LINE
}

void SphereInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Sphere")) {
		Collision::Sphere *sphere = registry_->GetComponent<Collision::Sphere>(entity);
		if (sphere) {
			ImGui::DragFloat3("Center", &sphere->center.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
			ImGui::DragFloat("Radius", &sphere->radius, 0.01f, 0.0f, std::numeric_limits<float>::max());
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}