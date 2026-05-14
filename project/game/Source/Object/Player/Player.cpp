#include "Player.h"
#include "EntityComponentSystem.h"
#include "IndirectCommand.h"
#include "Input.h"
#include "Audio.h"
#include "BlendMode.h"
#include "Object.h"
#include "Model.h"
#include "Material.h"
#include "RigidBody.h"
#include "Footprint.h"
#include "AABB.h"
#include "Sphere.h"
#include "Particle.h"
#include "ArmParticle.h"
#include "AttackParticle.h"
#include "PlayerBehaviorRoot.h"
#include "Easing.h"

Player::Player(Registry *registry, IndirectCommandManager *indirectCommandManager, ModelManager *modelManager, ObjectManager *objectManager, FootprintManager *footprintManager, ParticleManager *particleManager, Input *input, Audio *audio)
	: registry_(registry)
	, indirectCommandManager_(indirectCommandManager)
	, modelManager_(modelManager)
	, objectManager_(objectManager)
	, footprintManager_(footprintManager)
	, particleManager_(particleManager)
	, input_(input)
	, audio_(audio) {}

Player::~Player() = default;

void Player::Initialize(const Vector3 &position) {
	// プレイヤーの部位名リスト
	std::vector<std::string> partsNames = {
		"body",
		"head",
		"leftArm",
		"rightArm"
	};

	// プレイヤーの部位の追加
	for (const std::string &partsName : partsNames) {
		AddParts(partsName);
	}

	uint32_t bodyEntity = entities_[static_cast<size_t>(PartsType::kBody)];

	// プレイヤーの部位の親子関係の設定
	for (size_t i = 0; i < entities_.size(); i++) {
		if (i == static_cast<size_t>(PartsType::kBody)) {
			continue;
		}

		Transform *childTransform = registry_->GetComponent<Transform>(entities_[i]);
		if (childTransform) {
			registry_->AddComponent(entities_[i], HasParent{});
			childTransform->parentEntity = bodyEntity;
		}
	}

	// プレイヤーの初期位置の設定
	Transform *bodyTransform = registry_->GetComponent<Transform>(bodyEntity);
	if (bodyTransform) {
		bodyTransform->translate = position;
	}

	// プレイヤーの剛体の追加
	registry_->AddComponent(bodyEntity, RigidBody{});

	// プレイヤーの体モデルの取得
	Model *bodyModel = registry_->GetComponent<Model>(bodyEntity);

	// プレイヤーのフットプリントの追加
	footprintEntity_ = registry_->GenerateEntity();
	registry_->AddComponent(footprintEntity_, footprintManager_->CreateFootprint(footprintEntity_, { 1.0f, 0.0f, 0.0f, 1.0f }));
	registry_->AddComponent(footprintEntity_, Collision::Sphere{});
	registry_->AddComponent(footprintEntity_, SphereRenderer{});

	// 左腕パーティクルの初期化
	leftArmParticle_ = std::make_unique<ArmParticle>(registry_, particleManager_);
	leftArmParticle_->Initialize(entities_[static_cast<size_t>(PartsType::kLeftArm)], "leftArm");

	// 右腕パーティクルの初期化
	rightArmParticle_ = std::make_unique<ArmParticle>(registry_, particleManager_);
	rightArmParticle_->Initialize(entities_[static_cast<size_t>(PartsType::kRightArm)], "rightArm");

	// 攻撃パーティクルの初期化
	attackParticle_ = std::make_unique<AttackParticle>(registry_, particleManager_, footprintManager_);
	attackParticle_->Initialize(bodyEntity);

	// 現在の振る舞いの初期化
	currentBehavior_ = std::make_unique<PlayerBehaviorRoot>();
	currentBehavior_->SetPlayer(this);
	currentBehavior_->Initialize();

	// 状態の初期化
	jumpCounter_ = 0;
	isPressMove_ = false;
}

void Player::Update() {
	// ジャンプ中は重力を強化する
	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (rigidBody) {
		if (jumpCounter_) {
			rigidBody->force.y = -19.62f;
		}
	}

	// 現在の振る舞いの更新
	if (currentBehavior_) {
		currentBehavior_->Update();
	}

	// 振る舞いの変更
	if (nextBehavior_) {
		currentBehavior_ = std::move(nextBehavior_);
		nextBehavior_ = nullptr;
		currentBehavior_->SetPlayer(this);
		currentBehavior_->Initialize();
	}

	// 左腕パーティクルの更新
	leftArmParticle_->Update();

	// 右腕パーティクルの更新
	rightArmParticle_->Update();

#ifdef USE_IMGUI
	ImGui::Text("PlayerEntity: %u", entities_[static_cast<size_t>(PartsType::kBody)]);
#endif // USE_IMGUI
}

