#pragma once
#include <cstdint>

/// @brief 球レンダラー
struct SphereRenderer final {};

/// @brief 球コライダー
struct SphereCollider final {};

class Registry;
class DebugRenderer;

/// @brief 球の描画システム
class SphereRenderSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param debugRenderer デバッグレンダラー
	SphereRenderSystem(Registry *registry, DebugRenderer *debugRenderer) : registry_(registry), debugRenderer_(debugRenderer) {}

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;				// レジストリ
	DebugRenderer *debugRenderer_ = nullptr;	// デバッグレンダラー
};

/// @brief 球インスペクター
class SphereInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	SphereInspector(Registry *registry) : registry_(registry) {}

	/// @brief 球インスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};