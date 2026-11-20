#include "TreeEntityManager.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Transform.h"
#include "Material.h"
#include "Primitive.h"
#include "TreeGenerator.h"

uint32_t TreeEntityManager::CreateEntity(const Primitive &primitive, const Vector3 &position) {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, Name{ .value = "Tree" + std::to_string(treeEntityCounter_++) });
	registry_->AddComponent(entity, MeshType::kTree);
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, EulerTransform{ .translate = position });
	registry_->AddComponent(entity, Material{ .environmentCoefficient = 0.0f });
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyRelationshipTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, DirtyTextureData{});
	registry_->AddComponent(entity, DirtyIndirectCommandData{});
	registry_->AddComponent(entity, instanceAllocator_->Allocate(entity));
	registry_->AddComponent(entity, primitive);
	registry_->AddComponent(entity, UseCulling{});
	return entity;
}

void TreeEntityManager::CreateEntity(uint32_t entity, const Primitive &primitive, const Vector3 &position) {
	registry_->AddComponent(entity, Name{ .value = "Tree" + std::to_string(treeEntityCounter_++) });
	registry_->AddComponent(entity, MeshType::kTree);
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, EulerTransform{ .translate = position });
	registry_->AddComponent(entity, Material{ .environmentCoefficient = 0.0f });
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyRelationshipTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, DirtyTextureData{});
	registry_->AddComponent(entity, DirtyIndirectCommandData{});
	registry_->AddComponent(entity, instanceAllocator_->Allocate(entity));
	registry_->AddComponent(entity, primitive);
	registry_->AddComponent(entity, UseCulling{});
}

uint32_t TreeEntityManager::CreateLeafEntity(uint32_t treeEntity, const Primitive &primitive, const Vector3 &position) {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, Name{ .value = "Leaf" + std::to_string(leafEntityCounter_++) });
	registry_->AddComponent(entity, MeshType::kPlane);
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, EulerTransform{ .scale = { 0.2f, 0.2f, 0.2f }, .translate = position });
	registry_->AddComponent(entity, Material{ .environmentCoefficient = 0.0f });
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyRelationshipTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, DirtyTextureData{});
	registry_->AddComponent(entity, DirtyIndirectCommandData{});
	registry_->AddComponent(entity, instanceAllocator_->Allocate(entity));
	registry_->AddComponent(entity, primitive);
	registry_->AddComponent(entity, UseCulling{});
	return entity;
}

void TreeEntityManager::DestroyEntity(uint32_t entity) {
	instanceAllocator_->Free(entity);
	registry_->RemoveAllComponents(entity);
	registry_->AddComponent(entity, DirtyIndirectCommandData{});
}