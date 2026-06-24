#pragma once

class Registry;
struct SelectionContext;

/// @brief 変換マニピュレーター
class TransformManipulator {
public:
	/// @brief コンストラクタ
	/// @param registry レジストリ
	/// @param selection 選択コンテキスト
	TransformManipulator(Registry *registry, SelectionContext *selection) : registry_(registry), selection_(selection) {}

	/// @brief 描画
	void Draw();

private:
	Registry *registry_ = nullptr;			// レジストリ
	SelectionContext *selection_ = nullptr;	// 選択コンテキスト
};