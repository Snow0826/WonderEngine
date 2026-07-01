#include "PlayerAttackBehaviorEnd.h"
#include "PlayerBehaviorRoot.h"
#include "PlayerBehaviorAttack.h"
#include "Player.h"
#include "AttackParticle.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void PlayerAttackBehaviorEnd::Update() {
#ifdef USE_IMGUI
	ImGui::Text("AttackBehavior: End");
#endif // USE_IMGUI

	// プレイヤーの攻撃の振る舞いの取得
	PlayerBehaviorAttack *playerBehaviorAttack = GetPlayerBehaviorAttack();

	// プレイヤーの取得
	Player *player = playerBehaviorAttack->GetPlayer();

	// タイマーの更新
	endAttackTimer_++;
	if (endAttackTimer_ > kEndAttackDuration) {
		endAttackTimer_ = 0;
		player->GetAttackParticle()->Stop();
		player->SetNextBehavior(std::make_unique<PlayerBehaviorRoot>());
	}
}