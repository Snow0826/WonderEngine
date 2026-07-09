#include "AnimatedCube.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Model.h"
#include "Object.h"
#include "Material.h"

void AnimatedCube::Create() {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, MeshType::kModel);
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, EulerTransform{});
	registry_->AddComponent(entity, Relationship{});
	registry_->AddComponent(entity, Material{});
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, objectManager_->CreateObject(entity));
	registry_->AddComponent(entity, modelManager_->FindModel("AnimatedCube.gltf"));
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));
	registry_->AddComponent(entity, AnimationPlayer{});
	registry_->AddComponent(entity, AnimationInterpolationMode::Linear);
}