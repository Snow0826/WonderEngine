#pragma once
#include <Vector2.h>
#include <string>

/// @brief 地面の設定
struct GroundDesc {
	std::string_view modelName = "ground.obj";	// モデル名
	bool enableLighting = false;				// ライティングの有効化
	Vector2 terrainOriginXZ{ 0.0f, 0.0f };		// 地形の原点座標
	Vector2 terrainSizeXZ{ 1024.0f, 1024.0f };	// 地形のサイズ
};

class Registry;
class ModelManager;
class InstanceAllocator;

/// @brief 地面
class GroundFactory {
public:
	/// @brief 地面の作成
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	/// @param instanceAllocator インスタンスアロケータ
	/// @param desc 地面の設定
	static void Create(Registry &registry, const ModelManager &modelManager, InstanceAllocator &instanceAllocator, const GroundDesc &desc);
};