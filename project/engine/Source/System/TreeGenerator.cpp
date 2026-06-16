#include "TreeGenerator.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Cylinder.h"
#include "Object.h"
#include "Transform.h"
#include "Material.h"
#include "Random.h"

uint32_t TreeGenerator::Generate(uint32_t parent, const Vector3 &start, const Vector3 &direction, const Quaternion &parentWorldRotate, uint32_t divide, float topRadius, float bottomRadius, float length, int32_t depth) {
	if (depth <= 0) {
		return UINT_MAX;
	}

	Quaternion worldRotate = Quaternion::DirectionToDirection({ 0.0f, 1.0f, 0.0f }, direction);
	Quaternion localRotate = parentWorldRotate.Inverse() * worldRotate;
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, QuaternionTransform{ .rotate = localRotate, .translate = start });
	registry_->AddComponent(entity, Material{ .environmentCoefficient = 0.0f });
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, objectManager_->CreateObject(entity));
	registry_->AddComponent(entity, cylinderGenerator_->CreateCylinder(divide, topRadius, bottomRadius, length, "Bark001_1K-JPG_Color.jpg"));
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));

	Relationship relationship{ .parent = parent };
	for (size_t i = 0; i < 2; i++) {
		Vector3 newDirection = direction + Random::generate({ -1.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }).normalized();
		uint32_t child = Generate(entity, { 0.0f, length, 0.0f }, newDirection.normalized(), worldRotate, divide, topRadius * 0.7f, bottomRadius * 0.7f, length * 0.7f, depth - 1);
		if (child != UINT_MAX) {
			relationship.children.emplace_back(child);
		}
	}

	registry_->AddComponent(entity, relationship);
	return entity;
}

void TreeGenerator::Delete(uint32_t entity) {
	if (auto relationship = registry_->GetComponent<Relationship>(entity)) {
		for (uint32_t child : relationship->children) {
			Delete(child);
		}
	}
	indirectCommandManager_->RemoveIndirectCommand(entity);
	objectManager_->RemoveObject(entity);
	registry_->RemoveAllComponents(entity);
}