#define NOMINMAX
#include "SampleScene.h"
#include "SceneManager.h"
#include "Skybox.h"
#include "SkyboxEntity.h"
#include "Cylinder.h"
#include "TreeGenerator.h"
#include "HitEffectParticle.h"
#include "SlashEffectParticle.h"
#include "AnimatedCube.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	Vector3 start;
	Vector3 direction{ 0.0f, 1.0f, 0.0f };
	float length = 5.0f;
	int32_t depth = 5;
	uint32_t divide = 32;
	float topRadius = 0.1f;
	float bottomRadius = 0.1f;
}

SampleScene::SampleScene() = default;
SampleScene::~SampleScene() = default;

void SampleScene::OnInitialize() {
	// マネージャーの取得
	MeshManager *meshManager = sceneManager_->GetMeshManager();
	TextureManager *textureManager = sceneManager_->GetTextureManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();
	ModelManager *modelManager = sceneManager_->GetModelManager();

	// ジェネレーターの初期化
	SkyboxGenerator skyboxGenerator{ meshManager, textureManager };

	// スカイボックスエンティティの作成
	SkyboxEntity::Create(registry_.get(), &skyboxGenerator, objectManager_.get());

	// ヒットエフェクトのパーティクルの初期化
	hitEffectParticle_ = std::make_unique<HitEffectParticle>(registry_.get(), particleManager);
	hitEffectParticle_->Initialize();

	// スラッシュエフェクトのパーティクルの初期化
	slashEffectParticle_ = std::make_unique<SlashEffectParticle>(registry_.get(), particleManager);
	slashEffectParticle_->Initialize();

	// アニメーションキューブの作成
	AnimatedCube animatedCube{ registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get() };
	animatedCube.Create();
}

void SampleScene::OnUpdate() {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("TreeGenerator")) {
		int32_t divideInt = divide;
		ImGui::DragInt("Divide", &divideInt, 1, 3, 500);
		ImGui::DragFloat("TopRadius", &topRadius, 0.1f, 0.0f, std::numeric_limits<float>::max());
		ImGui::DragFloat("BottomRadius", &bottomRadius, 0.1f, 0.0f, std::numeric_limits<float>::max());
		ImGui::DragFloat("Length", &length, 0.1f, 0.0f, std::numeric_limits<float>::max());
		ImGui::DragFloat3("Start", &start.x, 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragFloat3("Direction", &direction.x, 0.1f, 0.0f, 1.0f);
		ImGui::DragInt("Depth", &depth, 1, 0, 10);
		direction = direction.normalized();
		divide = static_cast<uint32_t>(divideInt);
		if (ImGui::Button("Generate")) {
			MeshManager *meshManager = sceneManager_->GetMeshManager();
			TextureManager *textureManager = sceneManager_->GetTextureManager();
			CylinderGenerator cylinderGenerator{ meshManager, textureManager };
			TreeGenerator treeGenerator{ registry_.get(), &cylinderGenerator, objectManager_.get(), indirectCommandManager_.get() };
			treeGenerator.Generate(start, direction, divide, topRadius, bottomRadius, length, depth);
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI

	// ヒットエフェクトのパーティクルの更新
	hitEffectParticle_->Update();

	// スラッシュエフェクトのパーティクルの更新
	slashEffectParticle_->Update();
}