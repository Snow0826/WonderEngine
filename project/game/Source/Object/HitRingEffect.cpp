#include "HitRingEffect.h"
#include "BlendMode.h"
#include "EntityComponentSystem.h"
#include "Particle.h"
#include <numbers>

void HitRingEffect::Initialize() {
	// エミッターの設定
	Emitter emitter{
		.transform{.translate = { 3.0f, 0.0f, 0.0f } },
		.area = {},
		.scale = {
			.min = { 1.0f, 1.0f, 1.0f },
			.max = { 1.0f, 1.0f, 1.0f }
		},
		.rotate = {
			.min = { -std::numbers::pi_v<float>, -std::numbers::pi_v<float>, 0.0f },
			.max = { std::numbers::pi_v<float>, std::numbers::pi_v<float>, 0.0f }
		},
		.velocity = {},
		.color = {
			.min = { 1.0f, 1.0f, 1.0f, 1.0f },
			.max = { 1.0f, 1.0f, 1.0f, 1.0f }
		},
		.lifeTime = {
			.min = 1.0f,
			.max = 1.0f
		},
		.count = 4,
		.frequency = 1.0f,
		.frequencyTime = 0.0f
	};

	// パーティクルグループの設定
	ParticleGroup particleGroup = particleManager_->FindParticleGroup("hitRingEffect");
	particleGroup.isBillboard = false;

	// エンティティの生成
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(entity_, Relationship{});
	registry_->AddComponent(entity_, particleGroup);
	registry_->AddComponent(entity_, emitter);
}

void HitRingEffect::Update() {
	particleManager_->Emit(entity_);
}