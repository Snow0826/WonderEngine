#pragma once
#include <cstdint>

/// @brief カプセルレンダラー
struct CapsuleRenderer final {};

/// @brief カプセルコライダー
struct CapsuleCollider final {};

class Registry;
class DebugRenderer;

/// @brief カプセルの描画システム
class CapsuleRenderSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param debugRenderer デバッグレンダラー
	CapsuleRenderSystem(Registry *registry, DebugRenderer *debugRenderer) : registry_(registry), debugRenderer_(debugRenderer) {}

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;				// レジストリ
	DebugRenderer *debugRenderer_ = nullptr;	// デバッグレンダラー
};

/// @brief カプセルインスペクター
class CapsuleInspector final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	CapsuleInspector(Registry *registry) : registry_(registry) {}

	/// @brief カプセルインスペクターの描画
	/// @param entity エンティティ
	void Draw(uint32_t entity);

private:
	Registry *registry_ = nullptr;	// レジストリ
};