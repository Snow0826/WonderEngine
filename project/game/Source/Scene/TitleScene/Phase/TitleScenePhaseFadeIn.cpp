#include "TitleScenePhaseFadeIn.h"
#include "TitleScenePhaseMain.h"
#include "TitleScene.h"
#include "Fade.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void TitleScenePhaseFadeIn::Initialize() {
	// タイトルシーンの取得
	TitleScene *titleScene = GetTitleScene();
	
	// フェードの取得
	Fade *fade = titleScene->GetFade();
	
	// フェードインの開始
	fade->Add();
	fade->Start(Fade::Status::FadeIn, 1.0f);
}

void TitleScenePhaseFadeIn::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: FadeIn");
#endif // USE_IMGUI

	// タイトルシーンの取得
	TitleScene *titleScene = GetTitleScene();

	// フェードの取得
	Fade *fade = titleScene->GetFade();

	// フェードの更新
	fade->Update();

	// フェードインが完了したらメインフェーズに切り替え
	if (fade->IsFinished()) {
		fade->Stop();
		fade->Remove();
		titleScene->ChangePhase(new TitleScenePhaseMain);
	}
}