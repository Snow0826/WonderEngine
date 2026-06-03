#include "SimpleSkin.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Model.h"
#include "Object.h"
#include "Material.h"
#include "SkeletonRenderer.h"

void SimpleSkin::Create() {
	Model model = modelManager_->FindModel("simpleSkin.gltf");
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, EulerTransform{ .translate = {-5.0f, 0.0f, 5.0f} });
	registry_->AddComponent(entity, Material{});
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, objectManager_->CreateObject(entity));
	registry_->AddComponent(entity, model);
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));
	registry_->AddComponent(entity, AnimationPlayer{});
	registry_->AddComponent(entity, AnimationInterpolationMode::Linear);
	registry_->AddComponent(entity, modelManager_->CreateSkeleton(model.modelData.rootNode));
	registry_->AddComponent(entity, DebugSkeletonSettings{});
	registry_->AddComponent(entity, SkeletonRenderer{});
}