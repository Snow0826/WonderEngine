#include "EntityComponentSystem.h"
#include "Model.h"
#include "Transform.h"
#include "Easing.h"

void AnimationSystem::Update(float deltaTime) {
	TransformSystem transformSystem{ registry_ };
	registry_->ForEach<AnimationInterpolationMode, AnimationPlayer, Model>([&](uint32_t entity, AnimationInterpolationMode *mode, AnimationPlayer *player, Model *model) {
		const AnimationClip &animationClip = model->modelData.animations[player->animationIndex];
		// アニメーション時間の更新
		player->currentTime += deltaTime * player->speed;
		if (player->isLoop) {
			player->currentTime = std::fmodf(player->currentTime, animationClip.duration);
		} else {
			player->currentTime = std::min(player->currentTime, animationClip.duration);
		}

		// ノードアニメーションの適用
		for (NodeAnimation nodeAnimation : animationClip.nodeAnimations) {
			switch (*mode) {
				case AnimationInterpolationMode::Linear:
					if (!nodeAnimation.translations.empty()) {
						model->modelData.rootNode.translation = SampleLinearVector3(nodeAnimation.translations, player->currentTime);
					}
					if (!nodeAnimation.rotations.empty()) {
						model->modelData.rootNode.rotation = SampleLinearQuaternion(nodeAnimation.rotations, player->currentTime);
					}
					if (!nodeAnimation.scales.empty()) {
						model->modelData.rootNode.scale = SampleLinearVector3(nodeAnimation.scales, player->currentTime);
					}
					break;
				case AnimationInterpolationMode::Step:
					if (!nodeAnimation.translations.empty()) {
						model->modelData.rootNode.translation = SampleStepVector3(nodeAnimation.translations, player->currentTime);
					}
					if (!nodeAnimation.rotations.empty()) {
						model->modelData.rootNode.rotation = SampleStepQuaternion(nodeAnimation.rotations, player->currentTime);
					}
					if (!nodeAnimation.scales.empty()) {
						model->modelData.rootNode.scale = SampleStepVector3(nodeAnimation.scales, player->currentTime);
					}
					break;
				default:
					break;
			}
		}

		// 変換の更新をマーク
		transformSystem.MarkDirty(entity);
		}, exclude<>());
}

Vector3 AnimationSystem::SampleLinearVector3(const std::vector<KeyFrameVector3> &keyframes, float time) {
	if (keyframes.empty()) {
		return Vector3{};
	}
	if (time <= keyframes.front().time) {
		return keyframes.front().value;
	}
	if (time >= keyframes.back().time) {
		return keyframes.back().value;
	}
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		const KeyFrameVector3 &kf0 = keyframes[i];
		const KeyFrameVector3 &kf1 = keyframes[i + 1];
		if (time < kf1.time) {
			float t = (time - kf0.time) / (kf1.time - kf0.time);
			return Easing<Vector3>::Lerp(kf0.value, kf1.value, t);
		}
	}
	return keyframes.back().value;
}

Quaternion AnimationSystem::SampleLinearQuaternion(const std::vector<KeyFrameQuaternion> &keyframes, float time) {
	if (keyframes.empty()) {
		return Quaternion::IdentityQuaternion();
	}
	if (time <= keyframes.front().time) {
		return keyframes.front().value;
	}
	if (time >= keyframes.back().time) {
		return keyframes.back().value;
	}
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		const KeyFrameQuaternion &kf0 = keyframes[i];
		const KeyFrameQuaternion &kf1 = keyframes[i + 1];
		if (time < kf1.time) {
			float t = (time - kf0.time) / (kf1.time - kf0.time);
			return Quaternion::Slerp(kf0.value, kf1.value, t);
		}
	}
	return keyframes.back().value;
}

Vector3 AnimationSystem::SampleStepVector3(const std::vector<KeyFrameVector3> &keyframes, float time) {
	if (keyframes.empty()) {
		return Vector3{};
	}
	if (time <= keyframes.front().time) {
		return keyframes.front().value;
	}
	if (time >= keyframes.back().time) {
		return keyframes.back().value;
	}
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		const KeyFrameVector3 &kf0 = keyframes[i];
		const KeyFrameVector3 &kf1 = keyframes[i + 1];
		if (time < kf1.time) {
			return kf0.value;
		}
	}
	return keyframes.back().value;
}

Quaternion AnimationSystem::SampleStepQuaternion(const std::vector<KeyFrameQuaternion> &keyframes, float time) {
	if (keyframes.empty()) {
		return Quaternion::IdentityQuaternion();
	}
	if (time <= keyframes.front().time) {
		return keyframes.front().value;
	}
	if (time >= keyframes.back().time) {
		return keyframes.back().value;
	}
	for (size_t i = 0; i < keyframes.size() - 1; ++i) {
		const KeyFrameQuaternion &kf0 = keyframes[i];
		const KeyFrameQuaternion &kf1 = keyframes[i + 1];
		if (time < kf1.time) {
			return kf0.value;
		}
	}
	return keyframes.back().value;
}