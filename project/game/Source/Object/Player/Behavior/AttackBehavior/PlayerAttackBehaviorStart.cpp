#include "PlayerAttackBehaviorStart.h"
#include "PlayerAttackBehaviorSwing.h"
#include "PlayerBehaviorAttack.h"
#include "Player.h"
#include "AttackParticle.h"
#include "EntityComponentSystem.h"
#include "Transform.h"
#include "Easing.h"

void PlayerAttackBehaviorStart::Update() {
#ifdef USE_IMGUI
	ImGui::Text("AttackBehavior: Start");
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
		leftArmTransform->rotate = Easing<Vector3>::InCirc(playerBehaviorAttack->GetLeftArmRotate(), kLeftArmAttackRotate, startAttackTimer_, kStartAttackDuration);
		leftArmTransform->translate = Easing<Vector3>::InCirc(playerBehaviorAttack->GetLeftArmTranslate(), kLeftArmAttackTranslate, startAttackTimer_, kStartAttackDuration);
	}

	// 右腕の更新
	Transform *rightArmTransform = registry->GetComponent<Transform>(player->GetPartsEntity(Player::PartsType::kRightArm));
	if (rightArmTransform) {
		rightArmTransform->rotate = Easing<Vector3>::InCirc(playerBehaviorAttack->GetRightArmRotate(), kRightArmAttackRotate, startAttackTimer_, kStartAttackDuration);
		rightArmTransform->translate = Easing<Vector3>::InCirc(playerBehaviorAttack->GetRightArmTranslate(), kRightArmAttackTranslate, startAttackTimer_, kStartAttackDuration);
	}

	// タイマーの更新
	startAttackTimer_++;
	if (startAttackTimer_ > kStartAttackDuration) {
		startAttackTimer_ = 0;
		player->GetAttackParticle()->Start();
		playerBehaviorAttack->SetNextAttackBehavior(std::make_unique<PlayerAttackBehaviorSwing>());
	}
}