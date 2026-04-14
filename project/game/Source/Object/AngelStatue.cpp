#include "AngelStatue.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Model.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"
#include "AABB.h"

void AngelStatue::Create(const Vector3 &rotate, const Vector3 &translate) {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, Transform{ .scale = {8.0f, 8.0f, 8.0f}, .rotate = rotate, .translate = translate });
	registry_->AddComponent(entity, Material{});
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, objectManager_->CreateObject(entity));
	registry_->AddComponent(entity, modelManager_->FindModel("angel_statue.gltf"));
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));
	registry_->AddComponent(entity, AABBRenderer{});
	registry_->AddComponent(entity, AABBCollider{});
}