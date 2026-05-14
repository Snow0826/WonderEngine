#include "Skybox.h"
#include "Mesh.h"
#include "Texture.h"

Skybox SkyboxGenerator::CreateSkybox(const std::string &textureFileName) {
	Skybox skybox{
		.meshHandle = meshManager_->CreateBox(),									// メッシュハンドル
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
	};
	return skybox;
}