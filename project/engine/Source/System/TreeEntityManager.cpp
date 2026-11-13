#include "TreeEntityManager.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Transform.h"
#include "Material.h"
#include "Primitive.h"

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
	registry_->AddComponent(entity, DirtyMeshLOD{});
	registry_->AddComponent(entity, DirtyCullingData{});
	registry_->AddComponent(entity, instanceAllocator_->Allocate(entity));
	registry_->AddComponent(entity, primitive);
	registry_->AddComponent(entity, UseCulling{});
    return entity;
}

void TreeEntityManager::DestroyEntity(uint32_t entity) {
	if (auto relationship = registry_->GetComponent<Relationship>(entity)) {
		for (uint32_t child : relationship->children) {
			DestroyEntity(child);
		}
	}
	instanceAllocator_->Free(entity);
	registry_->RemoveAllComponents(entity);
	registry_->AddComponent(entity, DirtyMeshLOD{});
	registry_->AddComponent(entity, DirtyCullingData{});
}