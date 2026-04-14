#pragma once
#include "Vector3.h"

/// @brief OBBの回転情報
struct OBBRotate final {
	Vector3 rotate; // 回転角
};

/// @brief OBBレンダラー
struct OBBRenderer final {};

/// @brief OBBコライダー
struct OBBCollider final {};

class Registry;
class DebugRenderer;

/// @brief OBBの描画システム
class OBBRenderSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param debugRenderer デバッグレンダラー
	OBBRenderSystem(Registry *registry, DebugRenderer *debugRenderer) : registry_(registry), debugRenderer_(debugRenderer) {}

	/// @brief 各軸の単位ベクトルの更新
	void UpdateOrientations();

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;				// レジストリ
	DebugRenderer *debugRenderer_ = nullptr;	// デバッグレンダラー
};

/// @brief OBBインスペクター
class OBBInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	OBBInspector(Registry *registry) : registry_(registry) {}

	/// @brief OBBインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};