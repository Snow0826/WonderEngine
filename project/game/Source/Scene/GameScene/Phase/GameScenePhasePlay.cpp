#include "GameScenePhasePlay.h"
#include "GameScenePhaseDeath.h"
#include "GameScene.h"
#include "Player.h"
#include "CameraController.h"
#include "Fade.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void GameScenePhasePlay::Initialize() {
	GetGameScene()->GetRuleUIFade()->Start(Fade::Status::FadeIn, 1.0f);
}

void GameScenePhasePlay::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: Play");
#endif // USE_IMGUI

	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	// ルールUIフェードの取得
	Fade *ruleUIFade = gameScene->GetRuleUIFade();

	// プレイヤーの更新
	gameScene->GetPlayer()->Update();

	// カメラコントローラーの更新
	gameScene->GetCameraController()->Update();

	// ルールUIフェードの更新
	ruleUIFade->Update();

	// ルールUIフェードが終了したら削除
	if (ruleUIFade->IsFinished()) {
		ruleUIFade->Stop();
		ruleUIFade->Remove();
	}

	// 敵の出現
	gameScene->PopEnemy();

	if (gameScene->GetPlayer()->IsDead()) {
		// プレイヤーが死亡したら死亡フェーズに切り替え
		gameScene->ChangePhase(new GameScenePhaseDeath);
	} else {
		// 一定時間経過したらクリアフェーズに切り替え
		gameScene->ChangeToGameClearPhase();
	}
}