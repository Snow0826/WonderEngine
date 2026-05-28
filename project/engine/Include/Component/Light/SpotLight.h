#pragma once
#include "Vector3.h"
#include "Vector4.h"

/// @brief スポットライト
struct SpotLight final {
	Vector4 color = { 1.0f, 1.0f, 1.0f, 1.0f };	// ライトの色
	Vector3 position = { 2.0f, 1.25f, 0.0f };	// ライトの位置
	float intensity = 0.0f;						// 輝度
	Vector3 direction = { -1.0f, -1.0f, 0.0f };	// ライトの方向
	float distance = 7.0f;						// ライトの距離
	float decay = 2.0f;							// 減衰率
	float cosAngle = 0.0f;						// ライトの余弦
	float cosFalloffStart = 0.0f;				// フォールオフ開始の余弦
};

/// @brief スポットライトフォールオフ
struct SpotLightFalloff final {
	float start = 0.0f;	// 開始
	float end = 0.0f;	// 終了
};

class Registry;

/// @brief スポットライトインスペクター
class SpotLightInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	SpotLightInspector(Registry *registry) : registry_(registry) {}

	/// @brief スポットライトインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;
};