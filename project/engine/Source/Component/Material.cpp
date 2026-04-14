#define NOMINMAX
#include "Material.h"
#include "EntityComponentSystem.h"

void MaterialInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Material")) {
		Material *material = registry_->GetComponent<Material>(entity);
		if (material) {
			if (ImGui::ColorEdit4("color", &material->color.x)) {
				registry_->AddComponent<DirtyMaterial>(entity, DirtyMaterial{});
			}

			if (ImGui::Checkbox("enableLighting", reinterpret_cast<bool *>(&material->enableLighting))) {
				registry_->AddComponent<DirtyMaterial>(entity, DirtyMaterial{});
			}

			if (ImGui::TreeNode("UVTransformMatrix")) {
				for (size_t i = 0; i < 4; i++) {
					if (ImGui::DragFloat4(("Row " + std::to_string(i)).c_str(), &material->uvTransformMatrix.m[i][0], 0.001f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max())) {
						registry_->AddComponent<DirtyMaterial>(entity, DirtyMaterial{});
					}
				}
				ImGui::TreePop();
			}

			if (ImGui::DragFloat("shininess", &material->shininess, 0.1f, 0.0f, std::numeric_limits<float>::max())) {
				registry_->AddComponent<DirtyMaterial>(entity, DirtyMaterial{});
			}

			if (ImGui::ColorEdit3("specular", &material->specular.x)) {
				registry_->AddComponent<DirtyMaterial>(entity, DirtyMaterial{});
			}

			if (ImGui::Button("Reset")) {
				*material = Material{};
				registry_->AddComponent<DirtyMaterial>(entity, DirtyMaterial{});
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}