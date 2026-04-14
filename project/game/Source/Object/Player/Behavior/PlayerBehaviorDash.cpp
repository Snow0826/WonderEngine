#include "PlayerBehaviorDash.h"
#include "PlayerBehaviorRoot.h"
#include "Player.h"
#include "EntityComponentSystem.h"
#include "Transform.h"
#include "RigidBody.h"

void PlayerBehaviorDash::Initialize() { dashParameter_ = 0; }

void PlayerBehaviorDash::Update() {
#ifdef USE_IMGUI
	ImGui::Text("Behavior: Dash");
#endif // USE_IMGUI

	Player *player = GetPlayer();
	Registry *registry = player->GetRegistry();
	TransformSystem transformSystem{ registry };
	RigidBody *rigidBody = registry->GetComponent<RigidBody>(player->GetPartsEntity(Player::PartsType::kBody));
	if (rigidBody) {
		Vector3 forward = transformSystem.GetForward(player->GetPartsEntity(Player::PartsType::kBody));
		rigidBody->velocity.x = forward.x * -50.0f;
		rigidBody->velocity.z = forward.z * -50.0f;
	}

	dashParameter_++;
	if (dashParameter_ > kDashDuration) {
		dashParameter_ = 0;
		player->SetNextBehavior(std::make_unique<PlayerBehaviorRoot>());
	}
}