#define NOMINMAX
#include "Sphere.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Collision.h"
#include "Model.h"

void SphereRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Collision::Sphere, SphereRenderer>([&](uint32_t entity, Collision::Sphere *sphere, SphereRenderer *sphereRenderer) {
		debugRenderer_->AddSphere(*sphere);
		}, exclude<Disabled>());

	registry_->ForEach<Model, SphereRenderer>([&](uint32_t entity, Model *model, SphereRenderer *sphereRenderer) {
		for (const MeshData &meshData : model->modelData.meshes) {
			debugRenderer_->AddSphere(meshData.worldCollisionData.sphere);
		}
		}, exclude<Disabled>());
#endif // DRAW_LINE
}

void SphereInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Sphere")) {
		Collision::Sphere *sphere = registry_->GetComponent<Collision::Sphere>(entity);
		if (sphere) {
			ImGui::DragFloat3("Center", &sphere->center.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat("Radius", &sphere->radius, 0.01f, 0.0f, std::numeric_limits<float>::max());
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}