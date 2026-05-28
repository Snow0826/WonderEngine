#include "Box.h"
#include "Mesh.h"
#include "Texture.h"

Box BoxGenerator::CreateBox(const std::string &textureFileName) {
	Box box{
		.meshHandle = meshManager_->CreateBox(),												// メッシュハンドル
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),				// テクスチャハンドル
		.enableMipMaps = true,																	// ミップマップの有効化
		.error = 0.0f,																			// LODエラー
		.sphere = meshManager_->CreateLocalSphere(box.meshHandle),								// 球
		.aabb = meshManager_->CreateLocalAABB(box.meshHandle),									// AABB
		.obb = meshManager_->CreateLocalOBB(box.meshHandle),									// OBB
	};
	return box;
}