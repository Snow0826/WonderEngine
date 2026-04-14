#include "GameScenePhasePlayFadeOut.h"
#include "GameScenePhaseClearFadeIn.h"
#include "GameScene.h"
#include "Fade.h"
#include "BitmapFont.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void GameScenePhasePlayFadeOut::Initialize() {
	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();
	
	// フェードの取得
	Fade *fade = gameScene->GetFade();
	
	// フェードアウトの開始
	fade->Add();
	fade->Start(Fade::Status::FadeOut, 1.0f);
}

void GameScenePhasePlayFadeOut::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: PlayFadeOut");
#endif // USE_IMGUI

	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	// フェードの取得
	Fade *fade = gameScene->GetFade();

	// フェードの更新
	fade->Update();

	// フェードアウトが完了したらクリアフェーズに切り替え
	if (fade->IsFinished()) {
		fade->Stop();
		gameScene->ClearUI();
		gameScene->GetBitmapFont()->Clear();
		gameScene->ChangePhase(new GameScenePhaseClearFadeIn);
	}
}