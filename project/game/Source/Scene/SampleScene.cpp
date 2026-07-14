#define NOMINMAX
#include "SampleScene.h"
#include "SceneManager.h"
#include "Skybox.h"
#include "SkyboxEntity.h"
#include "AnimatedCube.h"
#include "SimpleSkin.h"
#include "Human.h"
#include "ParticleObject.h"
#include "Primitive.h"
#include "TreeGenerator.h"
#include "DebugCamera.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	Vector3 crownCenter{ 0.0f, 0.0f, 0.0f };
	Vector3 crownRadius{ 5.0f, 5.0f, 5.0f };
	uint32_t leafCount = 5000;
	float minRadius = 0.01f;
	float gamma = 2.0f;
	float influenceRadius = std::numeric_limits<float>::max();
	float killRadius = 1.6f;
	float branchLength = 0.3f;
}

SampleScene::SampleScene() = default;
SampleScene::~SampleScene() = default;

void SampleScene::OnInitialize() {
	// マネージャーの取得
	MeshManager *meshManager = sceneManager_->GetMeshManager();
	TextureManager *textureManager = sceneManager_->GetTextureManager();
	ModelManager *modelManager = sceneManager_->GetModelManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();

	// ジェネレーターの初期化
	SkyboxGenerator skyboxGenerator{ meshManager, textureManager };

	// スカイボックスエンティティの作成
	SkyboxEntity::Create(registry_.get(), &skyboxGenerator);

	// アニメーションするキューブの作成
	AnimatedCube animatedCube{ registry_.get(), modelManager };
	animatedCube.Create({ 0.0f, 0.0f, 20.0f });

	// シンプルスキンの作成
	SimpleSkin simpleSkin{ registry_.get(), modelManager };
	simpleSkin.Create({ -3.0f, 0.0f, 5.0f });

	// ヒューマンの作成
	Human human{ registry_.get(), modelManager };
	human.Create("walk.gltf", { 0.0f, 0.0f, 5.0f });
	human.Create("sneakWalk.gltf", { 3.0f, 0.0f, 5.0f });

	// パーティクルオブジェクトの作成
	//ParticleObject particleObject{ registry_.get(), particleManager };
	//particleObject.Create();

	// メインカメラの作成
	mainCamera_ = std::make_unique<DebugCamera>(registry_.get(), sceneManager_->GetInput());
	mainCamera_->Initialize(cameraEntities_[mainCameraType_]);
}

void SampleScene::OnUpdate() {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("TreeGenerator")) {
		ImGui::DragFloat3("CrownCenter", &crownCenter.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("CrownRadius", &crownRadius.x, 0.01f, 1.0f, 10.0f);
		ImGui::DragInt("LeafCount", reinterpret_cast<int *>(&leafCount), 1, 100, 5000);
		ImGui::DragFloat("MinRadius", &minRadius, 0.01f, 0.01f, 1.0f);
		ImGui::DragFloat("Gamma", &gamma, 0.01f, 1.0f, 5.0f);
		ImGui::DragFloat("InfluenceRadius", &influenceRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("KillRadius", &killRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("BranchLength", &branchLength, 0.01f, 0.1f, 1.0f);
		MeshManager *meshManager = sceneManager_->GetMeshManager();
		TextureManager *textureManager = sceneManager_->GetTextureManager();
		PrimitiveGenerator primitiveGenerator{ meshManager, textureManager };
		TreeGenerator treeGenerator{ registry_.get(), &primitiveGenerator };
		if (ImGui::Button("Generate")) {
			uint32_t treeEntity = treeGenerator.Generate(crownCenter, crownRadius, leafCount, minRadius, gamma, influenceRadius, killRadius, branchLength);
			treeEntities_.emplace_back(treeEntity);
		}

		if (ImGui::Button("Delete") && !treeEntities_.empty()) {
			treeGenerator.Delete(treeEntities_.back());
			treeEntities_.pop_back();
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI

	// メインカメラの更新
	if (!isDebugCameraActive_) {
		mainCamera_->Update();
	}
}