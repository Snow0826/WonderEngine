#define NOMINMAX
#include "SampleScene.h"
#include "SceneManager.h"
#include "Skybox.h"
#include "SkyboxEntity.h"
#include "Cylinder.h"
#include "TreeGenerator.h"
#include "HitEffect.h"
#include "SlashEffect.h"
#include "HitRingEffect.h"
#include "SlashRingEffect.h"
#include "MagicCircleEffect.h"
#include "LeafEffect.h"
#include "AnimatedCube.h"
#include "SimpleSkin.h"
#include "Human.h"
#include "Quaternion.h"
#include "Random.h"
#include "Particle.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Object.h"
#include "DebugCamera.h"
#include <numbers>

#ifdef USE_IMGUI
#include <imgui.h>
#endif // USE_IMGUI

namespace {
	uint32_t count = 10;
	Vector3 start;
	Range<Vector3> positionRange{ .min = { -100.0f, 0.0f, -100.0f }, .max = { 100.0f, 0.0f, 100.0f } };
	Vector3 direction{ 0.0f, 1.0f, 0.0f };
	float length = 5.0f;
	int32_t depth = 5;
	uint32_t divide = 32;
	float topRadius = 0.1f;
	float bottomRadius = 0.2f;
}

SampleScene::SampleScene() = default;
SampleScene::~SampleScene() = default;

void SampleScene::OnInitialize() {
	// マネージャーの取得
	MeshManager *meshManager = sceneManager_->GetMeshManager();
	TextureManager *textureManager = sceneManager_->GetTextureManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();
	ModelManager *modelManager = sceneManager_->GetModelManager();
	CylinderGenerator cylinderGenerator{ meshManager, textureManager };

	// ジェネレーターの初期化
	SkyboxGenerator skyboxGenerator{ meshManager, textureManager };

	// スカイボックスエンティティの作成
	SkyboxEntity::Create(registry_.get(), &skyboxGenerator, objectManager_.get());

	// ヒットエフェクトの初期化
	hitEffect_ = std::make_unique<HitEffect>(registry_.get(), particleManager);
	hitEffect_->Initialize();

	// スラッシュエフェクトの初期化
	slashEffect_ = std::make_unique<SlashEffect>(registry_.get(), particleManager);
	slashEffect_->Initialize();

	// ヒットリングエフェクトの初期化
	hitRingEffect_ = std::make_unique<HitRingEffect>(registry_.get(), particleManager);
	hitRingEffect_->Initialize();

	// スラッシュリングエフェクトの初期化
	slashRingEffect_ = std::make_unique<SlashRingEffect>(registry_.get(), particleManager);
	slashRingEffect_->Initialize();

	// メインカメラの作成
	mainCamera_ = std::make_unique<DebugCamera>(registry_.get(), sceneManager_->GetInput());
	mainCamera_->Initialize(cameraEntities_[mainCameraType_]);

	// アニメーションキューブの作成
	AnimatedCube animatedCube{ registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get() };
	animatedCube.Create();

	// シンプルスキンの作成
	SimpleSkin simpleSkin{ registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get() };
	simpleSkin.Create();

	// ヒューマンの作成
	Human human{ registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get() };
	human.Create("walk.gltf", { 0.0f, 0.0f, 5.0f });
	human.Create("sneakWalk.gltf", { 3.0f, 0.0f, 5.0f });

	// 魔法陣エフェクトの初期化
	magicCircleEffect_ = std::make_unique<MagicCircleEffect>(registry_.get(), indirectCommandManager_.get(), &cylinderGenerator, objectManager_.get());
	magicCircleEffect_->Initialize();
}

void SampleScene::OnUpdate() {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("TreeGenerator")) {
		int32_t divideInt = divide;
		int32_t countInt = count;
		ImGui::DragInt("Divide", &divideInt, 1, 3, 500);
		ImGui::DragInt("Count", &countInt, 1, 1, 100);
		ImGui::DragFloat("TopRadius", &topRadius, 0.1f, 0.0f, std::numeric_limits<float>::max());
		ImGui::DragFloat("BottomRadius", &bottomRadius, 0.1f, 0.0f, std::numeric_limits<float>::max());
		ImGui::DragFloat("Length", &length, 0.1f, 0.0f, std::numeric_limits<float>::max());
		ImGui::DragFloat3("Start", &start.x, 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragFloat3("Direction", &direction.x, 0.1f, 0.0f, 1.0f);
		ImGui::DragFloat3("PositionRangeMin", &positionRange.min.x, 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragFloat3("PositionRangeMax", &positionRange.max.x, 0.1f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
		ImGui::DragInt("Depth", &depth, 1, 0, 10);
		direction = direction.normalized();
		divide = static_cast<uint32_t>(divideInt);
		count = static_cast<uint32_t>(countInt);
		MeshManager *meshManager = sceneManager_->GetMeshManager();
		TextureManager *textureManager = sceneManager_->GetTextureManager();
		CylinderGenerator cylinderGenerator{ meshManager, textureManager };
		TreeGenerator treeGenerator{ registry_.get(), &cylinderGenerator, objectManager_.get(), indirectCommandManager_.get() };
		if (ImGui::Button("Generate")) {
			for (size_t i = 0; i < count; i++) {
				uint32_t treeEntity = treeGenerator.Generate(5.0f, 1000, 2.5f, 0.6f, 0.3f);
				treeEntities_.emplace_back(treeEntity);
			}
		}

		if (ImGui::Button("Delete") && !treeEntities_.empty()) {
			treeGenerator.Delete(treeEntities_.back());
			treeEntities_.pop_back();
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI

	// ヒットエフェクトの更新
	hitEffect_->Update();

	// スラッシュエフェクトの更新
	slashEffect_->Update();

	// ヒットリングエフェクトの更新
	hitRingEffect_->Update();

	// スラッシュリングエフェクトの更新
	slashRingEffect_->Update();
	
	// 魔法陣エフェクトの更新
	magicCircleEffect_->Update();
	
	// メインカメラの更新
	mainCamera_->Update();
}