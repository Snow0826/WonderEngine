#include "Ground.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "BlendMode.h"
#include "Object.h"
#include "Model.h"
#include "Transform.h"
#include "Material.h"
#include "Plane.h"
#include "FootprintMap.h"
#include "Footprint.h"
#include "Sphere.h"
#include <cassert>

void Ground::Create(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager, FootprintManager *footprintManager) {
	// レジストリのnullチェック
	assert(registry);

	// 間接コマンドマネージャーのnullチェック
	assert(indirectCommandManager);

	// モデルマネージャーのnullチェック
	assert(modelManager);

	// オブジェクトマネージャーのnullチェック
	assert(objectManager);

	// ミップマップ無効化
	Model model = modelManager->FindModel("ground.obj");
	model.enableMipMaps[model.modelData.meshes.back().materialIndex] = false;

	// 地面の追加
	uint32_t entity = registry->GenerateEntity();
	registry->AddComponent(entity, BlendMode::kBlendModeNormal);
	registry->AddComponent(entity, Transform{});
	registry->AddComponent(entity, Material{});
	registry->AddComponent(entity, DirtyTransform{});
	registry->AddComponent(entity, DirtyMaterial{});
	registry->AddComponent(entity, objectManager->CreateObject(entity));
	registry->AddComponent(entity, model);
	registry->AddComponent(entity, Collision::Plane{ .normal = {0.0f, 1.0f, 0.0f}, .distance = 0.0f });
	registry->AddComponent(entity, PlaneCollider{});
	registry->AddComponent(entity, PlaneRenderer{});
	registry->AddComponent(entity, FootprintMap{ .terrainOriginXZ = Vector2{ 0.0f, 0.0f }, .terrainSizeXZ = Vector2{ 512.0f, 512.0f }, });
	registry->AddComponent(entity, indirectCommandManager->AddIndirectCommand(entity));

	// デフォルトフットプリントの追加
	entity = registry->GenerateEntity();
	registry->AddComponent(entity, footprintManager->CreateFootprint(entity, { 0.0f, 0.0f, 1.0f, 1.0f }));
	registry->AddComponent(entity, OnceFootprint{});
	registry->AddComponent(entity, Collision::Sphere{ .radius = 10.0f });
	registry->AddComponent(entity, SphereRenderer{});

	// デフォルトフットプリントの追加
	entity = registry->GenerateEntity();
	registry->AddComponent(entity, footprintManager->CreateFootprint(entity, { 0.0f, 0.0f, 1.0f, 1.0f }));
	registry->AddComponent(entity, OnceFootprint{});
	registry->AddComponent(entity, Collision::Sphere{ .center = {.x = -128.0f, .z = -128.0f}, .radius = 10.0f });
	registry->AddComponent(entity, SphereRenderer{});

	// デフォルトフットプリントの追加
	entity = registry->GenerateEntity();
	registry->AddComponent(entity, footprintManager->CreateFootprint(entity, { 0.0f, 0.0f, 1.0f, 1.0f }));
	registry->AddComponent(entity, OnceFootprint{});
	registry->AddComponent(entity, Collision::Sphere{ .center = {.x = -128.0f, .z = 128.0f}, .radius = 10.0f });
	registry->AddComponent(entity, SphereRenderer{});

	// デフォルトフットプリントの追加
	entity = registry->GenerateEntity();
	registry->AddComponent(entity, footprintManager->CreateFootprint(entity, { 0.0f, 0.0f, 1.0f, 1.0f }));
	registry->AddComponent(entity, OnceFootprint{});
	registry->AddComponent(entity, Collision::Sphere{ .center = {.x = 128.0f, .z = -128.0f}, .radius = 10.0f });
	registry->AddComponent(entity, SphereRenderer{});

	// デフォルトフットプリントの追加
	entity = registry->GenerateEntity();
	registry->AddComponent(entity, footprintManager->CreateFootprint(entity, { 0.0f, 0.0f, 1.0f, 1.0f }));
	registry->AddComponent(entity, OnceFootprint{});
	registry->AddComponent(entity, Collision::Sphere{ .center = {.x = 128.0f, .z = 128.0f}, .radius = 10.0f });
	registry->AddComponent(entity, SphereRenderer{});
}