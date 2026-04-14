#pragma once
#include <cstdint>

/// @brief 平面レンダラー
struct PlaneRenderer final {};

/// @brief 平面コライダー
struct PlaneCollider final {};

class Registry;
class DebugRenderer;

/// @brief 平面の描画システム
class PlaneRenderSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param debugRenderer デバッグレンダラー
	PlaneRenderSystem(Registry *registry, DebugRenderer *debugRenderer) : registry_(registry), debugRenderer_(debugRenderer) {}

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;				// レジストリ
	DebugRenderer *debugRenderer_ = nullptr;	// デバッグレンダラー
};

/// @brief 平面インスペクター
class PlaneInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	PlaneInspector(Registry *registry) : registry_(registry) {}

	/// @brief 平面インスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};