#include "ArmParticle.h"
#include "BlendMode.h"
#include "EntityComponentSystem.h"
#include "Particle.h"
#include "Model.h"

void ArmParticle::Initialize(uint32_t entity, const std::string &groupName) {
	// 腕エンティティの取得
	armEntity_ = entity;

	// 腕パーティクルグループの作成
	ParticleGroup armParticleGroup = particleManager_->FindParticleGroup(groupName);

	// 腕パーティクルエミッターの設定
	Emitter armEmitter{
		.transform{},
		.area = {
			.min = { -0.2f, -0.2f, -0.2f },
			.max = { 0.2f, 0.2f, 0.2f }
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
			.min = { 1.0f, 0.0f, 0.0f, 0.0f },
			.max = { 1.0f, 1.0f, 0.0f, 1.0f }
		},
		.lifeTime = {
			.min = 0.5f,
			.max = 1.0f
		},
		.count = 10,
		.frequency = 0.2f,
		.frequencyTime = 0.0f
	};

	// 腕パーティクルエンティティの生成
	particleEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(particleEntity_, BlendMode::kBlendModeAdditive);
	registry_->AddComponent(particleEntity_, armParticleGroup);
	registry_->AddComponent(particleEntity_, armEmitter);
	registry_->AddComponent(particleEntity_, Field{});
}

void ArmParticle::Update() {
	Model *armModel = registry_->GetComponent<Model>(armEntity_);
	if (armModel) {
		TransformSystem transformSystem{ registry_ };
		for (const MeshData &armMesh : armModel->modelData.meshes) {
			// エミッターの位置を更新
			Emitter *emitter = registry_->GetComponent<Emitter>(particleEntity_);
			if (emitter) {
				emitter->transform.translate = armMesh.sphere.center;
			}

			// フィールドの方向を更新
			Field *field = registry_->GetComponent<Field>(particleEntity_);
			if (field) {
				field->acceleration = transformSystem.GetUp(armEntity_) * 10.0f;
				field->area = armMesh.aabb;
			}
		}

	}

	// パーティクルの発生
	particleManager_->Emit(particleEntity_);
}