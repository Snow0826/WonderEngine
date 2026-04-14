#include "GameScene.h"
#include "SceneManager.h"
#include "SampleScene.h"
#include "Audio.h"
#include "Input.h"
#include "World.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "BlendMode.h"
#include "Texture.h"
#include "Object.h"
#include "Sprite.h"
#include "Particle.h"
#include "Model.h"
#include "Material.h"
#include "AABB.h"
#include "Sphere.h"
#include "Plane.h"
#include "Skydome.h"
#include "Ground.h"
#include "Wall.h"
#include "Building.h"
#include "AngelStatue.h"
#include "Player.h"
#include "Enemy.h"
#include "CameraController.h"
#include "DirectionalLight.h"
#include "GameScenePhasePlayFadeIn.h"
#include "GameScenePhasePlayFadeOut.h"
#include "BitmapFont.h"
#include "Fade.h"
#include "Random.h"

GameScene::GameScene() = default;
GameScene::~GameScene() {
	Audio *audio = sceneManager_->GetAudio();
	SoundHandle soundHandle = sceneManager_->GetSoundHandle();
	if (audio->IsPlaying(soundHandle.bgmHandleMap_[BGMType::Game])) {
		audio->Stop(soundHandle.bgmHandleMap_[BGMType::Game]);
	}
	delete phase_;
	phase_ = nullptr;
}

