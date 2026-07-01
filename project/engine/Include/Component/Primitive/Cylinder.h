#pragma once
#include "Collision.h"
#include <string>

/// @brief 円柱
struct Cylinder final {
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

/// @brief 円柱ジェネレーター
class CylinderGenerator final {
public:
	/// @brief コンストラクタ
	/// @param meshManager メッシュマネージャー
	/// @param textureManager テクスチャマネージャー
	CylinderGenerator(MeshManager *meshManager, TextureManager *textureManager)
		: meshManager_(meshManager)
		, textureManager_(textureManager) {
	}

	/// @brief 円柱の作成
	/// @param divide 分割数
	/// @param topRadius 上面の半径
	/// @param bottomRadius 下面の半径
	/// @param height 高さ
	/// @param cap キャップの有無
	/// @param textureFileName テクスチャファイル名
	/// @return 円柱
	Cylinder CreateCylinder(uint32_t divide, float topRadius, float bottomRadius, float height, bool cap, const std::string &textureFileName);

private:
	MeshManager *meshManager_ = nullptr;		// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};