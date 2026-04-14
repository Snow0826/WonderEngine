#include "GameScenePhaseClearFadeIn.h"
#include "GameScenePhaseClear.h"
#include "GameScene.h"
#include "EntityComponentSystem.h"
#include "Fade.h"
#include "Transform.h"
#include <numbers>

void GameScenePhaseClearFadeIn::Initialize() {
	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	// フェードの取得
	Fade *fade = gameScene->GetFade();

	// フェードインの開始
	fade->Start(Fade::Status::FadeIn, 1.0f);

	// カメラの位置調整
	Transform *cameraTransform = gameScene->GetRegistry()->GetComponent<Transform>(gameScene->GetMainCameraEntity());
	if (cameraTransform) {
		cameraTransform->rotate = { std::numbers::pi_v<float> / 2.0f, 0.0f, 0.0f };
		cameraTransform->translate = { 0.0f, 700.0f, 0.0f };
	}
}

void GameScenePhaseClearFadeIn::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: ClearFadeIn");
#endif // USE_IMGUI

	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	// フェードの取得
	Fade *fade = gameScene->GetFade();

	// フェードの更新
	fade->Update();

	// フェードインが完了したらクリアフェーズに切り替え
	if (fade->IsFinished()) {
		fade->Stop();
		fade->Remove();
		gameScene->ChangePhase(new GameScenePhaseClear);
	}
}