void GameScene::OnInitialize() {
	Input *input = sceneManager_->GetInput();
	Audio *audio = sceneManager_->GetAudio();
	SoundHandle soundHandle = sceneManager_->GetSoundHandle();
	TextureManager *textureManager = sceneManager_->GetTextureManager();
	ModelManager *modelManager = sceneManager_->GetModelManager();
	ParticleManager *particleManager = sceneManager_->GetParticleManager();

	// ビットマップフォントのテクスチャを取得
	for (size_t i = 0; i < 10; i++) {
		bitmapFont_->AddTextureHandle(textureManager->GetTextureReadHandle(std::to_string(i) + ".png"));
	}

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

	// プレイヤーの初期化
	player_ = std::make_unique<Player>(registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get(), footprintManager_.get(), particleManager, input, audio);
	player_->Initialize({ 0.0f, 0.0f, 0.0f });
	player_->SetCameraEntity(cameraEntities_[mainCameraType_]);
	player_->SetSoundHandle(soundHandle);

	// カメラコントローラーの初期化
	cameraController_ = std::make_unique<CameraController>(registry_.get(), input);
	cameraController_->Initialize(cameraEntities_[mainCameraType_], player_->GetPartsEntity(Player::PartsType::kBody));

	// ルールUIフェードの初期化
	ruleUIFade_ = std::make_unique<Fade>(registry_.get(), spriteManager_.get(), objectManager_.get());
	ruleUIFade_->Add("rule.png", Vector4{ 1.0f, 1.0f, 1.0f, 1.0f });

	// ビットマップフォントの初期化
	bitmapFont_->Initialize();

	// UIエンティティの生成
	uiEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(uiEntity_, BlendMode::kBlendModeNormal);
	registry_->AddComponent(uiEntity_, objectManager_->CreateObject(uiEntity_));
	registry_->AddComponent(uiEntity_, spriteManager_->CreateSprite("UI.png"));
	registry_->AddComponent(uiEntity_, Transform{});
	registry_->AddComponent(uiEntity_, Material{ .enableLighting = false });

	// Xスプライトの初期化
	Sprite sprite = spriteManager_->CreateSprite("T_X_X_Color_3D.png");
	sprite.spriteData.position = { 1080.0f, 620.0f };
	sprite.spriteData.size = { 48.0f, 48.0f };

	// Xエンティティの生成
	xEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(xEntity_, BlendMode::kBlendModeNormal);
	registry_->AddComponent(xEntity_, objectManager_->CreateObject(xEntity_));
	registry_->AddComponent(xEntity_, sprite);
	registry_->AddComponent(xEntity_, Transform{});
	registry_->AddComponent(xEntity_, Material{ .enableLighting = false });

	// Aスプライトの初期化
	sprite = spriteManager_->CreateSprite("T_X_A_Color_3D.png");
	sprite.spriteData.position = { 1080.0f, 670.0f };
	sprite.spriteData.size = { 48.0f, 48.0f };

	// Aエンティティの生成
	aEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(aEntity_, BlendMode::kBlendModeNormal);
	registry_->AddComponent(aEntity_, objectManager_->CreateObject(aEntity_));
	registry_->AddComponent(aEntity_, sprite);
	registry_->AddComponent(aEntity_, Transform{});
	registry_->AddComponent(aEntity_, Material{ .enableLighting = false });

	// RBスプライトの初期化
	sprite = spriteManager_->CreateSprite("T_X_RB_3D.png");
	sprite.spriteData.position = { 1080.0f, 570.0f };
	sprite.spriteData.size = { 48.0f, 48.0f };

	// RBエンティティの生成
	rbEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(rbEntity_, BlendMode::kBlendModeNormal);
	registry_->AddComponent(rbEntity_, objectManager_->CreateObject(rbEntity_));
	registry_->AddComponent(rbEntity_, sprite);
	registry_->AddComponent(rbEntity_, Transform{});
	registry_->AddComponent(rbEntity_, Material{ .enableLighting = false });

	// 左壁の当たり判定の作成
	uint32_t wallEntity = registry_->GenerateEntity();
	registry_->AddComponent(wallEntity, Collision::Plane{ .normal = {1.0f, 0.0f, 0.0f}, .distance = -256.0f });
	registry_->AddComponent(wallEntity, PlaneCollider{});
	registry_->AddComponent(wallEntity, PlaneRenderer{});

	// 右壁の当たり判定の作成
	wallEntity = registry_->GenerateEntity();
	registry_->AddComponent(wallEntity, Collision::Plane{ .normal = {-1.0f, 0.0f, 0.0f}, .distance = -256.0f });
	registry_->AddComponent(wallEntity, PlaneCollider{});
	registry_->AddComponent(wallEntity, PlaneRenderer{});

	// 前壁の当たり判定の作成
	wallEntity = registry_->GenerateEntity();
	registry_->AddComponent(wallEntity, Collision::Plane{ .normal = {0.0f, 0.0f, 1.0f}, .distance = -256.0f });
	registry_->AddComponent(wallEntity, PlaneCollider{});
	registry_->AddComponent(wallEntity, PlaneRenderer{});

	// 後壁の当たり判定の作成
	wallEntity = registry_->GenerateEntity();
	registry_->AddComponent(wallEntity, Collision::Plane{ .normal = {0.0f, 0.0f, -1.0f}, .distance = -256.0f });
	registry_->AddComponent(wallEntity, PlaneCollider{});
	registry_->AddComponent(wallEntity, PlaneRenderer{});

	// 平行光源の設定
	DirectionalLight *directionalLight = registry_->GetComponent<DirectionalLight>(directionalLightEntity_);
	directionalLight->direction = Vector3{ 1.0f, -1.0f, 0.0f }.normalized();

	// フェーズの初期化
	ChangePhase(new GameScenePhasePlayFadeIn);

	// BGMの再生
	audio->Play(soundHandle.bgmHandleMap_[BGMType::Game], true);
}

void GameScene::OnUpdate() {
#ifdef USE_IMGUI
	if (ImGui::Button("ToSampleScene")) {
		sceneManager_->SetNextScene(new SampleScene);
	}
	ImGui::Text("PlayTimer: %d", playTimer_);
	ImGui::Text("EnemyCount: %zu", enemies_.size());
	ImGui::Text("IsWaitingToPop: %d", isWaitingToPop_);
	ImGui::Text("WaitToPopTimer: %d", waitToPopTimer_);
#endif // USE_IMGUI

	// プレイタイマーの更新
	bitmapFont_->ToSprite((kPlayDuration - playTimer_) / 60);

	// フェーズの更新
	phase_->Update();

	// 死亡した敵の削除
	enemies_.remove_if([](const std::unique_ptr<Enemy> &enemy) { return enemy->IsDead(); });

	// 敵の更新
	for (auto &enemy : enemies_) {
		enemy->Update();
	}
}

