#include "SkeletonRenderer.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Model.h"

void SkeletonRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Model, EulerTransform, SkinMesh, DebugSkeletonSettings, SkeletonRenderer>([&](uint32_t entity, Model *model, EulerTransform *transform, SkinMesh *skinMesh, DebugSkeletonSettings *debugSettings, SkeletonRenderer *skeletonRenderer) {
		for (const Joint &joint : model->modelData.skeleton.joints) {
			Vector3 worldPosition = Vector3{} * joint.skeletonSpaceMatrix * transform->worldMatrix;
			debugRenderer_->AddSphere({ worldPosition, debugSettings->jointRadius });
			for (const uint32_t child : joint.children) {
				Vector3 childWorldPosition = Vector3{} * model->modelData.skeleton.joints[child].skeletonSpaceMatrix * transform->worldMatrix;
				debugRenderer_->AddLine({ worldPosition, childWorldPosition, {1.0f, 1.0f, 1.0f, 1.0f} });
			}
		}
		}, exclude<Disabled>());

	registry_->ForEach<Model, QuaternionTransform, SkinMesh, DebugSkeletonSettings, SkeletonRenderer>([&](uint32_t entity, Model *model, QuaternionTransform *transform, SkinMesh *skinMesh, DebugSkeletonSettings *debugSettings, SkeletonRenderer *skeletonRenderer) {
		for (const Joint &joint : model->modelData.skeleton.joints) {
			Vector3 worldPosition = Vector3{} * joint.skeletonSpaceMatrix * transform->worldMatrix;
			debugRenderer_->AddSphere({ worldPosition, debugSettings->jointRadius });
			for (const uint32_t child : joint.children) {
				Vector3 childWorldPosition = Vector3{} * model->modelData.skeleton.joints[child].skeletonSpaceMatrix * transform->worldMatrix;
				debugRenderer_->AddLine({ worldPosition, childWorldPosition, {1.0f, 1.0f, 1.0f, 1.0f} });
			}
		}
		}, exclude<Disabled>());
#endif // DRAW_LINE
}