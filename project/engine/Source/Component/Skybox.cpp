#include "Skybox.h"
#include "Mesh.h"
#include "Texture.h"

Skybox SkyboxGenerator::CreateSkybox(const std::string &meshName, const std::string &textureFileName) {
	meshManager_->CreateBox(meshName);
	Skybox skybox{
		.meshName = meshName,	// メッシュ名
		.textureHandle = textureManager_->GetTextureReadHandle(textureFileName),	// テクスチャハンドル
	};
	return skybox;
}