#include "Ground.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Model.h"
#include "Material.h"
#include "UVTransform.h"
#include "FootprintMap.h"
#include <cassert>

void GroundFactory::Create(Registry &registry, InstanceAllocator &instanceAllocator, const GroundDesc &desc) {
	// 下レイヤーの地面のエンティティを生成
	uint32_t entity = registry.GenerateEntity();
	registry.AddComponent(entity, MeshType::kModel);
	registry.AddComponent(entity, BlendMode::kBlendModeNone);
	registry.AddComponent(entity, EulerTransform{});
	registry.AddComponent(entity, Relationship{});
	registry.AddComponent(entity, Material{ .enableLighting = desc.enableLighting });
	registry.AddComponent(entity, DirtyTransform{});
	registry.AddComponent(entity, DirtyRelationshipTransform{});
	registry.AddComponent(entity, DirtyMaterial{});
	registry.AddComponent(entity, DirtyTextureData{});
	registry.AddComponent(entity, DirtyMeshLOD{});
	registry.AddComponent(entity, DirtyCullingData{});
	registry.AddComponent(entity, instanceAllocator.Allocate(entity));
	registry.AddComponent(entity, Model{ .name = desc.modelName.data() });
	registry.AddComponent(entity, FootprintMap{ .terrainOriginXZ = desc.terrainOriginXZ, .terrainSizeXZ = desc.terrainSizeXZ });
}