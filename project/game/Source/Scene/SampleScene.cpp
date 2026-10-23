#define NOMINMAX
#include "SampleScene.h"
#include "EntityComponentSystem.h"
#include "SceneManager.h"
#include "Input.h"
#include "Skybox.h"
#include "SkyboxFactory.h"
#include "GroundFactory.h"
#include "Player.h"
#include "CameraController.h"
#include "Primitive.h"
#include "TreeSpeciesDatabase.h"
#include "PoissonDiskSampler.h"
#include "DebugCamera.h"
#include "DirectionalLight.h"
#include "Logger.h"
#include "CPUTimer.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

SampleScene::SampleScene() = default;
SampleScene::~SampleScene() = default;

void SampleScene::OnInitialize() {
	// マネージャーの取得
	SceneManager *sceneManager = GetSceneManager();
	Registry *registry = GetRegistry();
	InstanceAllocator *instanceAllocator = GetInstanceAllocator();
	FootprintManager *footprintManager = GetFootprintManager();
	TreeGenerator *treeGenerator = GetTreeGenerator();
	MeshManager *meshManager = sceneManager->GetMeshManager();
	TextureManager *textureManager = sceneManager->GetTextureManager();
	ParticleManager *particleManager = sceneManager->GetParticleManager();
	std::ofstream *logStream = sceneManager->GetLogStream();
	Input *input = sceneManager->GetInput();
	PoissonDiskSampler sampler;
	std::vector<Vector2> samples = sampler.Generate(1024.0f, 1024.0f, 16.0f);

	// ジェネレーターの初期化
	SkyboxGenerator skyboxGenerator{ meshManager, textureManager };

	// スカイボックスエンティティの作成
	SkyboxFactory::Create(*registry, skyboxGenerator);

	// 地面の作成
	GroundFactory::Create(*registry, *instanceAllocator);

	// 森の作成
	CPUTimer treeTimer;
	CPUTimer timer;
	treeTimer.Begin();
	for (const Vector2& sample : samples) {
		timer.Begin();
		TreeConfig treeConfig = MakeTreeConfig(TreeSpecies::Cedar, { sample.x - 512.0f, 0.0f, sample.y - 512.0f });
		treeGenerator->Generate(treeConfig);
		timer.End();
		Logger::Log(*logStream, "Tree generation time: " + std::to_string(timer.GetMs()) + " ms\n");
	}
	treeTimer.End();
	Logger::Log(*logStream, "Total tree generation time: " + std::to_string(treeTimer.GetMs()) + " ms\n");


	// プレイヤーの初期化
	player_ = std::make_unique<Player>(registry, instanceAllocator, footprintManager);
	player_->Initialize();
	player_->SetCameraEntity(GetCameraEntity(CameraType::kMainCamera));

	// カメラコントローラーの初期化
	cameraController_ = std::make_unique<CameraController>(registry, input);
	cameraController_->Initialize(GetCameraEntity(CameraType::kMainCamera), player_->GetEntity());

	// 平行光源の設定
	auto directionalLight = registry->GetComponent<DirectionalLight>(GetDirectionalLightEntity());
	directionalLight->direction = { 0.5f, -1.0f, 0.2f };

	// 木の設定エンティティの追加
	uint32_t entity = registry->GenerateEntity();
	registry->AddComponent(entity, Name{ .value = "TreeConfig" });
	registry->AddComponent(entity, MakeTreeConfig(TreeSpecies::Willow, { 0.0f, 0.0f, 0.0f }));
}

void SampleScene::OnUpdate(float deltaTime) {
	SceneManager *sceneManager = GetSceneManager();
	Input *input = sceneManager->GetInput();

	// メインカメラの更新
	if (!IsDebugCameraActive()) {
		cameraController_->Update();
	}

	// プレイヤーの移動量のクリア
	player_->ClearMove();

	// プレイヤーの移動
	if (input->IsPressKey(DIK_W)) {
		player_->Move(0.0f, 1.0f);
	}

	if (input->IsPressKey(DIK_A)) {
		player_->Move(-1.0f, 0.0f);
	}

	if (input->IsPressKey(DIK_S)) {
		player_->Move(0.0f, -1.0f);
	}

	if (input->IsPressKey(DIK_D)) {
		player_->Move(1.0f, 0.0f);
	}

	// プレイヤーの更新
	player_->Update(deltaTime);
}