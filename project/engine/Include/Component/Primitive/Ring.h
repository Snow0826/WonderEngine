#pragma once
#include "Collision.h"
#include <string>

/// @brief 円環
struct Ring final {
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

/// @brief 円環ジェネレーター
class RingGenerator final {
public:
	/// @brief コンストラクタ
	/// @param meshManager メッシュマネージャー
	/// @param textureManager テクスチャマネージャー
	RingGenerator(MeshManager *meshManager, TextureManager *textureManager)
		: meshManager_(meshManager)
		, textureManager_(textureManager) {
	}

	/// @brief 円環の作成
	/// @param divide 分割数
	/// @param outerRadius 外径
	/// @param innerRadius 内径
	/// @param textureFileName テクスチャファイル名
	/// @return 円環
	Ring CreateRing(uint32_t divide, float outerRadius, float innerRadius, const std::string &textureFileName);

private:
	MeshManager *meshManager_ = nullptr;		// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};