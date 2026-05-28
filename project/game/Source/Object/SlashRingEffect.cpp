#include "SlashRingEffect.h"
#include "BlendMode.h"
#include "EntityComponentSystem.h"
#include "Particle.h"
#include <numbers>

void SlashRingEffect::Initialize() {
	// エミッターの設定
	Emitter emitter{
		.transform{.translate = { 0.0f, 3.0f, 0.0f } },
		.area = {},
		.scale = {
			.min = { 1.0f, 1.0f, 1.0f },
			.max = { 1.0f, 1.0f, 1.0f }
		},
		.rotate = {},
		.velocity = {},
		.color = {
			.min = { 1.0f, 1.0f, 1.0f, 1.0f },
			.max = { 1.0f, 1.0f, 1.0f, 1.0f }
		},
		.lifeTime = {
			.min = 1.0f,
			.max = 1.0f
		},
		.count = 1,
		.frequency = 1.0f,
		.frequencyTime = 0.0f
	};

	// エンティティの生成
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(entity_, particleManager_->FindParticleGroup("slashRingEffect"));
	registry_->AddComponent(entity_, emitter);
}

void SlashRingEffect::Update() {
	particleManager_->Emit(entity_);
}