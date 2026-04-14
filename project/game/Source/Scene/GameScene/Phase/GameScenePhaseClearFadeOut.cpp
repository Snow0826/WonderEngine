#include "GameScenePhaseClearFadeOut.h"
#include "GameScene.h"
#include "Fade.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "Renderer.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void GameScenePhaseClearFadeOut::Initialize() {
	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	// フェードの取得
	Fade *fade = gameScene->GetFade();

	// フェードアウトの開始
	fade->Add();
	fade->Start(Fade::Status::FadeOut, 1.0f);
}

void GameScenePhaseClearFadeOut::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: ClearFadeOut");
#endif // USE_IMGUI

	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	// フェードの取得
	Fade *fade = gameScene->GetFade();

	// フェードの更新
	fade->Update();

	// フェードアウトが完了したらタイトルシーンに切り替え
	if (fade->IsFinished()) {
		SceneManager *sceneManager = gameScene->GetSceneManager();
		fade->Stop();
		fade->Remove();
		sceneManager->GetRenderer()->SetGameFinished();
		sceneManager->SetNextScene(new TitleScene);
	}
}