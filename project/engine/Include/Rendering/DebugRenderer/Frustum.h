#pragma once
#include "Vector4.h"
#include <array>

/// @brief 視錐台データ
struct Frustum final {
	std::array<Vector4, 6> frustumPlanes;	// 視錐台の平面
	std::array<Vector4, 8> frustumCorners;	// 視錐台の頂点
};

struct ViewProjectionData;

/// @brief 視錐台データの作成
/// @param viewProjection ビュープロジェクションデータ
/// @return 視錐台データ
Frustum MakeFrustum(const ViewProjectionData &viewProjection);

/// @brief 視錐台レンダラー
struct FrustumRenderer final {};

class Registry;
class DebugRenderer;

/// @brief 視錐台の描画システム
class FrustumRenderSystem final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param debugRenderer デバッグレンダラー
	FrustumRenderSystem(Registry *registry, DebugRenderer *debugRenderer) : registry_(registry), debugRenderer_(debugRenderer) {}

	/// @brief 更新
	void Update();

private:
	Registry *registry_ = nullptr;						// レジストリ
	DebugRenderer *debugRenderer_ = nullptr;			// デバッグレンダラー
	static inline const uint32_t kMaxLineCount = 12;	// ラインの最大数
};