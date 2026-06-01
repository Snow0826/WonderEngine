#include "Human.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Model.h"
#include "Object.h"
#include "Material.h"
#include "SkeletonRenderer.h"

void Human::Create(const std::string &fileName, const Vector3 &position) {
	Model model = modelManager_->FindModel(fileName);
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, Transform{ .translate = position });
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
	registry_->AddComponent(entity, DebugSkeletonSettings{ .jointRadius = 0.02f });
	registry_->AddComponent(entity, SkeletonRenderer{});
}