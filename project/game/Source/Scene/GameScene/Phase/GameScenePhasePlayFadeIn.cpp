#include "GameScenePhasePlayFadeIn.h"
#include "GameScenePhasePlay.h"
#include "GameScene.h"
#include "Fade.h"
#include "Player.h"
#include "CameraController.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void GameScenePhasePlayFadeIn::Initialize() {
	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();
	
	// フェードの取得
	Fade *fade = gameScene->GetFade();
	
	// フェードインの開始
	fade->Add();
	fade->Start(Fade::Status::FadeIn, 1.0f);
}

void GameScenePhasePlayFadeIn::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: PlayFadeIn");
#endif // USE_IMGUI

	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	// フェードの取得
	Fade *fade = gameScene->GetFade();

	// プレイヤーの更新
	gameScene->GetPlayer()->Update();

	// カメラコントローラーの更新
	gameScene->GetCameraController()->Update();

	// フェードの更新
	fade->Update();

	// フェードインが完了したらプレイフェーズに切り替え
	if (fade->IsFinished()) {
		fade->Stop();
		fade->Remove();
		gameScene->ChangePhase(new GameScenePhasePlay);
	}
}