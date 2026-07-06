#define NOMINMAX
#include "SampleScene.h"
#include "SceneManager.h"
#include "Skybox.h"
#include "SkyboxEntity.h"
#include "AnimatedCube.h"
#include "Cylinder.h"
#include "TreeGenerator.h"
#include "DebugCamera.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	float leafRadius = 5.0f;
	uint32_t leafCount = 5000;
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

	// ジェネレーターの初期化
	SkyboxGenerator skyboxGenerator{ meshManager, textureManager };

	// スカイボックスエンティティの作成
	SkyboxEntity::Create(registry_.get(), &skyboxGenerator, objectManager_.get());

	// アニメーションするキューブの作成
	animatedCube_ = std::make_unique<AnimatedCube>(registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get());
	animatedCube_->Create();

	// メインカメラの作成
	mainCamera_ = std::make_unique<DebugCamera>(registry_.get(), sceneManager_->GetInput());
	mainCamera_->Initialize(cameraEntities_[mainCameraType_]);
}

void SampleScene::OnUpdate() {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("TreeGenerator")) {
		ImGui::DragFloat("LeafRadius", &leafRadius, 0.01f, 1.0f, 10.0f);
		ImGui::DragInt("LeafCount", reinterpret_cast<int *>(&leafCount), 1, 100, 5000);
		ImGui::DragFloat("InfluenceRadius", &influenceRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("KillRadius", &killRadius, 0.01f, 0.1f, 5.0f);
		ImGui::DragFloat("BranchLength", &branchLength, 0.01f, 0.1f, 1.0f);
		MeshManager *meshManager = sceneManager_->GetMeshManager();
		TextureManager *textureManager = sceneManager_->GetTextureManager();
		CylinderGenerator cylinderGenerator{ meshManager, textureManager };
		TreeGenerator treeGenerator{ registry_.get(), &cylinderGenerator, objectManager_.get(), indirectCommandManager_.get() };
		if (ImGui::Button("Generate")) {
			uint32_t treeEntity = treeGenerator.Generate(leafRadius, leafCount, influenceRadius, killRadius, branchLength);
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