#include "PlayerBehaviorJump.h"
#include "PlayerBehaviorAttack.h"
#include "PlayerBehaviorDash.h"
#include "PlayerBehaviorRoot.h"
#include "Player.h"
#include "Input.h"
#include "Audio.h"
#include "EntityComponentSystem.h"
#include "Transform.h"
#include "RigidBody.h"

void PlayerBehaviorJump::Initialize() {
	// プレイヤーの取得
	Player *player = GetPlayer();

	// レジストリの取得
	Registry *registry = player->GetRegistry();

	// 体の位置を基準位置に設定
	Transform *bodyTransform = registry->GetComponent<Transform>(player->GetPartsEntity(Player::PartsType::kBody));
	if (bodyTransform) {
		bodyTransform->translate.y = player->GetBasePositionY();
	}

	// 左腕の回転をリセット
	Transform *leftArmTransform = registry->GetComponent<Transform>(player->GetPartsEntity(Player::PartsType::kLeftArm));
	if (leftArmTransform) {
		leftArmTransform->rotate.x = 0.0f;
	}

	// 右腕の回転をリセット
	Transform *rightArmTransform = registry->GetComponent<Transform>(player->GetPartsEntity(Player::PartsType::kRightArm));
	if (rightArmTransform) {
		rightArmTransform->rotate.x = 0.0f;
	}

	// ジャンプの初速度を設定
	RigidBody *rigidBody = registry->GetComponent<RigidBody>(player->GetPartsEntity(Player::PartsType::kBody));
	if (rigidBody) {
		rigidBody->velocity.y = 10.0f;
	}
	(*player->GetJumpCounter())++;
}

void PlayerBehaviorJump::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Behavior: Jump");
#endif // USE_IMGUI

	// プレイヤーの取得	
	Player *player = GetPlayer();

	// 入力の取得
	Input *input = player->GetInput();

	// 音声の取得
	Audio *audio = player->GetAudio();

	// 音声ハンドルの取得
	SoundHandle soundHandle = player->GetSoundHandle();

	// ジャンプカウンターの取得
	uint32_t &jumpCounter = *player->GetJumpCounter();

	// プレイヤーの移動と回転
	player->Move();
	player->Rotate();

	// 体の剛体の取得
	RigidBody *rigidBody = player->GetRegistry()->GetComponent<RigidBody>(player->GetPartsEntity(Player::PartsType::kBody));
	if (!rigidBody) {
		return;
	}

	// キーボード入力による攻撃
	if (input->IsPressMouse(0)) {
		player->SetNextBehavior(std::make_unique<PlayerBehaviorAttack>());
		audio->Play(soundHandle.seHandleMap_[SEType::Attack], false);
	}

	// キーボード入力によるジャンプ
	if (input->IsTriggerKey(DIK_SPACE)) {
		if (jumpCounter < 2) {
			rigidBody->velocity.y = 10.0f;
			audio->Play(soundHandle.seHandleMap_[SEType::Jump], false);
			jumpCounter++;
		}
	}

	// キーボード入力によるダッシュ
	if (input->IsTriggerKey(DIK_LSHIFT)) {
		player->SetNextBehavior(std::make_unique<PlayerBehaviorDash>());
		audio->Play(soundHandle.seHandleMap_[SEType::Dash], false);
	}

	if (input->Connected()) {
		// コントローラー入力による攻撃
		if (input->IsTriggerButton(XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
			player->SetNextBehavior(std::make_unique<PlayerBehaviorAttack>());
			audio->Play(soundHandle.seHandleMap_[SEType::Attack], false);
		}

		// コントローラー入力によるジャンプ
		if (input->IsTriggerButton(XINPUT_GAMEPAD_A)) {
			if (jumpCounter < 2) {
				rigidBody->velocity.y = 10.0f;
				audio->Play(soundHandle.seHandleMap_[SEType::Jump], false);
				jumpCounter++;
			}
		}

		// コントローラー入力によるダッシュ
		if (input->IsTriggerButton(XINPUT_GAMEPAD_X)) {
			player->SetNextBehavior(std::make_unique<PlayerBehaviorDash>());
			audio->Play(soundHandle.seHandleMap_[SEType::Dash], false);
		}
	} else {
#ifdef USE_IMGUI
		ImGui::Text("No Controller Connected");
#endif // USE_IMGUI
	}

	if (!jumpCounter) {
		player->SetNextBehavior(std::make_unique<PlayerBehaviorRoot>());
	}
}