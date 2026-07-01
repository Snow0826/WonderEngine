#include "Cylinder.h"
#include "Mesh.h"
#include "Texture.h"

Cylinder CylinderGenerator::CreateCylinder(uint32_t divide, float topRadius, float bottomRadius, float height, bool cap, const std::string &textureFileName) {
	Cylinder cylinder{
		.meshHandle = meshManager_->CreateCylinder(divide, topRadius, bottomRadius, height, cap),	// メッシュハンドル
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),					// テクスチャハンドル
		.enableMipMaps = true,																		// ミップマップの有効化
		.error = 0.0f,																				// LODエラー
		.sphere = meshManager_->CreateLocalSphere(cylinder.meshHandle),								// 球
		.aabb = meshManager_->CreateLocalAABB(cylinder.meshHandle),									// AABB
		.obb = meshManager_->CreateLocalOBB(cylinder.meshHandle),									// OBB
	};
	return cylinder;
}