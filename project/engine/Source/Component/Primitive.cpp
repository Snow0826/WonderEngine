#include "Primitive.h"
#include "Mesh.h"
#include "Texture.h"

Primitive PrimitiveGenerator::CreatePlane(const std::string &meshName, const std::string &textureFileName) {
	meshManager_->CreatePlane(meshName);
	Primitive plane{
		.meshName = meshName,
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return plane;
}

Primitive PrimitiveGenerator::CreateBox(const std::string &meshName, const std::string &textureFileName) {
	meshManager_->CreateBox(meshName);
	Primitive box{
		.meshName = meshName,
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return box;
}

Primitive PrimitiveGenerator::CreateRing(const std::string &meshName, uint32_t divide, float outerRadius, float innerRadius, const std::string &textureFileName) {
	meshManager_->CreateRing(meshName, divide, outerRadius, innerRadius);
	Primitive ring{
		.meshName = meshName,
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return ring;
}

Primitive PrimitiveGenerator::CreateCylinder(const std::string &meshName, uint32_t divide, float topRadius, float bottomRadius, float height, bool cap, const std::string &textureFileName) {
	meshManager_->CreateCylinder(meshName, divide, topRadius, bottomRadius, height, cap);
	Primitive cylinder{
		.meshName = meshName,
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
		.enableMipMaps = true,														// ミップマップの有効化
		.error = 0.0f,																// LODエラー
	};
	return cylinder;
}