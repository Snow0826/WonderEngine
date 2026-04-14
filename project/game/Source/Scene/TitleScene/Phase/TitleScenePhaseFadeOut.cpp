#include "TitleScenePhaseFadeOut.h"
#include "TitleScene.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "Fade.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void TitleScenePhaseFadeOut::Initialize() {
	// タイトルシーンの取得
	TitleScene *titleScene = GetTitleScene();
	
	// フェードの取得
	Fade *fade = titleScene->GetFade();
	
	// フェードアウトの開始
	fade->Add();
	fade->Start(Fade::Status::FadeOut, 1.0f);
}

void TitleScenePhaseFadeOut::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: FadeOut");
#endif // USE_IMGUI

	// タイトルシーンの取得
	TitleScene *titleScene = GetTitleScene();

	// フェードの取得
	Fade *fade = titleScene->GetFade();

	// フェードの更新
	fade->Update();

	// フェードアウトが完了したらゲームシーンに切り替え
	if (fade->IsFinished()) {
		fade->Stop();
		fade->Remove();
		titleScene->GetSceneManager()->SetNextScene(new GameScene);
	}
}