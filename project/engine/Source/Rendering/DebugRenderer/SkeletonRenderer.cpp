#include "SkeletonRenderer.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Model.h"

void SkeletonRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Transform, Skeleton, DebugSkeletonSettings, SkeletonRenderer>([&](uint32_t entity, Transform *transform, Skeleton *skeleton, DebugSkeletonSettings *debugSettings, SkeletonRenderer *skeletonRenderer) {
		for (const Joint &joint : skeleton->joints) {
			Vector3 worldPosition = Vector3{} * joint.skeletonSpaceMatrix * transform->worldMatrix;
			debugRenderer_->AddSphere({ worldPosition, debugSettings->jointRadius });
			for (const uint32_t child : joint.children) {
				Vector3 childWorldPosition = Vector3{} * skeleton->joints[child].skeletonSpaceMatrix * transform->worldMatrix;
				debugRenderer_->AddLine({ worldPosition, childWorldPosition, {1.0f, 1.0f, 1.0f, 1.0f} });
			}
		}
		}, exclude<Disabled>());
#endif // DRAW_LINE
}