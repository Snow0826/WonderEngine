#include "PlayerBehaviorRoot.h"
#include "PlayerBehaviorAttack.h"
#include "PlayerBehaviorJump.h"
#include "PlayerBehaviorDash.h"
#include "Player.h"
#include "Input.h"
#include "Audio.h"
#include "EntityComponentSystem.h"
#include "Sphere.h"

void PlayerBehaviorRoot::Initialize() {
	Player *player = GetPlayer();
	Registry *registry = player->GetRegistry();
	registry->RemoveComponent<SphereCollider>(player->GetPartsEntity(Player::PartsType::kLeftArm));
	registry->RemoveComponent<SphereCollider>(player->GetPartsEntity(Player::PartsType::kRightArm));
}

void PlayerBehaviorRoot::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Behavior: Root");
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

	// 基本動作の更新
	player->Move();
	player->Rotate();
	player->Float();

	// キーボード入力による攻撃
	if (input->IsPressMouse(0)) {
		player->SetNextBehavior(std::make_unique<PlayerBehaviorAttack>());
		audio->Play(soundHandle.seHandleMap_[SEType::Attack], false);
	}

	// キーボード入力によるジャンプ
	if (input->IsTriggerKey(DIK_SPACE)) {
		if (!jumpCounter) {
			player->SetNextBehavior(std::make_unique<PlayerBehaviorJump>());
			audio->Play(soundHandle.seHandleMap_[SEType::Jump], false);
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
			if (!jumpCounter) {
				player->SetNextBehavior(std::make_unique<PlayerBehaviorJump>());
				audio->Play(soundHandle.seHandleMap_[SEType::Jump], false);
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
}