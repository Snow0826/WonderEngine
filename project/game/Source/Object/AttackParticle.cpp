#include "AttackParticle.h"
#include "BlendMode.h"
#include "EntityComponentSystem.h"
#include "Particle.h"
#include "Sphere.h"
#include "Footprint.h"
#include <numbers>

void AttackParticle::Initialize(uint32_t entity) {
	// プレイヤーエンティティの保存
	playerEntity_ = entity;

	// 攻撃パーティクルグループの作成
	ParticleGroup attackParticleGroup = particleManager_->FindParticleGroup("attack");

	// 攻撃パーティクルエミッターの設定
	Emitter attackEmitter{
		.transform{},
		.area = {
			.min = { -1.0f, -1.0f, -1.0f },
			.max = { 1.0f, 1.0f, 1.0f }
		},
		.scale = {
			.min = { 1.0f, 1.0f, 1.0f },
			.max = { 2.0f, 2.0f, 2.0f }
		},
		.velocity = {
			.min = { -5.0f, -5.0f, -5.0f },
			.max = { 5.0f, 5.0f, 5.0f }
		},
		.color = {
			.min = { 1.0f, 0.5f, 0.0f, 1.0f },
			.max = { 1.0f, 1.0f, 0.0f, 1.0f }
		},
		.lifeTime = {
			.min = 0.3f,
			.max = 0.7f
		},
		.count = 20,
		.frequency = 0.1f,
		.frequencyTime = 0.0f
	};

	// 攻撃パーティクルエンティティの生成
	particleEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(particleEntity_, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(particleEntity_, attackParticleGroup);
	registry_->AddComponent(particleEntity_, attackEmitter);
	registry_->AddComponent(particleEntity_, Field{});
	registry_->AddComponent(particleEntity_, Collision::Sphere{});
}

void AttackParticle::Update() {
	// エミッターの位置を更新
	TransformSystem transformSystem{ registry_ };
	Vector3 worldPosition = transformSystem.GetWorldPosition(playerEntity_);
	Vector3 forward = transformSystem.GetForward(playerEntity_) * -20.0f;
	Emitter *emitter = registry_->GetComponent<Emitter>(particleEntity_);
	if (emitter) {
		emitter->transform.translate = worldPosition + forward;
	}

	// フィールドの方向を更新
	Field *field = registry_->GetComponent<Field>(particleEntity_);
	if (field) {
		field->area.min = worldPosition + forward - kFieldAreaSize;
		field->area.max = worldPosition + forward + kFieldAreaSize;
		field->acceleration.y = 10.0f;
		field->angularVelocity = std::numbers::pi_v<float>;
		field->radius = 0.5f;
	}

	// 衝突判定用の球の位置と半径を更新
	Collision::Sphere *sphere = registry_->GetComponent<Collision::Sphere>(particleEntity_);
	if (sphere) {
		sphere->center = worldPosition + forward;
		sphere->center.y = 0.0f;
		sphere->radius = kFieldAreaSize.length() / 2.0f;
	}

	// パーティクルの発生
	particleManager_->Emit(particleEntity_);
}

void AttackParticle::Start() {
	registry_->AddComponent(particleEntity_, SphereRenderer{});
	registry_->AddComponent(particleEntity_, SphereCollider{});
	registry_->AddComponent(particleEntity_, footprintManager_->CreateFootprint(particleEntity_, { 1.0f, 0.0f, 0.0f, 1.0f }));
}

void AttackParticle::Stop() {
	registry_->RemoveComponent<SphereRenderer>(particleEntity_);
	registry_->RemoveComponent<SphereCollider>(particleEntity_);
	footprintManager_->RemoveFootprint(particleEntity_);
}