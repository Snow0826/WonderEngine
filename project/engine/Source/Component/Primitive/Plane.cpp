#include "Plane.h"
#include "Mesh.h"
#include "Texture.h"

Plane PlaneGenerator::CreatePlane(const std::string &textureFileName) {
	Plane plane{
		.meshHandle = meshManager_->CreatePlane(),												// メッシュハンドル
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),				// テクスチャハンドル
		.enableMipMaps = true,																	// ミップマップの有効化
		.error = 0.0f,																			// LODエラー
		.sphere = meshManager_->CreateLocalSphere(plane.meshHandle),							// 球
		.aabb = meshManager_->CreateLocalAABB(plane.meshHandle),								// AABB
		.obb = meshManager_->CreateLocalOBB(plane.meshHandle),									// OBB
	};
	return plane;
}