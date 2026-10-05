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
#include "TreeGenerator.h"
#include "DebugCamera.h"
#include "DirectionalLight.h"
#include "Logger.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	constexpr std::array<TreeConfig, 8> kTrees = {
		TreeConfig{
			.rootPosition = { 20.0f, 0.0f, -5.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
			.crownCenter = { 20.0f, 5.0f, -5.0f }, .crownRadius = { 10.0f, 5.0f, 10.0f },
			.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 8.0f, .killRadius = 1.6f, .branchLength = 0.6f
		},
		TreeConfig{
			.rootPosition = { 20.0f, 0.0f, 20.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
			.crownCenter = { 20.0f, 10.0f, 20.0f }, .crownRadius = { 20.0f, 5.0f, 20.0f },
			.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 8.0f, .killRadius = 1.6f, .branchLength = 0.6f
		},
		TreeConfig{
			.rootPosition = { -10.0f, 0.0f, 20.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
			.crownCenter = { -10.0f, 10.0f, 20.0f }, .crownRadius = { 10.0f, 5.0f, 10.0f },
			.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 16.0f, .killRadius = 1.6f, .branchLength = 0.3f
		},
		TreeConfig{
			.rootPosition = { -25.0f, 0.0f, 25.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
			.crownCenter = { -25.0f, 20.0f, 25.0f }, .crownRadius = { 5.0f, 15.0f, 5.0f },
			.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 16.0f, .killRadius = 1.6f, .branchLength = 0.3f
		},
		TreeConfig{
			.rootPosition = { -30.0f, 0.0f, 0.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
			.crownCenter = { -30.0f, 5.0f, 0.0f }, .crownRadius = { 20.0f, 5.0f, 20.0f },
			.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 16.0f, .killRadius = 0.8f, .branchLength = 0.6f
		},
		TreeConfig{
			.rootPosition = { -15.0f, 0.0f, -20.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
			.crownCenter = { -15.0f, 15.0f, -20.0f }, .crownRadius = { 5.0f, 10.0f, 5.0f },
			.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 16.0f, .killRadius = 0.8f, .branchLength = 0.6f
		},
		TreeConfig{
			.rootPosition = { 0.0f, 0.0f, -20.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
			.crownCenter = { 0.0f, 5.0f, -15.0f }, .crownRadius = { 5.0f, 5.0f, 10.0f },
			.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 8.0f, .killRadius = 0.8f, .branchLength = 0.3f
		},
		TreeConfig{
			.rootPosition = { 20.0f, 0.0f, -20.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
			.crownCenter = { 25.0f, 5.0f, -20.0f }, .crownRadius = { 10.0f, 5.0f, 5.0f },
			.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 8.0f, .killRadius = 0.8f, .branchLength = 0.3f
		},
	};

	TreeConfig generateTree = {
		.rootPosition = { 0.0f, 0.0f, 0.0f }, .rootDirection = { 0.0f, 1.0f, 0.0f },
		.crownCenter = { 0.0f, 5.0f, 0.0f }, .crownRadius = { 10.0f, 5.0f, 10.0f },
		.leafCount = 5000, .minRadius = 0.01f, .gamma = 2.0f, .influenceRadius = 8.0f, .killRadius = 1.6f, .branchLength = 0.3f
	};

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
	MeshManager *meshManager = sceneManager_->GetMeshManager();
	TextureManager *textureManager = sceneManager_->GetTextureManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();
	std::ofstream *logStream = sceneManager_->GetLogStream();
	Input *input = sceneManager_->GetInput();

	// ジェネレーターの初期化
	SkyboxGenerator skyboxGenerator{ meshManager, textureManager };

	// スカイボックスエンティティの作成
	SkyboxEntity::Create(registry_.get(), &skyboxGenerator);

	// 地面の作成
	GroundFactory::Create(*registry_, *instanceAllocator_, groundDesc);

	// 森の作成
	for (const TreeConfig &tree : kTrees) {
		PrimitiveGenerator primitiveGenerator{ meshManager, textureManager };
		TreeGenerator treeGenerator{ registry_.get(), &primitiveGenerator, instanceAllocator_.get() };
		uint32_t treeEntity = treeGenerator.Generate(tree);
		treeEntities_.emplace_back(treeEntity);
	}

	// プレイヤーの初期化
	player_ = std::make_unique<Player>(registry_.get(), instanceAllocator_.get(), footprintManager_.get());
	player_->Initialize();
	player_->SetCameraEntity(cameraEntities_[mainCameraType_]);

	// カメラコントローラーの初期化
	cameraController_ = std::make_unique<CameraController>(registry_.get(), input);
	cameraController_->Initialize(cameraEntities_[mainCameraType_], player_->GetEntity());

	// 平行光源の設定
	auto directionalLight = registry_->GetComponent<DirectionalLight>(directionalLightEntity_);
	directionalLight->direction = { 0.5f, -1.0f, 0.2f };
}

void SampleScene::OnUpdate(float deltaTime) {
	Input *input = sceneManager_->GetInput();
#ifdef USE_IMGUI
	if (ImGui::TreeNode("TreeGenerator")) {
		ImGui::DragFloat3("RootPosition", &generateTree.rootPosition.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("RootDirection", &generateTree.rootDirection.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat3("CrownCenter", &generateTree.crownCenter.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("CrownRadius", &generateTree.crownRadius.x, 0.01f, 1.0f, 10.0f);
		ImGui::DragInt("LeafCount", reinterpret_cast<int *>(&generateTree.leafCount), 1, 100, 5000);
		ImGui::DragFloat("MinRadius", &generateTree.minRadius, 0.01f, 0.01f, 1.0f);
		ImGui::DragFloat("Gamma", &generateTree.gamma, 0.01f, 1.0f, 5.0f);
		ImGui::DragFloat("InfluenceRadius", &generateTree.influenceRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("KillRadius", &generateTree.killRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("BranchLength", &generateTree.branchLength, 0.01f, 0.1f, 1.0f);
		generateTree.rootDirection = generateTree.rootDirection.normalized();
		MeshManager *meshManager = sceneManager_->GetMeshManager();
		TextureManager *textureManager = sceneManager_->GetTextureManager();
		PrimitiveGenerator primitiveGenerator{ meshManager, textureManager };
		TreeGenerator treeGenerator{ registry_.get(), &primitiveGenerator, instanceAllocator_.get() };
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
			AnimatedCube animatedCube{ registry_.get(), instanceAllocator_.get() };
			animatedCube.Create(animatedCubePosition);
		}
		ImGui::TreePop();
	}

	// シンプルスキンの作成
	if (ImGui::TreeNode("SimpleSkin")) {
		ImGui::DragFloat3("Position", &simpleSkinPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			SimpleSkin simpleSkin{ registry_.get(), instanceAllocator_.get() };
			simpleSkin.Create(simpleSkinPosition);
		}
		ImGui::TreePop();
	}

	// 歩く人間の作成
	if (ImGui::TreeNode("WalkHuman")) {
		ImGui::DragFloat3("Position", &walkHumanPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			Human human{ registry_.get(), instanceAllocator_.get() };
			human.Create("walk.gltf", walkHumanPosition);
		}
		ImGui::TreePop();
	}

	// スニークで歩く人間の作成
	if (ImGui::TreeNode("SneakWalkHuman")) {
		ImGui::DragFloat3("Position", &sneakWalkHumanPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			Human human{ registry_.get(), instanceAllocator_.get() };
			human.Create("sneakWalk.gltf", sneakWalkHumanPosition);
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI

	// メインカメラの更新
	if (!isDebugCameraActive_) {
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