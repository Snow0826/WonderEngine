#pragma once
#include "Collision.h"
#include <string>

/// @brief 立方体
struct Box final {
	uint32_t meshHandle = 0;	// メッシュハンドル
	uint32_t textureHandle = 0;	// テクスチャハンドル
	bool enableMipMaps = false;	// ミップマップの有効化
	float error = 0.0f;			// LODエラー
	Collision::Sphere sphere;	// 球
	Collision::AABB aabb;		// AABB
	Collision::OBB obb;			// OBB
};

class TextureManager;
class MeshManager;

/// @brief 立方体ジェネレーター
class BoxGenerator final {
public:
	/// @brief コンストラクタ
	/// @param meshManager メッシュマネージャー
	/// @param textureManager テクスチャマネージャー
	BoxGenerator(MeshManager *meshManager, TextureManager *textureManager)
		: meshManager_(meshManager)
		, textureManager_(textureManager) {
	}

	/// @brief 立方体の作成
	/// @param textureFileName テクスチャファイル名
	/// @return 立方体
	Box CreateBox(const std::string &textureFileName);

private:
	MeshManager *meshManager_ = nullptr;		// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};