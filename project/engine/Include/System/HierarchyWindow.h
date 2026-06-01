#pragma once
#include <cstdint>

class Registry;
struct SelectionContext;

/// @brief 階層ウィンドウ
class HierarchyWindow final {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param selection 選択コンテキスト
	HierarchyWindow(Registry *registry, SelectionContext *selection) : registry_(registry), selection_(selection) {}

	/// @brief 描画
	void Draw();

private:
	Registry *registry_ = nullptr;	// レジストリ
	SelectionContext *selection_ = nullptr;	// 選択コンテキスト
};