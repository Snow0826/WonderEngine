#include "Title.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Model.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"

void Title::Create(const std::string &fileName, const Vector3 &rotate, const Vector3 &translate, const Vector4 &color) {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, Transform{ .rotate = rotate, .translate = translate });
	registry_->AddComponent(entity, Material{ .color = color });
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, objectManager_->CreateObject(entity));
	registry_->AddComponent(entity, modelManager_->FindModel(fileName));
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));
}