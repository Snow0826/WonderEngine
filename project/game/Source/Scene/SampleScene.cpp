#define NOMINMAX
#include "SampleScene.h"
#include "SceneManager.h"
#include "Skybox.h"
#include "SkyboxEntity.h"
#include "Cylinder.h"
#include "CylinderEntity.h"
#include "HitEffectParticle.h"
#include "SlashEffectParticle.h"
#include "AnimatedCube.h"
#include "DebugRenderer.h"
#include "Random.h"

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	Vector3 start;
	Vector3 direction{ 0.0f, 1.0f, 0.0f };
	float length = 5.0f;
	int32_t depth = 5;
	uint32_t divide = 32;
	float topRadius = 1.0f;
	float bottomRadius = 1.0f;
	float height = 3.0f;
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
	if (ImGui::TreeNode("Branch")) {
		ImGui::DragFloat3("Start", &start.x, 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragFloat3("Direction", &direction.x, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat("Length", &length, 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragInt("Depth", &depth, 1, 0, 10);
		direction = direction.normalized();
		if (ImGui::Button("Generate")) {
			GenerateBranch(start, direction, length, depth);
		}
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Cylinder")) {
		int32_t divideInt = divide;
		ImGui::DragInt("Divide", &divideInt, 1, 3, 500);
		ImGui::DragFloat("TopRadius", &topRadius, 0.1f, 0.0f, std::numeric_limits<float>::max());
		ImGui::DragFloat("BottomRadius", &bottomRadius, 0.1f, 0.0f, std::numeric_limits<float>::max());
		ImGui::DragFloat("Height", &height, 0.1f, 0.0f, std::numeric_limits<float>::max());
		divide = static_cast<uint32_t>(divideInt);
		if (ImGui::Button("Generate")) {
			MeshManager *meshManager = sceneManager_->GetMeshManager();
			TextureManager *textureManager = sceneManager_->GetTextureManager();
			CylinderGenerator cylinderGenerator{ meshManager, textureManager };
			CylinderEntity cylinderEntity{ registry_.get(), &cylinderGenerator, objectManager_.get(), indirectCommandManager_.get() };
			cylinderEntity.Create(divide, topRadius, bottomRadius, height);
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI

	// ヒットエフェクトのパーティクルの更新
	hitEffectParticle_->Update();

	// スラッシュエフェクトのパーティクルの更新
	slashEffectParticle_->Update();

	for (const Branch &branch : branches_) {
		debugRenderer_->AddLine({ .start = branch.start, .end = branch.end, .color = { 0.0f, 1.0f, 0.0f, 1.0f } });
	}
}

void SampleScene::GenerateBranch(const Vector3 &start, const Vector3 &direction, float length, int32_t depth) {
	if (depth <= 0) {
		return;
	}

	Vector3 end = start + direction * length;
	branches_.emplace_back(Branch{ start, end });

	for (size_t i = 0; i < 2; i++) {
		Vector3 newDirection = direction + Random::generate({ -1.0f, 0.0f, -1.0f }, { 1.0f, 1.0f, 1.0f }).normalized();
		GenerateBranch(end, newDirection.normalized(), length * 0.7f, depth - 1);
	}
}
