#pragma once
#include "Vector3.h"
#include "Vector4.h"

/// @brief 点光源
struct PointLight final {
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	// ライトの色
	Vector3 position = { 0.0f, 0.0f, 0.0f };	// ライトの位置
	float intensity = 0.0f;						// 輝度
	float radius = 5.0f;						// ライトの範囲
	float decay = 2.0f;							// 減衰率
};

class Registry;

/// @brief 点光源インスペクター
class PointLightInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	PointLightInspector(Registry *registry) : registry_(registry) {}

	/// @brief 点光源インスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;
};