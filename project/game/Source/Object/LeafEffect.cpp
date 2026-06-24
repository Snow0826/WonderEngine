#include "LeafEffect.h"
#include "BlendMode.h"
#include "EntityComponentSystem.h"
#include "Particle.h"
#include <numbers>

void LeafEffect::Initialize() {
	// エミッターの設定
	Emitter emitter{
		.transform = {},
		.area = {
			.min = { -100.0f, 0.0f, -100.0f },
			.max = { 100.0f, 0.0f, 100.0f }
		},
		.scale = {
			.min = { 0.5f, 0.5f, 0.5f },
			.max = { 0.5f, 0.5f, 0.5f }
		},
		.rotate = {},
		.velocity = {},
		.color = {
			.min = { 1.0f, 1.0f, 1.0f, 1.0f },
			.max = { 1.0f, 1.0f, 1.0f, 1.0f }
		},
		.lifeTime = {
			.min = 10.0f,
			.max = 10.0f
		},
		.count = 1024,
		.frequency = 1.0f,
		.frequencyTime = 0.0f
	};

	// パーティクルグループの設定
	ParticleGroup particleGroup = particleManager_->FindParticleGroup("leafEffect");
	particleGroup.canCollideField = true;

	// エンティティの生成
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, BlendMode::kBlendModeNormal);
	registry_->AddComponent(entity_, Relationship{});
	registry_->AddComponent(entity_, particleGroup);
	registry_->AddComponent(entity_, emitter);
}

void LeafEffect::Update() {
	particleManager_->Emit(entity_);
}