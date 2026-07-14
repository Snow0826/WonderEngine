#pragma once
#include "Matrix4x4.h"
#include "Vector4.h"
#include <string>

/// @brief スカイボックス
struct Skybox final {
	std::string meshName;		// メッシュ名
	uint32_t textureHandle = 0;	// テクスチャハンドル
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	// 色
};

/// @brief スカイボックス(GPU)
struct SkyboxForGPU final {
	Matrix4x4 worldMatrix;	// ワールド行列
	Vector4 color;	// 色
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
	/// @param meshName メッシュ名
	/// @param textureFileName テクスチャファイル名
	/// @return スカイボックス
	Skybox CreateSkybox(const std::string &meshName, const std::string &textureFileName);

private:
	MeshManager *meshManager_ = nullptr;		// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};