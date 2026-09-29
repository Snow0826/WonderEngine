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
	registry_->AddComponent(entity_, modelManager_->FindModel("sphere.obj"));
	registry_->AddComponent(entity_, RigidBody{ .radius = 1.0f });
	registry_->AddComponent(entity_, footprintManager_->CreateFootprint(entity_, { 1.0f, 1.0f, 1.0f, 1.0f }));
	registry_->AddComponent(entity_, Collision::Sphere{ .radius = 1.0f });
	registry_->AddComponent(entity_, SphereRenderer{});
}

void Player::Update() {
	auto transform = registry_->GetComponent<EulerTransform>(entity_);
	auto rigidBody = registry_->GetComponent<RigidBody>(entity_);
	auto sphere = registry_->GetComponent<Collision::Sphere>(entity_);
	float magnitude = -kMiu * rigidBody->mass * kGravity;
	Vector3 frictionForce = -rigidBody->velocity.normalized() * magnitude;
	rigidBody->force += frictionForce;
	transform->rotateMatrix = MakeRotateMatrix(rigidBody->angularVelocity);
	sphere->center = transform->translate;
}

void Player::Move(float x, float z) {
	auto rigidBody = registry_->GetComponent<RigidBody>(entity_);
	rigidBody->force += Vector3{ x, 0.0f, z } * 40.0f;
}