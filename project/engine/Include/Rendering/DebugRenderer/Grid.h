#pragma once
#include <vector>

class DebugRenderer;
namespace Rendering {
	struct Line;
}

/// @brief グリッド
class Grid final {
public:
	/// @brief コンストラクタ
	/// @param debugRenderer デバッグレンダラー
	Grid(DebugRenderer *debugRenderer) : debugRenderer_(debugRenderer) {}

	/// @brief 初期化
	void Initialize();

	/// @brief 更新
	void Update();

	/// @brief 編集
	void Edit();

private:
	DebugRenderer *debugRenderer_ = nullptr;	// デバッグレンダラー
	std::vector<Rendering::Line> lines_;		// ラインリスト
	bool visible_ = true;						// 描画フラグ
	static inline constexpr float kGridHalfWidth = 50.0f;	// グリッドの半分の幅
	static inline constexpr uint32_t kSubdivision = 100;	// グリッドの分割数
	static inline constexpr float kGridEvery = (kGridHalfWidth * 2.0f) / static_cast<float>(kSubdivision);	// グリッドの1マスの幅
};