#include "GameScenePhaseDeath.h"
#include "GameScenePhasePlay.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "Transform.h"
#include "Player.h"
#include "DeathParticle.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

GameScenePhaseDeath::GameScenePhaseDeath() = default;
GameScenePhaseDeath::~GameScenePhaseDeath() = default;

void GameScenePhaseDeath::Initialize() {
	GameScene *gameScene = GetGameScene();
	Registry *registry = gameScene->GetRegistry();
	TransformSystem transformSystem{ registry };
	Vector3 playerPosition = transformSystem.GetWorldPosition(gameScene->GetPlayer()->GetPartsEntity(Player::PartsType::kBody));

	// デスパーティクルの初期化
	deathParticle_ = std::make_unique<DeathParticle>(registry, gameScene->GetSceneManager()->GetParticleManager());
	deathParticle_->Initialize(playerPosition);
	deathParticle_->Update();
}

void GameScenePhaseDeath::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: Death");
#endif // USE_IMGUI

	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	if (deathParticle_->IsDead()) {
		// デスパーティクルが消滅したらプレイフェーズに切り替え
		gameScene->GetPlayer()->Revival();
		gameScene->ChangePhase(new GameScenePhasePlay);
	} else {
		// 一定時間経過したらクリアフェーズに切り替え
		gameScene->ChangeToGameClearPhase();
	}
}