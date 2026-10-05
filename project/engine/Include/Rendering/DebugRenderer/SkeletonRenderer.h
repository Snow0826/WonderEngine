#pragma once

/// @brief スケルトンレンダラー
struct SkeletonRenderer final {};

/// @brief ジョイントの描画設定
struct DebugSkeletonSettings final {
	float jointRadius = 0.1f;	// ジョイントの半径
};

class Registry;
class ModelManager;
class DebugRenderer;

/// @brief スケルトンの描画システム
class SkeletonRenderSystem {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param modelManager モデルマネージャー
	/// @param debugRenderer デバッグレンダラー
	SkeletonRenderSystem(Registry *registry, ModelManager *modelManager, DebugRenderer *debugRenderer) : registry_(registry), modelManager_(modelManager), debugRenderer_(debugRenderer) {}

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;				// レジストリ
	ModelManager *modelManager_ = nullptr;		// モデルマネージャー
	DebugRenderer *debugRenderer_ = nullptr;	// デバッグレンダラー
};

