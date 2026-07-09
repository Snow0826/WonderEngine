#include "Primitive.h"
#include "Mesh.h"
#include "Texture.h"

Primitive PrimitiveGenerator::CreatePlane(const std::string &textureFileName) {
	Primitive plane{
		.meshHandle = meshManager_->CreatePlane(),									// メッシュハンドル
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
		.sphere = meshManager_->CreateLocalSphere(plane.meshHandle),				// 球
		.aabb = meshManager_->CreateLocalAABB(plane.meshHandle),					// AABB
		.obb = meshManager_->CreateLocalOBB(plane.meshHandle),						// OBB
	};
	return plane;
}

Primitive PrimitiveGenerator::CreateBox(const std::string &textureFileName) {
	Primitive box{
		.meshHandle = meshManager_->CreateBox(),									// メッシュハンドル
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
		.sphere = meshManager_->CreateLocalSphere(box.meshHandle),					// 球
		.aabb = meshManager_->CreateLocalAABB(box.meshHandle),						// AABB
		.obb = meshManager_->CreateLocalOBB(box.meshHandle),						// OBB
	};
	return box;
}

Primitive PrimitiveGenerator::CreateRing(uint32_t divide, float outerRadius, float innerRadius, const std::string &textureFileName) {
	Primitive ring{
		.meshHandle = meshManager_->CreateRing(divide, outerRadius, innerRadius),	// メッシュハンドル
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
		.sphere = meshManager_->CreateLocalSphere(ring.meshHandle),					// 球
		.aabb = meshManager_->CreateLocalAABB(ring.meshHandle),						// AABB
		.obb = meshManager_->CreateLocalOBB(ring.meshHandle),						// OBB
	};
	return ring;
}

Primitive PrimitiveGenerator::CreateCylinder(uint32_t divide, float topRadius, float bottomRadius, float height, bool cap, const std::string &textureFileName) {
	Primitive cylinder{
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