void Player::AfterTransform() {
	Model *model = registry_->GetComponent<Model>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (!model) {
		return;
	}
	Collision::Sphere bodySphere = model->modelData.meshes.back().sphere;
	bodySphere.center.y -= kBodyAmplitude * 2.0f;
	registry_->AddComponent(footprintEntity_, bodySphere);
}

void Player::OnCollision(const Collision::Plane &plane) {
	// モデルの取得
	Model *model = registry_->GetComponent<Model>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (!model) {
		return;
	}

	// 貫入量を計算
	float penetration = PenetrationDepth(model->modelData.meshes.back().sphere, plane);

	// 貫入量が0以下なら衝突していないので処理を抜ける
	if (penetration <= 0.0f) {
		return;
	}

	// 剛体の取得
	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (rigidBody) {
		rigidBody->velocity.y = 0.0f;
	}

	// SRTデータの取得
	Transform *transform = registry_->GetComponent<Transform>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (!transform) {
		return;
	}

	// 貫入量分だけ位置を修正
	transform->translate += plane.normal * penetration;

	// 法線ベクトルのY成分が0以下なら床ではないので処理を抜ける
	if (plane.normal.y <= 0.0f) {
		return;
	}

	basePositionY_ = transform->translate.y;

	// 着地音の再生
	if (jumpCounter_) {
		audio_->Play(soundHandle_.seHandleMap_[SEType::OnGround], false);
	}

	// ジャンプカウンターリセット
	jumpCounter_ = 0;
}

void Player::OnCollision(const Collision::AABB &aabb) {
	// モデルの取得
	Model *model = registry_->GetComponent<Model>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (!model) {
		return;
	}

	// 貫入量を計算
	float penetration = PenetrationDepth(model->modelData.meshes.back().sphere, aabb);

	// 貫入量が0以下なら衝突していないので処理を抜ける
	if (penetration <= 0.0f) {
		return;
	}

	// 剛体の取得
	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (rigidBody) {
		rigidBody->velocity.y = 0.0f;
	}

	// SRTデータの取得
	Transform *transform = registry_->GetComponent<Transform>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (!transform) {
		return;
	}

	// 法線ベクトルの計算
	Vector3 normal = Normal(model->modelData.meshes.back().sphere.center, aabb);

	// 貫入量分だけ位置を修正
	transform->translate += normal * penetration;

	// 法線ベクトルのY成分が0以下なら床ではないので処理を抜ける
	if (normal.y <= 0.0f) {
		return;
	}

	basePositionY_ = transform->translate.y;

	// 着地音の再生
	if (jumpCounter_) {
		audio_->Play(soundHandle_.seHandleMap_[SEType::OnGround], false);
	}

	// ジャンプカウンターリセット
	jumpCounter_ = 0;
}

void Player::OnCollision() {
	Dead();
}

void Player::Dead() {
	RemoveParts(PartsType::kBody);
	RemoveParts(PartsType::kHead);
	RemoveParts(PartsType::kLeftArm);
	RemoveParts(PartsType::kRightArm);
}

void Player::Revival() {
	// 各部位のコンポーネントを有効化
	ReviveParts(PartsType::kBody);
	ReviveParts(PartsType::kHead);
	ReviveParts(PartsType::kLeftArm);
	ReviveParts(PartsType::kRightArm);

	// 位置リセット
	Transform *transform = registry_->GetComponent<Transform>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (transform) {
		transform->translate = { 0.0f, 0.0f, 0.0f };
	}

	// ジャンプカウンターリセット
	jumpCounter_ = 0;
}

uint32_t Player::GetAttackParticleEntity() const {
	return attackParticle_->GetParticleEntity();
}

bool Player::IsDead() const {
	return registry_->HasComponent<Disabled>(entities_[static_cast<size_t>(PartsType::kBody)]);
}

void Player::AddParts(const std::string &partsName) {
	// エンティティの生成とコンポーネントの追加
	uint32_t entity = registry_->GenerateEntity();
	registry_->AddComponent(entity, BlendMode::kBlendModeNone);
	registry_->AddComponent(entity, Transform{});
	registry_->AddComponent(entity, Material{});
	registry_->AddComponent(entity, DirtyTransform{});
	registry_->AddComponent(entity, DirtyMaterial{});
	registry_->AddComponent(entity, objectManager_->CreateObject(entity));
	registry_->AddComponent(entity, modelManager_->FindModel(partsName + ".obj"));
	registry_->AddComponent(entity, UseCulling{});
	registry_->AddComponent(entity, SphereCollider{});
	registry_->AddComponent(entity, SphereRenderer{});
	registry_->AddComponent(entity, AABBRenderer{});
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));
	entities_.emplace_back(entity);
}

