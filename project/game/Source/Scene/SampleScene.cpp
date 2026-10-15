#define NOMINMAX
#include "SampleScene.h"
#include "EntityComponentSystem.h"
#include "SceneManager.h"
#include "Input.h"
#include "Model.h"
#include "Skybox.h"
#include "SkyboxEntity.h"
#include "Ground.h"
#include "AnimatedCube.h"
#include "SimpleSkin.h"
#include "Human.h"
#include "Player.h"
#include "CameraController.h"
#include "Primitive.h"
#include "TreeSpeciesDatabase.h"
#include "PoissonDiskSampler.h"
#include "DebugCamera.h"
#include "DirectionalLight.h"
#include "Random.h"
#include "Logger.h"
#include "CPUTimer.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	TreeConfig generateTree;
	GroundDesc groundDesc = {
		.modelName = "ground.obj",
		.enableLighting = false,
		.terrainOriginXZ = Vector2{ 0.0f, 0.0f },
		.terrainSizeXZ = Vector2{ 1024.0f, 1024.0f }
	};

	Vector3 animatedCubePosition{ 0.0f, 0.0f, 20.0f };
	Vector3 simpleSkinPosition{ -3.0f, 0.0f, 5.0f };
	Vector3 walkHumanPosition{ 0.0f, 0.0f, 5.0f };
	Vector3 sneakWalkHumanPosition{ 3.0f, 0.0f, 5.0f };
}

SampleScene::SampleScene() = default;
SampleScene::~SampleScene() = default;

void SampleScene::OnInitialize() {
	// マネージャーの取得
	SceneManager *sceneManager = GetSceneManager();
	Registry *registry = GetRegistry();
	InstanceAllocator *instanceAllocator = GetInstanceAllocator();
	FootprintManager *footprintManager = GetFootprintManager();
	MeshManager *meshManager = sceneManager->GetMeshManager();
	TextureManager *textureManager = sceneManager->GetTextureManager();
	ParticleManager *particleManager = sceneManager->GetParticleManager();
	std::ofstream *logStream = sceneManager->GetLogStream();
	Input *input = sceneManager->GetInput();
	PrimitiveGenerator primitiveGenerator{ meshManager, textureManager };
	TreeGenerator treeGenerator{ registry, &primitiveGenerator, instanceAllocator, logStream };
	PoissonDiskSampler sampler;
	std::vector<Vector2> samples = sampler.Generate(1024.0f, 1024.0f, 64.0f);

	// ジェネレーターの初期化
	SkyboxGenerator skyboxGenerator{ meshManager, textureManager };

	// スカイボックスエンティティの作成
	SkyboxEntity::Create(registry, &skyboxGenerator);

	// 地面の作成
	GroundFactory::Create(*registry, *instanceAllocator, groundDesc);

	// 森の作成
	CPUTimer treeTimer;
	CPUTimer timer;
	treeTimer.Begin();
	for (const Vector2& sample : samples) {
		timer.Begin();
		TreeConfig treeConfig = MakeTreeConfig(TreeSpecies::Willow, { sample.x - 512.0f, 0.0f, sample.y - 512.0f });
		uint32_t treeEntity = treeGenerator.Generate(treeConfig);
		treeEntities_.emplace_back(treeEntity);
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
}

void SampleScene::OnUpdate(float deltaTime) {
	SceneManager *sceneManager = GetSceneManager();
	Registry *registry = GetRegistry();
	InstanceAllocator *instanceAllocator = GetInstanceAllocator();
	Input *input = sceneManager->GetInput();
	std::ofstream *logStream = sceneManager->GetLogStream();
#ifdef USE_IMGUI
	if (ImGui::TreeNode("TreeGenerator")) {
		ImGui::DragFloat3("RootPosition", &generateTree.rootPosition.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("RootDirection", &generateTree.rootDirection.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat3("CrownCenter", &generateTree.crown.center.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("CrownRadius", &generateTree.crown.radius.x, 0.01f, 1.0f, 10.0f);
		ImGui::DragInt("LeafCount", reinterpret_cast<int *>(&generateTree.leafCount), 1, 100, 5000);
		ImGui::DragFloat("MinRadius", &generateTree.minRadius, 0.01f, 0.01f, 1.0f);
		ImGui::DragFloat("Gamma", &generateTree.gamma, 0.01f, 1.0f, 5.0f);
		ImGui::DragFloat("InfluenceRadius", &generateTree.influenceRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("KillRadius", &generateTree.killRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("BranchLength", &generateTree.branchLength, 0.01f, 0.1f, 1.0f);
		generateTree.rootDirection = generateTree.rootDirection.normalized();
		MeshManager *meshManager = sceneManager->GetMeshManager();
		TextureManager *textureManager = sceneManager->GetTextureManager();
		PrimitiveGenerator primitiveGenerator{ meshManager, textureManager };
		TreeGenerator treeGenerator{ registry, &primitiveGenerator, instanceAllocator, logStream };
		if (ImGui::Button("Generate")) {
			uint32_t treeEntity = treeGenerator.Generate(generateTree);
			treeEntities_.emplace_back(treeEntity);
		}

		if (ImGui::Button("Delete") && !treeEntities_.empty()) {
			treeGenerator.Delete(treeEntities_.back());
			treeEntities_.pop_back();
		}
		ImGui::TreePop();
	}

	// アニメーションするキューブの作成
	if (ImGui::TreeNode("AnimatedCube")) {
		ImGui::DragFloat3("Position", &animatedCubePosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			AnimatedCube animatedCube{ registry, instanceAllocator };
			animatedCube.Create(animatedCubePosition);
		}
		ImGui::TreePop();
	}

	// シンプルスキンの作成
	if (ImGui::TreeNode("SimpleSkin")) {
		ImGui::DragFloat3("Position", &simpleSkinPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			SimpleSkin simpleSkin{ registry, instanceAllocator };
			simpleSkin.Create(simpleSkinPosition);
		}
		ImGui::TreePop();
	}

	// 歩く人間の作成
	if (ImGui::TreeNode("WalkHuman")) {
		ImGui::DragFloat3("Position", &walkHumanPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			Human human{ registry, instanceAllocator };
			human.Create("walk.gltf", walkHumanPosition);
		}
		ImGui::TreePop();
	}

	// スニークで歩く人間の作成
	if (ImGui::TreeNode("SneakWalkHuman")) {
		ImGui::DragFloat3("Position", &sneakWalkHumanPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			Human human{ registry, instanceAllocator };
			human.Create("sneakWalk.gltf", sneakWalkHumanPosition);
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI

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