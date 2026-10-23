#include "GroundFactory.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Model.h"
#include "Material.h"
#include "FootprintMap.h"

void GroundFactory::Create(Registry &registry, InstanceAllocator &instanceAllocator) {
	// 下レイヤーの地面のエンティティを生成
	uint32_t entity = registry.GenerateEntity();
	registry.AddComponent(entity, Name{ .value = "Ground" });
	registry.AddComponent(entity, MeshType::kModel);
	registry.AddComponent(entity, BlendMode::kBlendModeNone);
	registry.AddComponent(entity, EulerTransform{});
	registry.AddComponent(entity, Material{ .enableLighting = false });
	registry.AddComponent(entity, DirtyTransform{});
	registry.AddComponent(entity, DirtyRelationshipTransform{});
	registry.AddComponent(entity, DirtyMaterial{});
	registry.AddComponent(entity, DirtyTextureData{});
	registry.AddComponent(entity, DirtyMeshLOD{});
	registry.AddComponent(entity, DirtyCullingData{});
	registry.AddComponent(entity, instanceAllocator.Allocate(entity));
	registry.AddComponent(entity, Model{ .name = "ground.obj" });
	registry.AddComponent(entity, FootprintMap{ .terrainOriginXZ = { 0.0f, 0.0f }, .terrainSizeXZ = { 1024.0f, 1024.0f } });
}