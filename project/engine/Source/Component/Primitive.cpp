#include "Primitive.h"
#include "Mesh.h"
#include "Texture.h"
#include "EntityComponentSystem.h"
#include "World.h"
#include "Transform.h"

#ifdef USE_IMGUI
#include <imgui.h>
#include <ImGuiManager.h>
#endif // USE_IMGUI

Primitive PrimitiveGenerator::CreatePlane(const std::string &meshName, const std::string &textureFileName) {
	meshManager_->CreatePlane(meshName);
	meshNames_.emplace_back(meshName);
	Primitive plane{
		.meshName = meshName,														// メッシュ名
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return plane;
}

Primitive PrimitiveGenerator::CreateBox(const std::string &meshName, const std::string &textureFileName) {
	meshManager_->CreateBox(meshName);
	meshNames_.emplace_back(meshName);
	Primitive box{
		.meshName = meshName,														// メッシュ名
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return box;
}

Primitive PrimitiveGenerator::CreateRing(const std::string &meshName, uint32_t divide, float outerRadius, float innerRadius, const std::string &textureFileName) {
	meshManager_->CreateRing(meshName, divide, outerRadius, innerRadius);
	meshNames_.emplace_back(meshName);
	Primitive ring{
		.meshName = meshName,														// メッシュ名
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return ring;
}

Primitive PrimitiveGenerator::CreateCylinder(const std::string &meshName, uint32_t divide, float topRadius, float bottomRadius, float height, bool cap, const std::string &textureFileName) {
	meshManager_->CreateCylinder(meshName, divide, topRadius, bottomRadius, height, cap);
	meshNames_.emplace_back(meshName);
	Primitive cylinder{
		.meshName = meshName,														// メッシュ名
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return cylinder;
}

Primitive PrimitiveGenerator::CreateTree(const std::string &meshName, const BranchMesh &branchMesh, uint32_t lengthSegments, uint32_t radialSegments, float uvScale, const std::string &textureFileName) {
	meshManager_->CreateTree(meshName, branchMesh, lengthSegments, radialSegments, uvScale);
	meshNames_.emplace_back(meshName);
	Primitive tree{
		.meshName = meshName,														// メッシュ名
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return tree;
}

void PrimitiveInspector::Draw([[maybe_unused]] uint32_t entity) {
#ifdef USE_IMGUI
	if (ImGui::TreeNode("Primitive")) {
		if (auto primitive = registry_->GetComponent<Primitive>(entity)) {
			if (ImGuiManager::Combo("Primitive", primitive->meshName, primitiveGenerator_->GetMeshNames())) {
				TransformSystem transformSystem{ registry_ };
				transformSystem.MarkDirty(entity);
				registry_->AddComponent(entity, DirtyTextureData{});
				registry_->AddComponent(entity, DirtyMeshLOD{});
				registry_->AddComponent(entity, DirtyCullingData{});
			}

			if (ImGui::TreeNode(primitive->meshName.c_str())) {
				size_t vertexCount = meshManager_->GetVertexCount(primitive->meshName);
				uint32_t indexCount = meshManager_->GetIndexCount(primitive->meshName);
				ImGui::Text("vertices: %zu", vertexCount);
				ImGui::Text("indices: %zu", indexCount);
				ImGui::Text("triangles: %zu", indexCount / 3);
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
#endif // USE_IMGUI
}