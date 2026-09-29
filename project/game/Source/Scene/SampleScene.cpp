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
#include "Primitive.h"
#include "TreeGenerator.h"
#include "DebugCamera.h"
#include "DirectionalLight.h"
#include "Logger.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	std::vector<Vector3> rootPositionList = {
		{ 20.0f, 0.0f, -5.0f }, { 20.0f, 0.0f, 20.0f }, { -10.0f, 0.0f, 20.0f }, { -25.0f, 0.0f, 25.0f },
		{ -30.0f, 0.0f, 0.0f }, { -15.0f, 0.0f, -20.0f }, { 0.0f, 0.0f, -20.0f }, { 20.0f, 0.0f, -20.0f }
	};
	std::vector<Vector3> rootDirectionList = {
		{ 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }
	};
	std::vector<Vector3> crownCenterList = {
		{ 20.0f, 5.0f, -5.0f }, { 20.0f, 10.0f, 20.0f }, { -10.0f, 10.0f, 20.0f }, { -25.0f, 20.0f, 25.0f },
		{ -30.0f, 5.0f, 0.0f }, { -15.0f, 15.0f, -20.0f }, { 0.0f, 5.0f, -15.0f }, { 25.0f, 5.0f, -20.0f }
	};
	std::vector<Vector3> crownRadiusList = {
		{ 10.0f, 5.0f, 10.0f }, { 20.0f, 5.0f, 20.0f }, { 10.0f, 5.0f, 10.0f }, { 5.0f, 15.0f, 5.0f },
		{ 20.0f, 5.0f, 20.0f }, { 5.0f, 10.0f, 5.0f }, { 5.0f, 5.0f, 10.0f }, { 10.0f, 5.0f, 5.0f }
	};
	std::vector<uint32_t> leafCountList = { 5000, 5000, 5000, 5000, 5000, 5000, 5000, 5000 };
	std::vector<float> minRadiusList = { 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f, 0.01f };
	std::vector<float> gammaList = { 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f };
	std::vector<float> influenceRadiusList = { 8.0f, 8.0f, 16.0f, 16.0f, 16.0f, 16.0f, 8.0f, 8.0f };
	std::vector<float> killRadiusList = { 1.6f, 1.6f, 1.6f, 1.6f, 0.8f, 0.8f, 0.8f, 0.8f };
	std::vector<float> branchLengthList = { 0.6f, 0.6f, 0.3f, 0.3f, 0.6f, 0.6f, 0.3f, 0.3f };
	Vector3 rootPosition{ 0.0f, 0.0f, 0.0f };
	Vector3 rootDirection{ 0.0f, 1.0f, 0.0f };
	Vector3 crownCenter{ 0.0f, 5.0f, 0.0f };
	Vector3 crownRadius{ 10.0f, 5.0f, 10.0f };
	uint32_t leafCount = 5000;
	float minRadius = 0.01f;
	float gamma = 2.0f;
	float influenceRadius = 8.0f;
	float killRadius = 1.6f;
	float branchLength = 0.3f;
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
	ModelManager *modelManager = sceneManager_->GetModelManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();
	std::ofstream *logStream = sceneManager_->GetLogStream();

	// ジェネレーターの初期化
	SkyboxGenerator skyboxGenerator{ meshManager, textureManager };

	// スカイボックスエンティティの作成
	SkyboxEntity::Create(registry_.get(), &skyboxGenerator);

	// 地面の作成
	Ground::Create(registry_.get(), modelManager, instanceAllocator_.get());

	// 森の作成
	for (size_t i = 0; i < 8; i++) {
		PrimitiveGenerator primitiveGenerator{ meshManager, textureManager };
		TreeGenerator treeGenerator{ registry_.get(), &primitiveGenerator, instanceAllocator_.get() };
		uint32_t treeEntity = treeGenerator.Generate(rootPositionList[i], rootDirectionList[i], crownCenterList[i], crownRadiusList[i], leafCountList[i], minRadiusList[i], gammaList[i], influenceRadiusList[i], killRadiusList[i]	, branchLengthList[i]);
		treeEntities_.emplace_back(treeEntity);
	}

	// プレイヤーの初期化
	player_ = std::make_unique<Player>(registry_.get(), modelManager, instanceAllocator_.get(), footprintManager_.get());
	player_->Initialize();

	// メインカメラの作成
	mainCamera_ = std::make_unique<DebugCamera>(registry_.get(), sceneManager_->GetInput());
	mainCamera_->Initialize(cameraEntities_[mainCameraType_]);

	// 平行光源の設定
	auto directionalLight = registry_->GetComponent<DirectionalLight>(directionalLightEntity_);
	directionalLight->direction = { 0.5f, -1.0f, 0.2f };
}

