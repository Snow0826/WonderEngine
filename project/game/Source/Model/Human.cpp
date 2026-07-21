#include "Human.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Model.h"
#include "Material.h"
#include "SkeletonRenderer.h"
#include <numbers>

void Human::Create(const std::string &fileName, const Vector3 &position) {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, MeshType::kModel);
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, EulerTransform{ .rotate = { 0.0f, std::numbers::pi_v<float>, 0.0f }, .translate = position });
	registry_->AddComponent(entity, Relationship{});
	registry_->AddComponent(entity, Material{});
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, DirtyTextureData{});
	registry_->AddComponent(entity, DirtyMeshLOD{});
	registry_->AddComponent(entity, DirtyCullingData{});
	registry_->AddComponent(entity, instanceAllocator_->Allocate(entity));
	registry_->AddComponent(entity, modelManager_->FindModel(fileName));
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, SkinMesh{});
	registry_->AddComponent(entity, AnimationPlayer{});
	registry_->AddComponent(entity, AnimationInterpolationMode::Linear);
	registry_->AddComponent(entity, DebugSkeletonSettings{ .jointRadius = 0.02f });
	registry_->AddComponent(entity, SkeletonRenderer{});
}