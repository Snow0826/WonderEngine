#pragma once
#include "Collision.h"
#include <string>

/// @brief 平面
struct Plane final {
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

/// @brief 平面ジェネレーター
class PlaneGenerator final {
public:
	/// @brief コンストラクタ
	/// @param meshManager メッシュマネージャー
	/// @param textureManager テクスチャマネージャー
	PlaneGenerator(MeshManager *meshManager, TextureManager *textureManager)
		: meshManager_(meshManager)
		, textureManager_(textureManager) {
	}

	/// @brief 平面の作成
	/// @param textureFileName テクスチャファイル名
	/// @return 平面
	Plane CreatePlane(const std::string &textureFileName);

private:
	MeshManager *meshManager_ = nullptr;		// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};