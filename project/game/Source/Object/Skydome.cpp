#include "Skydome.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "BlendMode.h"
#include "Object.h"
#include "Model.h"
#include "Transform.h"
#include "Material.h"
#include <cassert>

void Skydome::Create(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager) {
	// レジストリのnullチェック
	assert(registry);

	// 間接コマンドマネージャーのnullチェック
	assert(indirectCommandManager);

	// モデルマネージャーのnullチェック
	assert(modelManager);

	// オブジェクトマネージャーのnullチェック
	assert(objectManager);

	// 天球の追加
	uint32_t entity = registry->GenerateEntity();
	registry->AddComponent(entity, BlendMode::kBlendModeNone);
	registry->AddComponent(entity, Transform{});
	registry->AddComponent(entity, Material{ .enableLighting = false });
	registry->AddComponent(entity, DirtyTransform{});
	registry->AddComponent(entity, DirtyMaterial{});
	registry->AddComponent(entity, UseCulling{});
	registry->AddComponent(entity, objectManager->CreateObject(entity));
	registry->AddComponent(entity, modelManager->FindModel("skydome.obj"));
	registry->AddComponent(entity, indirectCommandManager->AddIndirectCommand(entity));
}