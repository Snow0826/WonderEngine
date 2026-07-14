#pragma once
#include "Collision.h"
#include <string>

/// @brief プリミティブ
struct Primitive final {
	std::string meshName;		// メッシュ名
	uint32_t textureHandle = 0;	// テクスチャハンドル
	bool enableMipMaps = false;	// ミップマップの有効化
	float error = 0.0f;			// LODエラー
	Collision::Sphere sphere;	// 球
	Collision::AABB aabb;		// AABB
	Collision::OBB obb;			// OBB
};

class MeshManager;
class TextureManager;

/// @brief プリミティブジェネレーター
class PrimitiveGenerator final {
public:
	/// @brief コンストラクタ
	/// @param meshManager メッシュマネージャー
	/// @param textureManager テクスチャマネージャー
	PrimitiveGenerator(MeshManager *meshManager, TextureManager *textureManager)
		: meshManager_(meshManager)
		, textureManager_(textureManager) {
	}

	/// @brief 平面の作成
	/// @param meshName メッシュ名
	/// @param textureFileName テクスチャファイル名
	/// @return 平面
	Primitive CreatePlane(const std::string &meshName, const std::string &textureFileName);

	/// @brief 立方体の作成
	/// @param meshName メッシュ名
	/// @param textureFileName テクスチャファイル名
	/// @return 立方体
	Primitive CreateBox(const std::string &meshName, const std::string &textureFileName);
	
	/// @brief 円環の作成
	/// @param meshName メッシュ名
	/// @param divide 分割数
	/// @param outerRadius 外径
	/// @param innerRadius 内径
	/// @param textureFileName テクスチャファイル名
	/// @return 円環
	Primitive CreateRing(const std::string &meshName, uint32_t divide, float outerRadius, float innerRadius, const std::string &textureFileName);
	
	/// @brief 円柱の作成
	/// @param meshName メッシュ名
	/// @param divide 分割数
	/// @param topRadius 上面の半径
	/// @param bottomRadius 下面の半径
	/// @param height 高さ
	/// @param cap キャップの有無
	/// @param textureFileName テクスチャファイル名
	/// @return 円柱
	Primitive CreateCylinder(const std::string &meshName, uint32_t divide, float topRadius, float bottomRadius, float height, bool cap, const std::string &textureFileName);

private:
	MeshManager *meshManager_ = nullptr;		// メッシュマネージャー
	TextureManager *textureManager_ = nullptr;	// テクスチャマネージャー
};