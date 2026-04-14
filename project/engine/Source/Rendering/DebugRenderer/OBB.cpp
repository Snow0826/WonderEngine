#define NOMINMAX
#include "OBB.h"
#include "EntityComponentSystem.h"
#include "DebugRenderer.h"
#include "Collision.h"
#include "Model.h"
#include "Matrix4x4.h"

void OBBRenderSystem::UpdateOrientations() {
	registry_->ForEach<Collision::OBB, OBBRotate>([&](uint32_t entity, Collision::OBB *obb, OBBRotate *obbRotate) {
		// 回転行列の作成
		Matrix4x4 rotateMatrix = MakeIdentity4x4();
		rotateMatrix = MakeRotateMatrix(obbRotate->rotate);

		// 各軸の設定
		obb->orientations[0] = { rotateMatrix.m[0][0], rotateMatrix.m[0][1], rotateMatrix.m[0][2] };
		obb->orientations[1] = { rotateMatrix.m[1][0], rotateMatrix.m[1][1], rotateMatrix.m[1][2] };
		obb->orientations[2] = { rotateMatrix.m[2][0], rotateMatrix.m[2][1], rotateMatrix.m[2][2] };
		}, exclude<Disabled>());
}

void OBBRenderSystem::Update() {
#ifdef DRAW_LINE
	registry_->ForEach<Collision::OBB, OBBRenderer>([&](uint32_t entity, Collision::OBB *obb, OBBRenderer *obbRenderer) {
		debugRenderer_->AddOBB(*obb);
		}, exclude<Disabled>());
	
	registry_->ForEach<Model, OBBRenderer>([&](uint32_t entity, Model *model, OBBRenderer *obbRenderer) {
		for (const MeshData &meshData : model->modelData.meshes) {
			debugRenderer_->AddOBB(meshData.worldCollisionData.obb);
		}
		}, exclude<Disabled>());
#endif // DRAW_LINE
}

void OBBInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("OBB")) {
		Collision::OBB *obb = registry_->GetComponent<Collision::OBB>(entity);
		if (obb) {
			ImGui::DragFloat3("center", &obb->center.x, 0.01f, -std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
			ImGui::DragFloat3("size", &obb->size.x, 0.01f, 0.0f, std::numeric_limits<float>::max());
		}

		OBBRotate *obbRotate = registry_->GetComponent<OBBRotate>(entity);
		if (obbRotate) {
			ImGui::SliderAngle("rotateX", &obbRotate->rotate.x, -360.0f, 360.0f);
			ImGui::SliderAngle("rotateY", &obbRotate->rotate.y, -360.0f, 360.0f);
			ImGui::SliderAngle("rotateZ", &obbRotate->rotate.z, -360.0f, 360.0f);
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}