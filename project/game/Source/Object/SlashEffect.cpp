#include "SlashEffect.h"
#include "BlendMode.h"
#include "EntityComponentSystem.h"
#include "Particle.h"
#include <numbers>

void SlashEffect::Initialize() {
	// エミッターの設定
	Emitter emitter{
		.transform{},
		.area = {},
		.scale = {
			.min = { 0.05f, 0.4f, 1.0f },
			.max = { 0.05f, 1.5f, 1.0f }
		},
		.rotate = {
			.min = { 0.0f, 0.0f, -std::numbers::pi_v<float> },
			.max = { 0.0f, 0.0f, std::numbers::pi_v<float> }
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
		.count = 3,
		.frequency = 1.0f,
		.frequencyTime = 0.0f
	};

	// エンティティの生成
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(entity_, Relationship{});
	registry_->AddComponent(entity_, particleManager_->FindParticleGroup("slashEffect"));
	registry_->AddComponent(entity_, emitter);
}

void SlashEffect::Update() {
	particleManager_->Emit(entity_);
}