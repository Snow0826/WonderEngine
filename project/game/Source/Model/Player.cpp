#include "Player.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "InstanceAllocator.h"
#include "Model.h"
#include "Material.h"
#include "RigidBody.h"
#include "Footprint.h"
#include "SphereRenderer.h"

namespace {
	constexpr float kMiu = 0.5f;		// 摩擦係数
	constexpr float kGravity = -9.81f;	// 重力加速度
	constexpr float kMagnitude = 20.0f;	// 力の大きさ
	constexpr Vector3 kWorldUp = { 0.0f, 1.0f, 0.0f };	// ワールド座標系の上方向ベクトル
}

void Player::Initialize() {
	entity_ = registry_->GenerateEntity();
	registry_->AddComponent(entity_, MeshType::kModel);
	registry_->AddComponent(entity_, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity_, EulerTransform{ .translate = Vector3{ 0.0f, 1.0f, 0.0f } });
	registry_->AddComponent(entity_, Relationship{});
	registry_->AddComponent(entity_, Material{ .environmentCoefficient = 0.0f });
	registry_->AddComponent(entity_, DirtyTransform{});
	registry_->AddComponent(entity_, DirtyRelationshipTransform{});
	registry_->AddComponent(entity_, DirtyMaterial{});
	registry_->AddComponent(entity_, DirtyTextureData{});
	registry_->AddComponent(entity_, DirtyMeshLOD{});
	registry_->AddComponent(entity_, DirtyCullingData{});
	registry_->AddComponent(entity_, instanceAllocator_->Allocate(entity_));
	registry_->AddComponent(entity_, Model{ .name = "sphere.obj" });
	registry_->AddComponent(entity_, RigidBody{ .radius = 1.0f });
	registry_->AddComponent(entity_, footprintManager_->CreateFootprint(entity_, { 1.0f, 1.0f, 1.0f, 1.0f }));
	registry_->AddComponent(entity_, Collision::Sphere{ .radius = 1.0f });
	registry_->AddComponent(entity_, SphereRenderer{});
}

void Player::Update(float deltaTime) {
	auto transform = registry_->GetComponent<EulerTransform>(entity_);
	auto rigidBody = registry_->GetComponent<RigidBody>(entity_);
	auto sphere = registry_->GetComponent<Collision::Sphere>(entity_);
	
	// 摩擦力の計算
	Vector3 horizontalVelocity = rigidBody->velocity;
	horizontalVelocity.y = 0.0f;
	float horizontalSpeed = horizontalVelocity.length();
	if (horizontalSpeed > 0.0f && deltaTime > 0.0f) {
		float frictionMagnitude = -kMiu * rigidBody->mass * kGravity;
		float maxFrictionMagnitude = rigidBody->mass * horizontalSpeed / deltaTime;
		frictionMagnitude = std::min(frictionMagnitude, maxFrictionMagnitude);
		Vector3 frictionForce = -horizontalVelocity.normalized() * frictionMagnitude;
		rigidBody->force += frictionForce;
	}

	// 回転と球の中心位置の更新
	transform->rotateMatrix = MakeRotateMatrix(rigidBody->angularVelocity);
	sphere->center = transform->translate;

	// プレイヤーの移動
	TransformSystem transformSystem{ registry_ };
	Vector3 forward = transformSystem.GetForward(cameraEntity_);
	forward.y = 0.0f;
	forward = forward.normalized();
	Vector3 right = kWorldUp.cross(forward);
	right = right.normalized();
	Vector3 move = right * move_.x + forward * move_.y;
	rigidBody->force += move.normalized() * kMagnitude;
}