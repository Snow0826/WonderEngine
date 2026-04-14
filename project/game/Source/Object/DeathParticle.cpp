#include "DeathParticle.h"
#include "BlendMode.h"
#include "EntityComponentSystem.h"
#include "Particle.h"

void DeathParticle::Initialize(const Vector3 &position) {
	// デスパーティクルグループの作成
	ParticleGroup deathParticleGroup = particleManager_->FindParticleGroup("death");

	// デスパーティクルエミッターの設定
	Emitter deathEmitter{
		.transform{.translate = position },
		.area = {
			.min = { 0.0f, 0.0f, 0.0f },
			.max = { 0.0f, 0.0f, 0.0f }
		},
		.scale = {
			.min = { 0.8f, 0.8f, 0.8f },
			.max = { 0.8f, 0.8f, 0.8f }
		},
		.velocity = {
			.min = { -1.0f, -1.0f, -1.0f },
			.max = { 1.0f, 1.0f, 1.0f }
		},
		.color = {
			.min = { 1.0f, 0.5f, 0.0f, 0.0f },
			.max = { 1.0f, 1.0f, 0.0f, 1.0f }
		},
		.lifeTime = {
			.min = 1.0f,
			.max = 2.0f
		},
		.count = 100,
		.frequency = 0.0f,
		.frequencyTime = 0.0f
	};

	// デスパーティクルエンティティの生成
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(entity_, deathParticleGroup);
	registry_->AddComponent(entity_, deathEmitter);
}

void DeathParticle::Update() {
	particleManager_->Emit(entity_);
}

bool DeathParticle::IsDead() const {
	return registry_->GetComponent<ParticleGroup>(entity_)->particles.empty();
}