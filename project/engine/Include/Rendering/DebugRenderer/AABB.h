#pragma once
#include <cstdint>

/// @brief AABBレンダラー
struct AABBRenderer final {};

/// @brief AABBコライダー
struct AABBCollider final {};

class Registry;
class DebugRenderer;

/// @brief AABBの描画システム
class AABBRenderSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param debugRenderer デバッグレンダラー
	AABBRenderSystem(Registry *registry, DebugRenderer *debugRenderer) : registry_(registry), debugRenderer_(debugRenderer) {}

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;				// レジストリ
	DebugRenderer *debugRenderer_ = nullptr;	// デバッグレンダラー
};

/// @brief AABBインスペクター
class AABBInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	AABBInspector(Registry *registry) : registry_(registry) {}

	/// @brief AABBインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};