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

	/// @brief ウィンドウの開閉状態への参照を取得する
	/// @return ウィンドウの開閉状態への参照
	bool &IsOpen() { return isOpen_; }

private:
	Registry *registry_ = nullptr;	// レジストリ
	SelectionContext *selection_ = nullptr;	// 選択コンテキスト
	bool isOpen_ = true;	// ウィンドウの開閉状態

	/// @brief エンティティの描画
	/// @param entity エンティティ
	void DrawEntity(uint32_t entity);
};