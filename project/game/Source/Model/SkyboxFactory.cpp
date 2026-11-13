#include "SkyboxFactory.h"
#include "EntityComponentSystem.h"
#include "Skybox.h"
#include "Transform.h"

void SkyboxFactory::Create(Registry &registry, SkyboxGenerator &skyboxGenerator) {
	// スカイボックスエンティティを生成
	uint32_t entity = registry.GenerateEntity();
	registry.AddComponent(entity, Name{ .value = "Skybox" });
	registry.AddComponent(entity, DirtyTransform{});
	registry.AddComponent(entity, DirtyRelationshipTransform{});
	registry.AddComponent(entity, skyboxGenerator.CreateSkybox("Skybox", "kloofendal_48d_partly_cloudy_puresky_2k.dds"));
	registry.AddComponent(entity, EulerTransform{ .scale = {800.0f, 800.0f, 800.0f} });
}