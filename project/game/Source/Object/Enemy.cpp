#include "Enemy.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "BlendMode.h"
#include "Object.h"
#include "Model.h"
#include "Transform.h"
#include "Material.h"
#include "AABB.h"
#include "Sphere.h"
#include "RigidBody.h"
#include "Footprint.h"

void Enemy::Initialize(const Vector3 &position) {
	// エンティティの生成
	enemyEntity_ = registry_->GenerateEntity();

	// コンポーネントの追加
	registry_->AddComponent(enemyEntity_, BlendMode::kBlendModeNone);
	registry_->AddComponent(enemyEntity_, Transform{ .translate = position });
	registry_->AddComponent(enemyEntity_, Material{});
	registry_->AddComponent(enemyEntity_, DirtyTransform{});
	registry_->AddComponent(enemyEntity_, DirtyMaterial{});
	registry_->AddComponent(enemyEntity_, objectManager_->CreateObject(enemyEntity_));
	registry_->AddComponent(enemyEntity_, modelManager_->FindModel("enemy.obj"));
	registry_->AddComponent(enemyEntity_, UseCulling{});
	registry_->AddComponent(enemyEntity_, RigidBody{});
	registry_->AddComponent(enemyEntity_, footprintManager_->CreateFootprint(enemyEntity_, { 0.0f, 0.0f, 1.0f, 1.0f }));
	registry_->AddComponent(enemyEntity_, SphereCollider{});
	registry_->AddComponent(enemyEntity_, SphereRenderer{});
	registry_->AddComponent(enemyEntity_, AABBRenderer{});
	registry_->AddComponent(enemyEntity_, indirectCommandManager_->AddIndirectCommand(enemyEntity_));
}

void Enemy::Update() {
	Transform *targetTransform = registry_->GetComponent<Transform>(playerEntity_);
	Transform *enemyTransform = registry_->GetComponent<Transform>(enemyEntity_);
	if (!targetTransform || !enemyTransform) {
		return;
	}

	TransformSystem transformSystem{ registry_ };
	Vector3 toPlayer = transformSystem.GetWorldPosition(enemyEntity_).normalized(transformSystem.GetWorldPosition(playerEntity_));

	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(enemyEntity_);
	if (rigidBody) {
		rigidBody->force.y = -19.62f; // 重力加速度
		float velocityY = rigidBody->velocity.y;
		rigidBody->velocity = Slerp(rigidBody->velocity, toPlayer, 0.1f) * 8.0f;
		rigidBody->velocity.y = velocityY;
	}
	enemyTransform->rotateMatrix = LookAt(enemyTransform->translate, targetTransform->translate, { .y = 1.0f });
}

void Enemy::OnCollision(const Collision::Plane &plane) {
	// モデルの取得
	Model *model = registry_->GetComponent<Model>(enemyEntity_);
	if (!model) {
		return;
	}

	// 貫入量を計算
	float penetration = PenetrationDepth(model->modelData.meshes.back().worldCollisionData.sphere, plane);

	// 貫入量が0以下なら衝突していないので処理を抜ける
	if (penetration <= 0.0f) {
		return;
	}

	// 剛体の取得
	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(enemyEntity_);
	if (rigidBody) {
		rigidBody->velocity.y = 0.0f;
	}

	// SRTデータの取得
	Transform *transform = registry_->GetComponent<Transform>(enemyEntity_);
	if (!transform) {
		return;
	}

	// 貫入量分だけ位置を修正
	transform->translate += plane.normal * penetration;
}

void Enemy::OnCollision(const Collision::AABB &aabb) {
	// モデルの取得
	Model *model = registry_->GetComponent<Model>(enemyEntity_);
	if (!model) {
		return;
	}

	// 貫入量を計算
	float penetration = PenetrationDepth(model->modelData.meshes.back().worldCollisionData.sphere, aabb);

	// 貫入量が0以下なら衝突していないので処理を抜ける
	if (penetration <= 0.0f) {
		return;
	}

	// 剛体の取得
	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(enemyEntity_);
	if (rigidBody) {
		rigidBody->velocity.y = 0.0f;
	}

	// SRTデータの取得
	Transform *transform = registry_->GetComponent<Transform>(enemyEntity_);
	if (!transform) {
		return;
	}

	// 法線ベクトルの計算
	Vector3 normal = Normal(model->modelData.meshes.back().worldCollisionData.sphere.center, aabb);

	// 貫入量分だけ位置を修正
	transform->translate += normal * penetration;
}

void Enemy::OnCollision() {
	Dead();
}

void Enemy::Dead() {
	indirectCommandManager_->RemoveIndirectCommand(enemyEntity_);
	footprintManager_->RemoveFootprint(enemyEntity_);
	registry_->RemoveAllComponents(enemyEntity_);
}

bool Enemy::IsDead() const {
	return !registry_->HasComponent<BlendMode>(enemyEntity_);
}