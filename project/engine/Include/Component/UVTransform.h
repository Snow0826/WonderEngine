#pragma once
#include "Vector3.h"

/// @brief UV変換データ
struct UVTransform final {
	Vector3 scale = { 1.0f, 1.0f, 1.0f };		// スケール
	Vector3 rotate = { 0.0f, 0.0f, 0.0f };		// 回転
	Vector3 translate = { 0.0f, 0.0f, 0.0f };	// 平行移動
};

class Registry;

/// @brief UV変換データインスペクター
class UVTransformInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	UVTransformInspector(Registry *registry) : registry_(registry) {}

	/// @brief UV変換データインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};