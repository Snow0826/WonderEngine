#include "PlayerBehaviorAttack.h"
#include "PlayerAttackBehaviorStart.h"
#include "Player.h"
#include "EntityComponentSystem.h"
#include "Sphere.h"
#include "Transform.h"
#include "RigidBody.h"

PlayerBehaviorAttack::PlayerBehaviorAttack() = default;
PlayerBehaviorAttack::~PlayerBehaviorAttack() = default;

void PlayerBehaviorAttack::Initialize() {
	// プレイヤーの取得
	Player *player = GetPlayer();

	// レジストリの取得
	Registry *registry = player->GetRegistry();

	// コライダーの追加
	registry->AddComponent(player->GetPartsEntity(Player::PartsType::kLeftArm), SphereCollider{});
	registry->AddComponent(player->GetPartsEntity(Player::PartsType::kRightArm), SphereCollider{});

	// 左腕の初期パラメーターの保存
	Transform *leftArmTransform = registry->GetComponent<Transform>(player->GetPartsEntity(Player::PartsType::kLeftArm));
	if (leftArmTransform) {
		leftArmTransform->rotate.x = 0.0f;
		leftArmRotate_ = leftArmTransform->rotate;
		leftArmTranslate_ = leftArmTransform->translate;
	}

	// 右腕の初期パラメーターの保存
	Transform *rightArmTransform = registry->GetComponent<Transform>(player->GetPartsEntity(Player::PartsType::kRightArm));
	if (rightArmTransform) {
		rightArmTransform->rotate.x = 0.0f;
		rightArmRotate_ = rightArmTransform->rotate;
		rightArmTranslate_ = rightArmTransform->translate;
	}

	// 現在の振る舞いの初期化
	currentAttackBehavior_ = std::make_unique<PlayerAttackBehaviorStart>();
	currentAttackBehavior_->SetPlayerBehaviorAttack(this);
}

void PlayerBehaviorAttack::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Behavior: Attack");
#endif // USE_IMGUI

	RigidBody *rigidBody = GetPlayer()->GetRegistry()->GetComponent<RigidBody>(GetPlayer()->GetPartsEntity(Player::PartsType::kBody));
	if (rigidBody) {
		rigidBody->velocity.x = 0.0f;
		rigidBody->velocity.z = 0.0f;
	}

	// 現在の振る舞いの更新
	if (currentAttackBehavior_) {
		currentAttackBehavior_->Update();
	}

	// 振る舞いの変更
	if (nextAttackBehavior_) {
		currentAttackBehavior_ = std::move(nextAttackBehavior_);
		nextAttackBehavior_ = nullptr;
		currentAttackBehavior_->SetPlayerBehaviorAttack(this);
	}
}

void PlayerBehaviorAttack::SetNextAttackBehavior(std::unique_ptr<BasePlayerAttackBehavior> nextAttackBehavior) {
	nextAttackBehavior_ = std::move(nextAttackBehavior);
}

Player *PlayerBehaviorAttack::GetPlayer() const {
	return BasePlayerBehavior::GetPlayer();
}