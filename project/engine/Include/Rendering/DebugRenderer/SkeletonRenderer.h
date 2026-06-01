#pragma once

/// @brief スケルトンレンダラー
struct SkeletonRenderer final {};

/// @brief ジョイントの描画設定
struct DebugSkeletonSettings final {
	float jointRadius = 0.1f;	// ジョイントの半径
};

class Registry;
class DebugRenderer;

/// @brief スケルトンの描画システム
class SkeletonRenderSystem {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param debugRenderer デバッグレンダラー
	SkeletonRenderSystem(Registry *registry, DebugRenderer *debugRenderer) : registry_(registry), debugRenderer_(debugRenderer) {}

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;				// レジストリ
	DebugRenderer *debugRenderer_ = nullptr;	// デバッグレンダラー
};

