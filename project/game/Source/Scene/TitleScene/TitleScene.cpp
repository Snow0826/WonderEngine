#include "TitleScene.h"
#include "SceneManager.h"
#include "GameScene.h"
#include "Input.h"
#include "Audio.h"
#include "Model.h"
#include "Texture.h"
#include "Skydome.h"
#include "Ground.h"
#include "Wall.h"
#include "Building.h"
#include "AngelStatue.h"
#include "Title.h"
#include "Button.h"
#include "EntityComponentSystem.h"
#include "Camera.h"
#include "Transform.h"
#include "DirectionalLight.h"
#include "TitleScenePhaseFadeIn.h"
#include <numbers>

TitleScene::TitleScene() = default;
TitleScene::~TitleScene() {
	Audio *audio = sceneManager_->GetAudio();
	SoundHandle soundHandle = sceneManager_->GetSoundHandle();
	if (audio->IsPlaying(soundHandle.bgmHandleMap_[BGMType::Title])) {
		audio->Stop(soundHandle.bgmHandleMap_[BGMType::Title]);
	}
	delete phase_;
	phase_ = nullptr;
}

void TitleScene::OnInitialize() {
	// モデルマネージャーの取得
	ModelManager *modelManager = sceneManager_->GetModelManager();

	// 天球の作成
	Skydome::Create(registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get());

	// 地面の作成
	Ground::Create(registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get(), footprintManager_.get());

	// 壁の作成
	Wall wall{ registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get() };
	for (int32_t i = -7; i < 7; i++) {
		wall.Create({ .x = -std::numbers::pi_v<float> / 2.0f }, { static_cast<float>(i + 1) * 36.0f, 4.0f, 256.0f });
		wall.Create({ .x = -std::numbers::pi_v<float> / 2.0f }, { static_cast<float>(i + 1) * 36.0f, 27.0f, 256.0f });
		wall.Create({ .x = -std::numbers::pi_v<float> / 2.0f, .y = std::numbers::pi_v<float> / 2.0f }, { 256.0f, 4.0f, static_cast<float>(i) * 36.0f });
		wall.Create({ .x = -std::numbers::pi_v<float> / 2.0f, .y = std::numbers::pi_v<float> / 2.0f }, { 256.0f, 27.0f, static_cast<float>(i) * 36.0f });
		wall.Create({ .x = -std::numbers::pi_v<float> / 2.0f, .y = std::numbers::pi_v<float> }, { static_cast<float>(i) * 36.0f, 4.0f, -256.0f });
		wall.Create({ .x = -std::numbers::pi_v<float> / 2.0f, .y = std::numbers::pi_v<float> }, { static_cast<float>(i) * 36.0f, 27.0f, -256.0f });
		wall.Create({ .x = -std::numbers::pi_v<float> / 2.0f, .y = std::numbers::pi_v<float> *1.5f }, { -256.0f, 4.0f, static_cast<float>(i + 1) * 36.0f });
		wall.Create({ .x = -std::numbers::pi_v<float> / 2.0f, .y = std::numbers::pi_v<float> *1.5f }, { -256.0f, 27.0f, static_cast<float>(i + 1) * 36.0f });
	}

	// 建物の作成
	Building building{ registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get() };
	for (size_t i = 0; i < 3; i++) {
		building.Create({}, { .x = -44.0f, .z = static_cast<float>(i) * -40.0f - 75.0f });
		building.Create({}, { .x = -44.0f, .z = static_cast<float>(i) * 40.0f + 112.0f });
		building.Create({}, { .x = 24.0f, .z = static_cast<float>(i) * -40.0f - 75.0f });
		building.Create({}, { .x = 24.0f, .z = static_cast<float>(i) * 40.0f + 112.0f });
		building.Create({ .y = std::numbers::pi_v<float> / 2.0f }, { .x = static_cast<float>(i) * -40.0f - 75.0f, .z = 44.0f });
		building.Create({ .y = std::numbers::pi_v<float> / 2.0f }, { .x = static_cast<float>(i) * 40.0f + 112.0f, .z = 44.0f });
		building.Create({ .y = std::numbers::pi_v<float> / 2.0f }, { .x = static_cast<float>(i) * -40.0f - 75.0f, .z = -24.0f });
		building.Create({ .y = std::numbers::pi_v<float> / 2.0f }, { .x = static_cast<float>(i) * 40.0f + 112.0f, .z = -24.0f });
		building.Create({ .y = std::numbers::pi_v<float> }, { .x = -24.0f, .z = static_cast<float>(i) * -40.0f - 112.0f });
		building.Create({ .y = std::numbers::pi_v<float> }, { .x = -24.0f, .z = static_cast<float>(i) * 40.0f + 75.0f });
		building.Create({ .y = std::numbers::pi_v<float> }, { .x = 44.0f, .z = static_cast<float>(i) * -40.0f - 112.0f });
		building.Create({ .y = std::numbers::pi_v<float> }, { .x = 44.0f, .z = static_cast<float>(i) * 40.0f + 75.0f });
		building.Create({ .y = std::numbers::pi_v<float> *1.5f }, { .x = static_cast<float>(i) * -40.0f - 112.0f, .z = 24.0f });
		building.Create({ .y = std::numbers::pi_v<float> *1.5f }, { .x = static_cast<float>(i) * 40.0f + 75.0f, .z = 24.0f });
		building.Create({ .y = std::numbers::pi_v<float> *1.5f }, { .x = static_cast<float>(i) * -40.0f - 112.0f, .z = -44.0f });
		building.Create({ .y = std::numbers::pi_v<float> *1.5f }, { .x = static_cast<float>(i) * 40.0f + 75.0f, .z = -44.0f });
	}

	for (size_t i = 0; i < 2; i++) {
		building.Create({}, { .x = 64.0f, .z = -15.0f + static_cast<float>(i) * 68.0f });
		building.Create({ .y = std::numbers::pi_v<float> / 2.0f }, { .x = -15.0f + static_cast<float>(i) * 68.0f, .z = -64.0f });
		building.Create({ .y = std::numbers::pi_v<float> }, { .x = -64.0f, .z = -53.0f + static_cast<float>(i) * 68.0f });
		building.Create({ .y = std::numbers::pi_v<float> *1.5f }, { .x = -53.0f + static_cast<float>(i) * 68.0f, .z = 64.0f });
	}

	// 天使像の作成
	AngelStatue angelStatue{ registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get() };
	angelStatue.Create({}, { .y = 8.0f, .z = -250.0f });
	angelStatue.Create({ .y = std::numbers::pi_v<float> / 2.0f }, { .x = -250.0f, .y = 8.0f });
	angelStatue.Create({ .y = std::numbers::pi_v<float> }, { .y = 8.0f, .z = 250.0f });
	angelStatue.Create({ .y = std::numbers::pi_v<float> *1.5f }, { .x = 250.0f, .y = 8.0f });

	// タイトルの作成
	Title title{ registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get() };
	title.Create("title0.obj", { std::numbers::pi_v<float> *1.5f, -std::numbers::pi_v<float> / 6.0f, 0.0f }, { -8.0f, 4.0f, 20.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
	title.Create("title1.obj", { std::numbers::pi_v<float> *1.5f, 0.0f, 0.0f }, { -2.0f, 4.0f, 20.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
	title.Create("title2.obj", { std::numbers::pi_v<float> *1.5f, std::numbers::pi_v<float> / 6.0f, 0.0f }, { 4.0f, 4.0f, 20.0f }, { 0.0f, 0.0f, 1.0f, 1.0f });

	// スタートボタンの初期化
	startButton_ = std::make_unique<Button>(registry_.get(), spriteManager_.get(), objectManager_.get());
	startButton_->Initialize();

	// カメラの設定
	Camera *camera = registry_->GetComponent<Camera>(cameraEntities_[mainCameraType_]);
	camera->farZ /= 2.0f;
	Transform *transform = registry_->GetComponent<Transform>(cameraEntities_[mainCameraType_]);
	transform->translate = { .y = 2.0f };
	transform->rotateMatrix = LookAt(transform->translate, { .z = 256.0f }, { .y = 1.0f });

	// 平行光源の設定
	DirectionalLight *directionalLight = registry_->GetComponent<DirectionalLight>(directionalLightEntity_);
	directionalLight->direction = Vector3{ 1.0f, -1.0f, 0.0f }.normalized();

	// フェーズの初期化
	ChangePhase(new TitleScenePhaseFadeIn);

	// BGMの再生
	sceneManager_->GetAudio()->Play(sceneManager_->GetSoundHandle().bgmHandleMap_[BGMType::Title], true);
}

void TitleScene::OnUpdate() {
	phase_->Update();
	startButton_->Update();
}

void TitleScene::ChangePhase(BaseTitleScenePhase *newBaseTitleScenePhase) {
	// 古いフェーズを削除
	if (phase_) {
		delete phase_;
		phase_ = nullptr;
	}

	// 新しいフェーズを設定
	phase_ = newBaseTitleScenePhase;

	// 新しいフェーズの初期化
	phase_->SetTitleScene(this);
	phase_->Initialize();
}