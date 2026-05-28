#pragma once
#include "Vector3.h"
#include "Vector4.h"

/// @brief 平行光源
struct DirectionalLight final {
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	// ライトの色
	Vector3 direction = { 0.0f, 0.0f, 0.0f };	// ライトの方向
	float intensity = 1.0f;						// 輝度
};

class Registry;

/// @brief 平行光源インスペクター
class DirectionalLightInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	DirectionalLightInspector(Registry *registry) : registry_(registry) {}

	/// @brief 平行光源インスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;
};