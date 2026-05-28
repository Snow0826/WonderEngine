#include "Ring.h"
#include "Mesh.h"
#include "Texture.h"

Ring RingGenerator::CreateRing(uint32_t divide, float outerRadius, float innerRadius, const std::string &textureFileName) {
	Ring ring{
		.meshHandle = meshManager_->CreateRing(divide, outerRadius, innerRadius),				// メッシュハンドル
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),				// テクスチャハンドル
		.enableMipMaps = true,																	// ミップマップの有効化
		.error = 0.0f,																			// LODエラー
		.sphere = meshManager_->CreateLocalSphere(ring.meshHandle),								// 球
		.aabb = meshManager_->CreateLocalAABB(ring.meshHandle),									// AABB
		.obb = meshManager_->CreateLocalOBB(ring.meshHandle),									// OBB
	};
	return ring;
}
