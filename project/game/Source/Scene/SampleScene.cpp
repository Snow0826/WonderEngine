#include "SampleScene.h"
#include "SceneManager.h"
#include "Skydome.h"
#include "Ground.h"
#include "Wall.h"
#include "Building.h"
#include "AngelStatue.h"
#include "Vector3.h"
#include <numbers>

SampleScene::SampleScene() = default;
SampleScene::~SampleScene() = default;

void SampleScene::OnInitialize() {
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
}

void SampleScene::OnUpdate() {}