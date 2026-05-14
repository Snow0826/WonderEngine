#pragma once
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"

/// @brief マテリアルデータが変更されたフラグ
struct DirtyMaterial final {};

/// @brief マテリアル
struct Material final {
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };			// 色
	int32_t enableLighting = 1;							// ライティングの有効化
	float padding[3]{};									// パディング
	Matrix4x4 uvTransformMatrix = MakeIdentity4x4();	// UV変換行列
	float shininess = 32.0f;							// 光沢度
	Vector3 specular = { 1.0f, 1.0f, 1.0f };			// スペキュラカラー
	float environmentCoefficient = 1.0f;				// 環境光の影響度
};

class Registry;

/// @brief マテリアルインスペクター
class MaterialInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	MaterialInspector(Registry *registry) : registry_(registry) {}

	/// @brief マテリアルインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};