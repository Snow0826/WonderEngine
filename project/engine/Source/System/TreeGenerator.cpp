#include "TreeGenerator.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Cylinder.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"
#include "Random.h"

void TreeGenerator::Generate(const Vector3 &start, const Vector3 &direction, uint32_t divide, float topRadius, float bottomRadius, float length, int32_t depth) {
	if (depth <= 0) {
		return;
	}

	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, Transform{ .translate = start, .quaternion = Quaternion::DirectionToDirection({ 0.0f, 1.0f, 0.0f }, direction) });
	registry_->AddComponent(entity, Material{});
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, objectManager_->CreateObject(entity));
	registry_->AddComponent(entity, cylinderGenerator_->CreateCylinder(divide, topRadius, bottomRadius, length, "white8x8.png"));
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));

	Vector3 end = start + direction * length;
	for (size_t i = 0; i < 2; i++) {
		Vector3 newDirection = direction + Random::generate({ -1.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }).normalized();
		Generate(end, newDirection.normalized(), divide, topRadius, bottomRadius, length * 0.7f, depth - 1);
	}
}