void SampleScene::OnUpdate(float deltaTime) {
	Input *input = sceneManager_->GetInput();
#ifdef USE_IMGUI
	if (ImGui::TreeNode("TreeGenerator")) {
		ImGui::DragFloat3("RootPosition", &rootPosition.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("RootDirection", &rootDirection.x, 0.01f, -1.0f, 1.0f);
		ImGui::DragFloat3("CrownCenter", &crownCenter.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("CrownRadius", &crownRadius.x, 0.01f, 1.0f, 10.0f);
		ImGui::DragInt("LeafCount", reinterpret_cast<int *>(&leafCount), 1, 100, 5000);
		ImGui::DragFloat("MinRadius", &minRadius, 0.01f, 0.01f, 1.0f);
		ImGui::DragFloat("Gamma", &gamma, 0.01f, 1.0f, 5.0f);
		ImGui::DragFloat("InfluenceRadius", &influenceRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("KillRadius", &killRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("BranchLength", &branchLength, 0.01f, 0.1f, 1.0f);
		rootDirection = rootDirection.normalized();
		MeshManager *meshManager = sceneManager_->GetMeshManager();
		TextureManager *textureManager = sceneManager_->GetTextureManager();
		PrimitiveGenerator primitiveGenerator{ meshManager, textureManager };
		TreeGenerator treeGenerator{ registry_.get(), &primitiveGenerator, instanceAllocator_.get() };
		if (ImGui::Button("Generate")) {
			uint32_t treeEntity = treeGenerator.Generate(rootPosition, rootDirection, crownCenter, crownRadius, leafCount, minRadius, gamma, influenceRadius, killRadius, branchLength);
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
			ModelManager *modelManager = sceneManager_->GetModelManager();
			AnimatedCube animatedCube{ registry_.get(), modelManager, instanceAllocator_.get() };
			animatedCube.Create(animatedCubePosition);
		}
		ImGui::TreePop();
	}

	// シンプルスキンの作成
	if (ImGui::TreeNode("SimpleSkin")) {
		ImGui::DragFloat3("Position", &simpleSkinPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			ModelManager *modelManager = sceneManager_->GetModelManager();
			SimpleSkin simpleSkin{ registry_.get(), modelManager, instanceAllocator_.get() };
			simpleSkin.Create(simpleSkinPosition);
		}
		ImGui::TreePop();
	}

	// 歩く人間の作成
	if (ImGui::TreeNode("WalkHuman")) {
		ImGui::DragFloat3("Position", &walkHumanPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			ModelManager *modelManager = sceneManager_->GetModelManager();
			Human human{ registry_.get(), modelManager, instanceAllocator_.get() };
			human.Create("walk.gltf", walkHumanPosition);
		}
		ImGui::TreePop();
	}

	// スニークで歩く人間の作成
	if (ImGui::TreeNode("SneakWalkHuman")) {
		ImGui::DragFloat3("Position", &sneakWalkHumanPosition.x, 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		if (ImGui::Button("Generate")) {
			ModelManager *modelManager = sceneManager_->GetModelManager();
			Human human{ registry_.get(), modelManager, instanceAllocator_.get() };
			human.Create("sneakWalk.gltf", sneakWalkHumanPosition);
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI

	// メインカメラの更新
	if (!isDebugCameraActive_) {
		mainCamera_->Update();
	}

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
	player_->Update();
}