void GameScene::OnAfterTransform() {
	// プレイヤーのワールド変換後処理
	player_->AfterTransform();

	// 全ての衝突判定をチェック
	CheckAllCollisions();
}

void GameScene::CheckAllCollisions() const {
	Model *playerBodyModel = registry_->GetComponent<Model>(player_->GetPartsEntity(Player::PartsType::kBody));
	Model *playerLeftArmModel = registry_->GetComponent<Model>(player_->GetPartsEntity(Player::PartsType::kLeftArm));
	Model *playerRightArmModel = registry_->GetComponent<Model>(player_->GetPartsEntity(Player::PartsType::kRightArm));
	Collision::Sphere *playerAttackParticleSphere = registry_->GetComponent<Collision::Sphere>(player_->GetAttackParticleEntity());

#pragma region プレイヤーと地面の衝突判定
	if (playerBodyModel &&
		!registry_->HasComponent<Disabled>(player_->GetPartsEntity(Player::PartsType::kBody)) &&
		registry_->HasComponent<SphereCollider>(player_->GetPartsEntity(Player::PartsType::kBody))) {
		for (const MeshData &playerMesh : playerBodyModel->modelData.meshes) {
			registry_->ForEach<Collision::Plane, PlaneCollider>([&](uint32_t entity, Collision::Plane *plane, PlaneCollider *planeCollider) {
				if (IsCollision(playerMesh.worldCollisionData.sphere, *plane)) {
					player_->OnCollision(*plane);
				}
				}, exclude<Disabled>());
		}
	}
#pragma endregion

#pragma region プレイヤーと建物の衝突判定
	if (playerBodyModel &&
		!registry_->HasComponent<Disabled>(player_->GetPartsEntity(Player::PartsType::kBody)) &&
		registry_->HasComponent<SphereCollider>(player_->GetPartsEntity(Player::PartsType::kBody))) {
		for (const MeshData &playerMesh : playerBodyModel->modelData.meshes) {
			registry_->ForEach<Model, AABBCollider>([&](uint32_t entity, Model *model, AABBCollider *aabbCollider) {
				for (const MeshData &mesh : model->modelData.meshes) {
					if (IsCollision(playerMesh.worldCollisionData.sphere, mesh.worldCollisionData.aabb)) {
						player_->OnCollision(mesh.worldCollisionData.aabb);
					}
				}
				}, exclude<Disabled>());
		}
	}
#pragma endregion

#pragma region 敵と地面の衝突判定
	for (auto &enemy : enemies_) {
		Model *enemyModel = registry_->GetComponent<Model>(enemy->GetEnemyEntity());
		if (enemyModel &&
			!registry_->HasComponent<Disabled>(enemy->GetEnemyEntity()) &&
			registry_->HasComponent<SphereCollider>(enemy->GetEnemyEntity())) {
			for (const MeshData &enemyMesh : enemyModel->modelData.meshes) {
				registry_->ForEach<Collision::Plane, PlaneCollider>([&](uint32_t entity, Collision::Plane *plane, PlaneCollider *planeCollider) {
					if (IsCollision(enemyMesh.worldCollisionData.sphere, *plane)) {
						enemy->OnCollision(*plane);
					}
					}, exclude<Disabled>());
			}
		}
	}
#pragma endregion

#pragma region 敵と建物の衝突判定
	for (auto &enemy : enemies_) {
		Model *enemyModel = registry_->GetComponent<Model>(enemy->GetEnemyEntity());
		if (enemyModel &&
			!registry_->HasComponent<Disabled>(enemy->GetEnemyEntity()) &&
			registry_->HasComponent<SphereCollider>(enemy->GetEnemyEntity())) {
			for (const MeshData &enemyMesh : enemyModel->modelData.meshes) {
				registry_->ForEach<Model, AABBCollider>([&](uint32_t entity, Model *model, AABBCollider *aabbCollider) {
					for (const MeshData &mesh : model->modelData.meshes) {
						if (IsCollision(enemyMesh.worldCollisionData.sphere, mesh.worldCollisionData.aabb)) {
							enemy->OnCollision(mesh.worldCollisionData.aabb);
						}
					}
					}, exclude<Disabled>());
			}
		}
	}
#pragma endregion

#pragma region プレイヤーと敵の衝突判定
	if (playerBodyModel &&
		!registry_->HasComponent<Disabled>(player_->GetPartsEntity(Player::PartsType::kBody)) &&
		registry_->HasComponent<SphereCollider>(player_->GetPartsEntity(Player::PartsType::kBody))) {
		for (const MeshData &playerBodyMesh : playerBodyModel->modelData.meshes) {
			for (auto &enemy : enemies_) {
				Model *enemyModel = registry_->GetComponent<Model>(enemy->GetEnemyEntity());
				if (enemyModel &&
					!registry_->HasComponent<Disabled>(enemy->GetEnemyEntity()) &&
					registry_->HasComponent<SphereCollider>(enemy->GetEnemyEntity())) {
					for (const MeshData &enemyMesh : enemyModel->modelData.meshes) {
						if (IsCollision(playerBodyMesh.worldCollisionData.sphere, enemyMesh.worldCollisionData.sphere)) {
							player_->OnCollision();
						}
					}
				}
			}
		}
	}
#pragma endregion

#pragma region 敵とプレイヤーの左腕の攻撃の衝突判定
	if (playerLeftArmModel &&
		!registry_->HasComponent<Disabled>(player_->GetPartsEntity(Player::PartsType::kLeftArm)) &&
		registry_->HasComponent<SphereCollider>(player_->GetPartsEntity(Player::PartsType::kLeftArm))) {
		for (const MeshData &playerLeftArmMesh : playerLeftArmModel->modelData.meshes) {
			for (auto &enemy : enemies_) {
				Model *enemyModel = registry_->GetComponent<Model>(enemy->GetEnemyEntity());
				if (enemyModel &&
					!registry_->HasComponent<Disabled>(enemy->GetEnemyEntity()) &&
					registry_->HasComponent<SphereCollider>(enemy->GetEnemyEntity())) {
					for (const MeshData &enemyMesh : enemyModel->modelData.meshes) {
						if (IsCollision(enemyMesh.worldCollisionData.sphere, playerLeftArmMesh.worldCollisionData.sphere)) {
							enemy->OnCollision();
						}
					}
				}
			}
		}
	}
#pragma endregion

#pragma region 敵とプレイヤーの右腕の攻撃の衝突判定
	if (playerRightArmModel &&
		!registry_->HasComponent<Disabled>(player_->GetPartsEntity(Player::PartsType::kRightArm)) &&
		registry_->HasComponent<SphereCollider>(player_->GetPartsEntity(Player::PartsType::kRightArm))) {
		for (const MeshData &playerRightArmMesh : playerRightArmModel->modelData.meshes) {
			for (auto &enemy : enemies_) {
				Model *enemyModel = registry_->GetComponent<Model>(enemy->GetEnemyEntity());
				if (enemyModel &&
					!registry_->HasComponent<Disabled>(enemy->GetEnemyEntity()) &&
					registry_->HasComponent<SphereCollider>(enemy->GetEnemyEntity())) {
					for (const MeshData &enemyMesh : enemyModel->modelData.meshes) {
						if (IsCollision(enemyMesh.worldCollisionData.sphere, playerRightArmMesh.worldCollisionData.sphere)) {
							enemy->OnCollision();
						}
					}
				}
			}
		}
	}
#pragma endregion

#pragma region プレイヤーの攻撃パーティクルと敵の衝突判定
	if (playerAttackParticleSphere &&
		!registry_->HasComponent<Disabled>(player_->GetAttackParticleEntity()) &&
		registry_->HasComponent<SphereCollider>(player_->GetAttackParticleEntity())) {
		for (auto &enemy : enemies_) {
			Model *enemyModel = registry_->GetComponent<Model>(enemy->GetEnemyEntity());
			if (enemyModel &&
				!registry_->HasComponent<Disabled>(enemy->GetEnemyEntity()) &&
				registry_->HasComponent<SphereCollider>(enemy->GetEnemyEntity())) {
				for (const MeshData &enemyMesh : enemyModel->modelData.meshes) {
					if (IsCollision(enemyMesh.worldCollisionData.sphere, *playerAttackParticleSphere)) {
						enemy->OnCollision();
					}
				}
			}
		}
	}
#pragma endregion
}

