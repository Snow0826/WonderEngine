#include "SkyboxEntity.h"
#include "EntityComponentSystem.h"
#include "Skybox.h"
#include "Transform.h"
#include <cassert>

void SkyboxEntity::Create(Registry *registry, SkyboxGenerator *skyboxGenerator) {
	// レジストリのnullチェック
	assert(registry);

	// スカイボックスジェネレーターのnullチェック
	assert(skyboxGenerator);

	// 天球の追加
	uint32_t entity = registry->GenerateEntity();
	registry->AddComponent(entity, Relationship{});
	registry->AddComponent(entity, skyboxGenerator->CreateSkybox("Skybox", "kloofendal_48d_partly_cloudy_puresky_2k.dds"));
	registry->AddComponent(entity, EulerTransform{ .scale = {800.0f, 800.0f, 800.0f} });
}