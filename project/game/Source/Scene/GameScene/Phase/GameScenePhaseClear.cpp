#include "GameScenePhaseClear.h"
#include "GameScenePhaseClearFadeOut.h"
#include "GameScene.h"
#include "SceneManager.h"
#include "Input.h"
#include "Audio.h"
#include "SoundHandle.h"
#include "World.h"
#include "EntityComponentSystem.h"
#include "BlendMode.h"
#include "Transform.h"
#include "Material.h"
#include "Object.h"
#include "Sprite.h"

void GameScenePhaseClear::Initialize() {
	GameScene *gameScene = GetGameScene();
	Registry *registry = gameScene->GetRegistry();
	SpriteManager *spriteManager = gameScene->GetSpriteManager();
	ObjectManager *objectManager = gameScene->GetObjectManager();
	Int4 color = gameScene->GetSceneManager()->GetWorld()->GetColor();
	if (color.x > color.z) {
		// WINエンティティの生成
		entity_ = registry->GenerateEntity();
		registry->AddComponent(entity_, BlendMode::kBlendModeNormal);
		registry->AddComponent(entity_, spriteManager->CreateSprite("win.png"));
		registry->AddComponent(entity_, objectManager->CreateObject(entity_));
		registry->AddComponent(entity_, Transform{});
		registry->AddComponent(entity_, Material{ .enableLighting = false });
	} else {
		// LOSEエンティティの生成
		entity_ = registry->GenerateEntity();
		registry->AddComponent(entity_, BlendMode::kBlendModeNormal);
		registry->AddComponent(entity_, spriteManager->CreateSprite("lose.png"));
		registry->AddComponent(entity_, objectManager->CreateObject(entity_));
		registry->AddComponent(entity_, Transform{});
		registry->AddComponent(entity_, Material{ .enableLighting = false });
	}
}

void GameScenePhaseClear::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Phase: Clear");
#endif // USE_IMGUI

	// ゲームシーンの取得
	GameScene *gameScene = GetGameScene();

	// シーンマネージャーの取得
	SceneManager *sceneManager = gameScene->GetSceneManager();

	// 入力の取得
	Input *input = sceneManager->GetInput();

	// オーディオの取得
	Audio *audio = sceneManager->GetAudio();

	// 音声ハンドルの取得
	SoundHandle soundHandle = sceneManager->GetSoundHandle();

	// キーボード入力によるフェーズ移行
	if (input->IsTriggerKey(DIK_SPACE)) {
		audio->Play(soundHandle.seHandleMap_[SEType::Decide], false);
		gameScene->ChangePhase(new GameScenePhaseClearFadeOut);
		return;
	}

	// コントローラー入力によるフェーズ移行
	if (input->Connected()) {
		if (input->IsTriggerButton(XINPUT_GAMEPAD_A)) {
			audio->Play(soundHandle.seHandleMap_[SEType::Decide], false);
			gameScene->ChangePhase(new GameScenePhaseClearFadeOut);
			return;
		}
	} else {
#ifdef USE_IMGUI
		ImGui::Text("No Controller Connected");
#endif // USE_IMGUI
	}
}