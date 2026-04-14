#include "TitleScenePhaseMain.h"
#include "TitleScenePhaseFadeOut.h"
#include "TitleScene.h"
#include "SceneManager.h"
#include "SoundHandle.h"
#include "Input.h"
#include "Audio.h"
#include "Button.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

void TitleScenePhaseMain::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: Main");
#endif // USE_IMGUI

	// タイトルシーンの取得
	TitleScene *titleScene = GetTitleScene();

	// シーンマネージャーの取得
	SceneManager *sceneManager = titleScene->GetSceneManager();

	// 入力の取得
	Input *input = sceneManager->GetInput();

	// オーディオの取得
	Audio *audio = sceneManager->GetAudio();

	// 音声ハンドルの取得
	SoundHandle soundHandle = sceneManager->GetSoundHandle();

	// キーボード入力によるフェーズ移行
	if (input->IsTriggerKey(DIK_SPACE)) {
		audio->Play(soundHandle.seHandleMap_[SEType::Start], false);
		titleScene->GetStartButton()->StartAnimation();
		titleScene->ChangePhase(new TitleScenePhaseFadeOut);
		return;
	}

	// コントローラー入力によるフェーズ移行
	if (input->Connected()) {
		if (input->IsTriggerButton(XINPUT_GAMEPAD_A)) {
			audio->Play(soundHandle.seHandleMap_[SEType::Start], false);
			titleScene->GetStartButton()->StartAnimation();
			titleScene->ChangePhase(new TitleScenePhaseFadeOut);
			return;
		}
	} else {
#ifdef USE_IMGUI
		ImGui::Text("No Controller Connected");
#endif // USE_IMGUI
	}
}