void Player::RemoveParts(PartsType partsType) {
	uint32_t entity = entities_[static_cast<size_t>(partsType)];
	registry_->AddComponent(entity, Disabled{});
	registry_->AddComponent(footprintEntity_, Disabled{});
	indirectCommandManager_->RemoveIndirectCommand(entity);
}

void Player::ReviveParts(PartsType partsType) {
	uint32_t entity = entities_[static_cast<size_t>(partsType)];
	registry_->RemoveComponent<Disabled>(entity);
	registry_->RemoveComponent<Disabled>(footprintEntity_);
	registry_->AddComponent(entity, indirectCommandManager_->AddIndirectCommand(entity));
}

void Player::Move() {
	Vector2 move = { 0.0f, 0.0f };

	// キーボード入力による移動
	if (input_->IsPressKey(DIK_W)) {
		move.y = 1.0f;
		isPressMove_ = true;
	}

	if (input_->IsPressKey(DIK_A)) {
		move.x = -1.0f;
		isPressMove_ = true;
	}

	if (input_->IsPressKey(DIK_S)) {
		move.y = -1.0f;
		isPressMove_ = true;
	}

	if (input_->IsPressKey(DIK_D)) {
		move.x = 1.0f;
		isPressMove_ = true;
	}

	// コントローラー入力による移動
	if (input_->Connected()) {
		// コントローラーの左スティックの値（正規化済み）
		XINPUT_STATE joyStickState = input_->GetJoyStickState();
		move.x = input_->NormalizeAxis(joyStickState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		move.y = input_->NormalizeAxis(joyStickState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
		if (move.x || move.y) {
			isPressMove_ = true;
		}
	} else {
#ifdef USE_IMGUI
		ImGui::Text("No Controller Connected");
#endif // USE_IMGUI
	}

	TransformSystem transformSystem{ registry_ };
	Vector3 right = transformSystem.GetRight(cameraEntity_);
	Vector3 forward = transformSystem.GetForward(cameraEntity_);

	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (!rigidBody) {
		return;
	}

	rigidBody->velocity.x = right.x * move.x + forward.x * move.y;
	rigidBody->velocity.z = right.z * move.x + forward.z * move.y;

	move = { rigidBody->velocity.x, rigidBody->velocity.z };
	move = move.normalized() * 20.0f;
	rigidBody->velocity.x = move.x;
	rigidBody->velocity.z = move.y;
}

void Player::Rotate() {
	Transform *transform = registry_->GetComponent<Transform>(entities_[static_cast<size_t>(PartsType::kBody)]);
	RigidBody *rigidBody = registry_->GetComponent<RigidBody>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (!transform || !rigidBody) {
		return;
	}

	if (isPressMove_) {
		Vector2 move = { rigidBody->velocity.x, rigidBody->velocity.z };
		move = move.normalized();
		bodyRotate_.y = std::atan2(-move.x, -move.y);
		isPressMove_ = false;
	}

	transform->rotate = LerpShortAngle(transform->rotate, bodyRotate_, 0.1f);
}

void Player::Float() {
	if (jumpCounter_) {
		return;
	}

	floatingParameter_ += kStep;
	floatingParameter_ = std::fmodf(floatingParameter_, std::numbers::pi_v<float> *2.0f);
	Transform *bodyTransform = registry_->GetComponent<Transform>(entities_[static_cast<size_t>(PartsType::kBody)]);
	if (bodyTransform) {
		bodyTransform->translate.y = std::sin(floatingParameter_) * kBodyAmplitude + basePositionY_ + kBodyAmplitude;
	}

	Transform *leftArmTransform = registry_->GetComponent<Transform>(entities_[static_cast<size_t>(PartsType::kLeftArm)]);
	if (leftArmTransform) {
		leftArmTransform->rotate.x = std::sin(floatingParameter_) * kArmAmplitude;
	}

	Transform *rightArmTransform = registry_->GetComponent<Transform>(entities_[static_cast<size_t>(PartsType::kRightArm)]);
	if (rightArmTransform) {
		rightArmTransform->rotate.x = std::sin(floatingParameter_) * kArmAmplitude;
	}
}

void Player::SetNextBehavior(std::unique_ptr<BasePlayerBehavior> nextBehavior) { nextBehavior_ = std::move(nextBehavior); }