void GameScene::ChangePhase(BaseGameScenePhase *newBaseGameScenePhase) {
	// 古いフェーズを削除
	if (phase_) {
		delete phase_;
		phase_ = nullptr;
	}

	// 新しいフェーズを設定
	phase_ = newBaseGameScenePhase;

	// 新しいフェーズの初期化
	phase_->SetGameScene(this);
	phase_->Initialize();
}

void GameScene::PopEnemy() {
	// 待機処理
	if (isWaitingToPop_) {
		waitToPopTimer_--;
		if (waitToPopTimer_ <= 0) {
			isWaitingToPop_ = false;	// 待機完了
		}
		return;
	}

	// 最大数に達していたら出現しない
	if (enemies_.size() >= kMaxEnemyCount) {
		return;
	}

	// モデルマネージャーの取得
	ModelManager *modelManager = sceneManager_->GetModelManager();

	// 敵の出現処理
	Vector3 worldPosition = transformSystem_->GetWorldPosition(player_->GetPartsEntity(Player::PartsType::kBody));
	std::unique_ptr<Enemy> enemy = std::make_unique<Enemy>(registry_.get(), indirectCommandManager_.get(), modelManager, objectManager_.get(), footprintManager_.get());
	Collision::AABB spawnArea = {
		.min = { -64.0f, 0.0f, -64.0f },
		.max = { 64.0f, 0.0f, 64.0f }
	};
	Vector3 spawnPosition = Random::generate(spawnArea.min, spawnArea.max);
	spawnPosition += worldPosition + worldPosition.normalized(spawnPosition) * 100.0f;
	spawnPosition.x = Wrap(spawnPosition.x, -256.0f, 256.0f);
	spawnPosition.z = Wrap(spawnPosition.z, -256.0f, 256.0f);
	registry_->ForEach<Model, AABBCollider>([&](uint32_t entity, Model *model, AABBCollider *aabbCollider) {
		for (const MeshData &mesh : model->modelData.meshes) {
			if (IsCollision(mesh.worldCollisionData.aabb, spawnPosition)) {
				spawnPosition.y = mesh.worldCollisionData.aabb.max.y;
			}
		}
		}, exclude<Disabled>());
	enemy->Initialize(spawnPosition);
	enemy->SetPlayerEntity(player_->GetPartsEntity(Player::PartsType::kBody));
	enemies_.emplace_back(std::move(enemy));

	// 次の出現まで待機
	isWaitingToPop_ = true;
	float t = static_cast<float>(playTimer_) / static_cast<float>(kPlayDuration);
	waitToPopTimer_ = static_cast<int32_t>(static_cast<float>(kMaxWaitToPopTime) * (1.0f - t));
}

void GameScene::DeadEnemy() {
	for (auto &enemy : enemies_) {
		enemy->Dead();
	}
}

void GameScene::ChangeToGameClearPhase() {
	if (++playTimer_ >= kPlayDuration) {
		sceneManager_->GetWorld()->SetResult(true);
		player_->Dead();
		DeadEnemy();
		ChangePhase(new GameScenePhasePlayFadeOut);
	}
}

void GameScene::ClearUI() {
	registry_->RemoveAllComponents(uiEntity_);
	registry_->RemoveAllComponents(xEntity_);
	registry_->RemoveAllComponents(aEntity_);
	registry_->RemoveAllComponents(rbEntity_);
}