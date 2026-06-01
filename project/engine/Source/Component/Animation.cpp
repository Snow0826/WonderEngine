#include "EntityComponentSystem.h"
#include "Model.h"
#include "Easing.h"

void AnimationSystem::Update(float deltaTime) {
	// アニメーション時間の更新
	registry_->ForEach<AnimationPlayer, Model>([&](uint32_t entity, AnimationPlayer *player, Model *model) {
		const AnimationClip &animationClip = model->modelData.animations[player->animationIndex];
		player->currentTime += deltaTime * player->speed;
		if (player->isLoop) {
			player->currentTime = std::fmodf(player->currentTime, animationClip.duration);
		} else {
			player->currentTime = std::min(player->currentTime, animationClip.duration);
		}
	});
	
	// アニメーションをルートノードに適用
	ApplyAnimationToRootNode();

	// アニメーションをスケルトンに適用
	ApplyAnimationToSkeleton();
}

void AnimationSystem::ApplyAnimationToRootNode() {
	TransformSystem transformSystem{ registry_ };
	registry_->ForEach<AnimationInterpolationMode, AnimationPlayer, Model>([&](uint32_t entity, AnimationInterpolationMode *mode, AnimationPlayer *player, Model *model) {
		// ノードアニメーションの適用
		const AnimationClip &animationClip = model->modelData.animations[player->animationIndex];
		if (auto it = animationClip.nodeAnimations.find(model->modelData.rootNode.name); it != animationClip.nodeAnimations.end()) {
			const NodeAnimation &rootNodeAnimation = (*it).second;
			switch (*mode) {
				case AnimationInterpolationMode::Linear:
					if (!rootNodeAnimation.translate.keyframes.empty()) {
						model->modelData.rootNode.transform.translate = SampleLinearVector3(rootNodeAnimation.translate.keyframes, player->currentTime);
					}
					if (!rootNodeAnimation.rotate.keyframes.empty()) {
						model->modelData.rootNode.transform.quaternion = SampleLinearQuaternion(rootNodeAnimation.rotate.keyframes, player->currentTime);
					}
					if (!rootNodeAnimation.scale.keyframes.empty()) {
						model->modelData.rootNode.transform.scale = SampleLinearVector3(rootNodeAnimation.scale.keyframes, player->currentTime);
					}
					break;
				case AnimationInterpolationMode::Step:
					if (!rootNodeAnimation.translate.keyframes.empty()) {
						model->modelData.rootNode.transform.translate = SampleStepVector3(rootNodeAnimation.translate.keyframes, player->currentTime);
					}
					if (!rootNodeAnimation.rotate.keyframes.empty()) {
						model->modelData.rootNode.transform.quaternion = SampleStepQuaternion(rootNodeAnimation.rotate.keyframes, player->currentTime);
					}
					if (!rootNodeAnimation.scale.keyframes.empty()) {
						model->modelData.rootNode.transform.scale = SampleStepVector3(rootNodeAnimation.scale.keyframes, player->currentTime);
					}
					break;
				default:
					break;
			}
		}

		// ルートノードの変換が変更されたことを通知
		transformSystem.MarkDirty(entity);
		}, exclude<Skeleton>());
}

void AnimationSystem::ApplyAnimationToSkeleton() {
	TransformSystem transformSystem{ registry_ };
	registry_->ForEach<AnimationInterpolationMode, AnimationPlayer, Model, Skeleton>([&](uint32_t entity, AnimationInterpolationMode *mode, AnimationPlayer *player, Model *model, Skeleton *skeleton) {
		// ノードアニメーションの適用
		const AnimationClip &animationClip = model->modelData.animations[player->animationIndex];
		for (Joint &joint : skeleton->joints) {
			if (auto it = animationClip.nodeAnimations.find(joint.name); it != animationClip.nodeAnimations.end()) {
				const NodeAnimation &nodeAnimation = (*it).second;
				switch (*mode) {
					case AnimationInterpolationMode::Linear:
						if (!nodeAnimation.translate.keyframes.empty()) {
							joint.transform.translate = SampleLinearVector3(nodeAnimation.translate.keyframes, player->currentTime);
						}
						if (!nodeAnimation.rotate.keyframes.empty()) {
							joint.transform.quaternion = SampleLinearQuaternion(nodeAnimation.rotate.keyframes, player->currentTime);
						}
						if (!nodeAnimation.scale.keyframes.empty()) {
							joint.transform.scale = SampleLinearVector3(nodeAnimation.scale.keyframes, player->currentTime);
						}
						break;
					case AnimationInterpolationMode::Step:
						if (!nodeAnimation.translate.keyframes.empty()) {
							joint.transform.translate = SampleStepVector3(nodeAnimation.translate.keyframes, player->currentTime);
						}
						if (!nodeAnimation.rotate.keyframes.empty()) {
							joint.transform.quaternion = SampleStepQuaternion(nodeAnimation.rotate.keyframes, player->currentTime);
						}
						if (!nodeAnimation.scale.keyframes.empty()) {
							joint.transform.scale = SampleStepVector3(nodeAnimation.scale.keyframes, player->currentTime);
						}
						break;
					default:
						break;
				}
				continue;
			}
		}

		// スケルトンの更新
		ModelManager::UpdateSkeleton(*skeleton);

		// スケルトンの変換が変更されたことを通知
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
		if (time >= kf0.time && time <= kf1.time) {
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
		if (time >= kf0.time && time <= kf1.time) {
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
		if (time >= kf0.time && time <= kf1.time) {
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
		if (time >= kf0.time && time <= kf1.time) {
			return kf0.value;
		}
	}
	return keyframes.back().value;
}