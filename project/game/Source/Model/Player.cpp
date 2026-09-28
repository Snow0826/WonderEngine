#include "Player.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Model.h"
#include "Material.h"
#include "RigidBody.h"

void Player::Create(const Vector3 &position) {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, MeshType::kModel);
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, EulerTransform{ .translate = position });
	registry_->AddComponent(entity, Relationship{});
	registry_->AddComponent(entity, Material{});
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyRelationshipTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, DirtyTextureData{});
	registry_->AddComponent(entity, DirtyMeshLOD{});
	registry_->AddComponent(entity, DirtyCullingData{});
	registry_->AddComponent(entity, instanceAllocator_->Allocate(entity));
	registry_->AddComponent(entity, modelManager_->FindModel("sphere.obj"));
	registry_->AddComponent(entity, RigidBody{});
}