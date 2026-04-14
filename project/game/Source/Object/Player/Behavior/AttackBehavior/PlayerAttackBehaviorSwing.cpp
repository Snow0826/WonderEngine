#include "PlayerAttackBehaviorSwing.h"
#include "PlayerAttackBehaviorEnd.h"
#include "PlayerBehaviorAttack.h"
#include "Player.h"
#include "AttackParticle.h"
#include "EntityComponentSystem.h"
#include "Transform.h"
#include "Easing.h"

void PlayerAttackBehaviorSwing::Update() {
#ifdef USE_IMGUI
	ImGui::Text("AttackBehavior: Swing");
#endif // USE_IMGUI

	// プレイヤーの攻撃の振る舞いの取得
	PlayerBehaviorAttack *playerBehaviorAttack = GetPlayerBehaviorAttack();

	// プレイヤーの取得
	Player *player = playerBehaviorAttack->GetPlayer();

	// レジストリの取得
	Registry *registry = player->GetRegistry();

	// 左腕の更新
	Transform *leftArmTransform = registry->GetComponent<Transform>(player->GetPartsEntity(Player::PartsType::kLeftArm));
	if (leftArmTransform) {
		leftArmTransform->rotate = Easing<Vector3>::InCirc(kLeftArmAttackRotate, playerBehaviorAttack->GetLeftArmRotate(), swingAttackTimer_, kSwingAttackDuration);
		leftArmTransform->translate = Easing<Vector3>::InCirc(kLeftArmAttackTranslate, playerBehaviorAttack->GetLeftArmTranslate(), swingAttackTimer_, kSwingAttackDuration);
	}

	// 右腕の更新
	Transform *rightArmTransform = registry->GetComponent<Transform>(player->GetPartsEntity(Player::PartsType::kRightArm));
	if (rightArmTransform) {
		rightArmTransform->rotate = Easing<Vector3>::InCirc(kRightArmAttackRotate, playerBehaviorAttack->GetRightArmRotate(), swingAttackTimer_, kSwingAttackDuration);
		rightArmTransform->translate = Easing<Vector3>::InCirc(kRightArmAttackTranslate, playerBehaviorAttack->GetRightArmTranslate(), swingAttackTimer_, kSwingAttackDuration);
	}

	// 攻撃パーティクルの更新
	player->GetAttackParticle()->Update();

	// タイマーの更新
	swingAttackTimer_++;
	if (swingAttackTimer_ > kSwingAttackDuration) {
		swingAttackTimer_ = 0;
		playerBehaviorAttack->SetNextAttackBehavior(std::make_unique<PlayerAttackBehaviorEnd>());
	}
}