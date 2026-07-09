#include "MagicCircleEffect.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Object.h"
#include "Primitive.h"
#include "Transform.h"
#include "Material.h"
#include "UVTransform.h"

void MagicCircleEffect::Initialize() {
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, MeshType::kCylinder);
	registry_->AddComponent(entity_, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(entity_, EulerTransform{.translate = { -3.0f, 0.0f, -3.0f } });
	registry_->AddComponent(entity_, Relationship{});
	registry_->AddComponent(entity_, Material{ .color = {0.0f, 0.0f, 1.0f, 1.0f}, .enableLighting = false, .enableFlipV = true });
	registry_->AddComponent(entity_, UVTransform{});
	registry_->AddComponent(entity_, DirtyTransform{});
	registry_->AddComponent(entity_, DirtyMaterial{});
	registry_->AddComponent(entity_, objectManager_->CreateObject(entity_));
	registry_->AddComponent(entity_, primitiveGenerator_->CreateCylinder(32, 1.0f, 1.0f, 3.0f, false, "gradationLine.png"));
	registry_->AddComponent(entity_, UseCulling{});
	registry_->AddComponent(entity_, indirectCommandManager_->AddIndirectCommand(entity_));
}

void MagicCircleEffect::Update() {
	auto material = registry_->GetComponent<Material>(entity_);
	auto uvTransform = registry_->GetComponent<UVTransform>(entity_);
	if (material && uvTransform) {
		uvTransform->translate.x += 0.001f;
		material->uvTransformMatrix = MakeAffineMatrix(uvTransform->scale, uvTransform->rotate, uvTransform->translate);
		registry_->AddComponent(entity_, DirtyMaterial{});
	}
}