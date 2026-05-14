#pragma once
#include <string>

/// @brief スカイボックス
struct Skybox final {
	uint32_t meshHandle = 0;	// メッシュハンドル
	uint32_t textureHandle = 0;	// テクスチャハンドル
};

class TextureManager;
class MeshManager;

/// @brief スカイボックスジェネレーター
class SkyboxGenerator final {
public:
	/// @brief コンストラクタ
	/// @param meshManager メッシュマネージャー
	/// @param textureManager テクスチャマネージャー
	SkyboxGenerator(MeshManager *meshManager, TextureManager *textureManager)
		: meshManager_(meshManager)
		, textureManager_(textureManager) {
	}

	/// @brief スカイボックスの作成
	/// @param textureFileName テクスチャファイル名
	/// @return スカイボックス
	Skybox CreateSkybox(const std::string &textureFileName);

private:
	MeshManager *meshManager_ = nullptr;		// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};