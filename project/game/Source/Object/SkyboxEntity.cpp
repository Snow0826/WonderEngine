#include "SkyboxEntity.h"
#include "EntityComponentSystem.h"
#include "Skybox.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"
#include <cassert>

void SkyboxEntity::Create(Registry *registry, SkyboxGenerator *skyboxGenerator, ObjectManager *objectManager) {
	// レジストリのnullチェック
	assert(registry);

	// スカイボックスジェネレーターのnullチェック
	assert(skyboxGenerator);

	// オブジェクトマネージャーのnullチェック
	assert(objectManager);

	// 天球の追加
	uint32_t entity = registry->GenerateEntity();
	registry->AddComponent(entity, Transform{ .scale = {800.0f, 800.0f, 800.0f} });
	registry->AddComponent(entity, Material{});
	registry->AddComponent(entity, DirtyTransform{});
	registry->AddComponent(entity, DirtyMaterial{});
	registry->AddComponent(entity, objectManager->CreateObject(entity));
	registry->AddComponent(entity, skyboxGenerator->CreateSkybox("kloofendal_48d_partly_cloudy_puresky_2k.dds"));
}