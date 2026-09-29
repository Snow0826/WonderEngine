#include "Ground.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Model.h"
#include "Material.h"
#include "UVTransform.h"
#include "FootprintMap.h"
#include <cassert>

void Ground::Create(Registry *registry, ModelManager *modelManager, InstanceAllocator *instanceAllocator) {
	// nullチェック
	assert(registry);
	assert(modelManager);
	assert(instanceAllocator);

	// 下レイヤーの地面のエンティティを生成
	uint32_t entity = registry->GenerateEntity();
	registry->AddComponent(entity, MeshType::kModel);
	registry->AddComponent(entity, BlendMode::kBlendModeNone);
	registry->AddComponent(entity, EulerTransform{});
	registry->AddComponent(entity, Relationship{});
	registry->AddComponent(entity, Material{ .enableLighting = false });
	registry->AddComponent(entity, DirtyTransform{});
	registry->AddComponent(entity, DirtyRelationshipTransform{});
	registry->AddComponent(entity, DirtyMaterial{});
	registry->AddComponent(entity, DirtyTextureData{});
	registry->AddComponent(entity, DirtyMeshLOD{});
	registry->AddComponent(entity, DirtyCullingData{});
	registry->AddComponent(entity, instanceAllocator->Allocate(entity));
	registry->AddComponent(entity, modelManager->FindModel("ground.obj"));
	registry->AddComponent(entity, FootprintMap{ .terrainOriginXZ = Vector2{ 0.0f, 0.0f }, .terrainSizeXZ = Vector2{ 1024.0f, 1024.0f } });
}