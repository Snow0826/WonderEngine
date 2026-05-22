#include "CylinderEntity.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Cylinder.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"

void CylinderEntity::Create(uint32_t divide, float topRadius, float bottomRadius, float height) {
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, Transform{});
	registry_->AddComponent(entity, Material{});
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, objectManager_->CreateObject(entity));
	registry_->AddComponent(entity, cylinderGenerator_->CreateCylinder(divide, topRadius, bottomRadius, height, "white8x8.png"));
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));
}